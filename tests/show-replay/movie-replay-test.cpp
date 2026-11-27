#include "../../src/media/movie_decoder.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using freej_media::MovieDecoder;

namespace {

struct Replay {
  std::vector<std::int64_t> pictures;
  std::vector<std::uint32_t> pixels;
  std::vector<std::int64_t> audio_times;
  std::vector<float> pcm;
};

bool replay(MovieDecoder &movie, Replay &result) {
  MovieDecoder::Frame frame;
  for (unsigned i = 0; i < 4096; ++i) {
    switch (movie.next(frame)) {
      case MovieDecoder::Result::video:
        result.pictures.push_back(frame.timestamp);
        result.pixels.insert(result.pixels.end(), frame.pixels.begin(), frame.pixels.end());
        break;
      case MovieDecoder::Result::audio:
        result.audio_times.push_back(frame.timestamp);
        result.pcm.insert(result.pcm.end(), frame.samples.begin(), frame.samples.end());
        break;
      case MovieDecoder::Result::again:
        break;
      case MovieDecoder::Result::end:
        return movie.next(frame) == MovieDecoder::Result::end;
      case MovieDecoder::Result::error:
        return false;
    }
  }
  return false;
}

bool dump(MovieDecoder &movie, const char *pixels, const char *pcm) {
  MovieDecoder::Frame frame;
  std::ofstream video_out(pixels, std::ios::binary);
  std::ofstream audio_out(pcm, std::ios::binary);
  std::cout << "F " << movie.width() << ' ' << movie.height() << ' '
            << movie.channels() << ' ' << movie.sample_rate() << '\n';
  for (unsigned i = 0; i < 4096; ++i) {
    switch (movie.next(frame)) {
      case MovieDecoder::Result::video:
        std::cout << "V " << frame.timestamp << '\n';
        video_out.write(reinterpret_cast<const char *>(frame.pixels.data()),
                        frame.pixels.size() * sizeof(frame.pixels.front()));
        break;
      case MovieDecoder::Result::audio:
        std::cout << "A " << frame.timestamp << ' ' << frame.samples.size() / movie.channels() << '\n';
        audio_out.write(reinterpret_cast<const char *>(frame.samples.data()),
                        frame.samples.size() * sizeof(frame.samples.front()));
        break;
      case MovieDecoder::Result::again:
        break;
      case MovieDecoder::Result::end:
        return static_cast<bool>(video_out) && static_cast<bool>(audio_out);
      case MovieDecoder::Result::error:
        return false;
    }
  }
  return false;
}

bool inspect(const char *input) {
  MovieDecoder movie;
  Replay initial;
  if (!movie.open(input) || movie.width() != 80 || movie.height() != 64 ||
      movie.channels() != 2 || movie.sample_rate() != 48000 || movie.frame_rate() != 20 ||
      !replay(movie, initial) || initial.pictures.size() != 12 || initial.pcm.size() != 48000 ||
      initial.audio_times.empty() || initial.audio_times.front() != 100000)
    return false;

  for (std::size_t i = 0; i < initial.pictures.size(); ++i) {
    if (initial.pictures[i] != static_cast<std::int64_t>(i) * 50000)
      return false;
  }
  if (initial.pixels.size() != 12 * 80 * 64 || initial.pixels.front() == initial.pixels[50])
    return false;
  if (std::abs(initial.pcm[1000] - initial.pcm[1001]) < 0.01)
    return false;
  std::cout << "PASS 12 timestamped frames with delayed video drain; 24,000 stereo samples at +100 ms\n";

  Replay sought;
  if (!movie.seek(237500) || !replay(movie, sought) || sought.pictures.size() != 7 ||
      sought.pictures.front() != 250000 || sought.pictures.back() != 550000 ||
      sought.audio_times.empty() || sought.audio_times.front() != 237500 ||
      sought.pcm.size() != 34800 ||
      !std::equal(sought.pcm.begin(), sought.pcm.end(), initial.pcm.begin() + 13200) ||
      !std::equal(sought.pixels.begin(), sought.pixels.end(), initial.pixels.begin() + 5 * 80 * 64))
    return false;
  std::cout << "PASS backward seek after EOS trims audio and returns 7 matching video frames\n";

  Replay reopened;
  if (!movie.open(input) || !replay(movie, reopened) ||
      initial.pixels != reopened.pixels || initial.pictures != reopened.pictures ||
      initial.pcm != reopened.pcm || initial.audio_times != reopened.audio_times)
    return false;
  std::cout << "PASS reopen produces identical pixels, sample data and timestamps\n";

  Replay resampled;
  if (!movie.seek(0))
    return false;
  movie.set_output_sample_rate(44100);
  if (!replay(movie, resampled) || resampled.pcm.size() != 44100 ||
      resampled.audio_times.empty() || resampled.audio_times.front() != 100000)
    return false;
  std::cout << "PASS resampling to 44.1 kHz drains the final samples\n";
  return true;
}

bool diagnostics(const char *input) {
  MovieDecoder movie;
  MovieDecoder::Frame frame;
  if (movie.next(frame) != MovieDecoder::Result::error ||
      movie.last_error().find("decode unopened input:") != 0)
    return false;
  const auto missing = std::string(input) + "/missing";
  if (movie.open(missing.c_str()) || movie.last_error().find("open input:") != 0 ||
      movie.last_error().find('(') == std::string::npos)
    return false;
  if (!movie.open(input) || !movie.last_error().empty())
    return false;
  std::cout << "PASS media diagnostics retain the failed operation and FFmpeg error\n";
  return true;
}

}

int main(int argc, char **argv) {
  if (argc == 4) {
    MovieDecoder movie;
    return movie.open(argv[1]) && dump(movie, argv[2], argv[3]) ? 0 : 1;
  }
  const char *input = argc == 2 ? argv[1] : std::getenv("MOVIE_TEST_CLIP");
  if (!input || !diagnostics(input) || !inspect(input)) {
    std::cerr << "FAIL movie replay\n";
    return 1;
  }
  return 0;
}
