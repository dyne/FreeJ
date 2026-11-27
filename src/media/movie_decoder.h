#ifndef FREEJ_MOVIE_DECODER_H
#define FREEJ_MOVIE_DECODER_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace freej_media {

class MovieDecoder {
public:
  enum class Result { video, audio, again, end, error };

  struct Frame {
    std::vector<std::uint32_t> pixels;
    std::vector<float> samples;
    std::int64_t timestamp = 0; // Microseconds on the common media timeline.
  };

  MovieDecoder();
  ~MovieDecoder();
  bool open(const char *path);
  bool seek(std::int64_t timestamp);
  Result next(Frame &frame);
  const std::string &last_error() const { return last_error_; }
  int width() const;
  int height() const;
  double frame_rate() const;
  int channels() const;
  int sample_rate() const;
  int output_sample_rate() const;
  void set_output_sample_rate(int rate);
  std::int64_t duration() const;

private:
  struct State;
  std::string last_error_;
  std::unique_ptr<State> state_;
};

}

#endif
