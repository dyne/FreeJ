#include "movie_decoder.h"

#include <algorithm>
#include <cstring>
#include <limits>
#include <mutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/mathematics.h>
#include <libswresample/swresample.h>
#include <libswscale/swscale.h>
}

namespace freej_media {
namespace {

template<typename T, void (*Release)(T **)>
struct ReleaseMedia {
  void operator()(T *value) const { Release(&value); }
};

using Input = std::unique_ptr<AVFormatContext, ReleaseMedia<AVFormatContext, avformat_close_input>>;
using Codec = std::unique_ptr<AVCodecContext, ReleaseMedia<AVCodecContext, avcodec_free_context>>;
using Packet = std::unique_ptr<AVPacket, ReleaseMedia<AVPacket, av_packet_free>>;
using Picture = std::unique_ptr<AVFrame, ReleaseMedia<AVFrame, av_frame_free>>;
using Resampler = std::unique_ptr<SwrContext, ReleaseMedia<SwrContext, swr_free>>;
using Scaler = std::unique_ptr<SwsContext, decltype(&sws_freeContext)>;

constexpr AVRational microseconds{1, AV_TIME_BASE};

struct Stream {
  AVStream *info = nullptr;
  Codec codec;
  std::int64_t next_timestamp = 0;
  bool drain_sent = false;
  bool drained = false;

  bool open(AVFormatContext *input, AVMediaType type) {
    const AVCodec *decoder = nullptr;
    const int index = av_find_best_stream(input, type, -1, -1, &decoder, 0);
    if (index < 0)
      return false;
    AVStream *stream = input->streams[index];
    Codec context(avcodec_alloc_context3(decoder));
    if (!context || avcodec_parameters_to_context(context.get(), stream->codecpar) < 0)
      return false;
    context->pkt_timebase = stream->time_base;
    if (avcodec_open2(context.get(), decoder, nullptr) < 0)
      return false;
    info = stream;
    codec = std::move(context);
    return true;
  }

  void seek(std::int64_t timestamp) {
    if (codec)
      avcodec_flush_buffers(codec.get());
    next_timestamp = timestamp;
    drain_sent = drained = false;
  }
};

}

struct MovieDecoder::State {
  Input input;
  Stream video;
  Stream audio;
  Packet packet{av_packet_alloc()};
  Picture decoded{av_frame_alloc()};
  Picture converted{av_frame_alloc()};
  Scaler scale{nullptr, sws_freeContext};
  Resampler resample;
  Stream *receiving = nullptr;
  bool input_ended = false;
  bool packet_pending = false;
  bool audio_timed = false;
  int width = 0;
  int height = 0;
  int output_rate = 0;
  std::int64_t origin = 0;
  std::int64_t discard_before = 0;
  std::int64_t audio_timestamp = 0;
  std::int64_t audio_samples = 0;
  AVRational rate{25, 1};

  Result video_frame(Frame &frame, const AVFrame &source, std::int64_t timestamp) {
    video.next_timestamp = av_sat_add64(timestamp, av_rescale_q(1, av_inv_q(rate), microseconds));
    if (timestamp < discard_before)
      return Result::again;
    // A fixed layer geometry is shared with the compositor for the lifetime of an open source.
    if (source.width != width || source.height != height)
      return Result::error;
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    constexpr auto output = AV_PIX_FMT_ARGB;
#else
    constexpr auto output = AV_PIX_FMT_BGRA;
#endif
    scale.reset(sws_getCachedContext(scale.release(), width, height,
                                   static_cast<AVPixelFormat>(source.format),
                                   width, height, output, SWS_BICUBIC, nullptr, nullptr, nullptr));
    if (!scale)
      return Result::error;
    if (!converted->data[0]) {
      converted->format = output;
      converted->width = width;
      converted->height = height;
      if (av_frame_get_buffer(converted.get(), 32) < 0)
        return Result::error;
    }
    if (sws_scale(scale.get(), source.data, source.linesize, 0, height,
                  converted->data, converted->linesize) != height)
      return Result::error;
    frame.pixels.resize(static_cast<std::size_t>(width) * height);
    for (int y = 0; y < height; ++y) {
      const auto *row = converted->data[0] + y * converted->linesize[0];
      std::memcpy(frame.pixels.data() + static_cast<std::size_t>(y) * width, row, width * 4);
    }
    frame.timestamp = timestamp;
    return Result::video;
  }

  Result audio_frame(Frame &frame, const AVFrame *source, std::int64_t timestamp) {
    if (!resample) {
      if (!source)
        return Result::end;
      SwrContext *converter = nullptr;
      const int status = swr_alloc_set_opts2(&converter, &audio.codec->ch_layout,
                    AV_SAMPLE_FMT_FLT, output_rate, &audio.codec->ch_layout,
                    static_cast<AVSampleFormat>(source->format), source->sample_rate, 0, nullptr);
      resample.reset(converter);
      if (status < 0 || swr_init(resample.get()) < 0)
        return Result::error;
    }

    const int channels = audio.codec->ch_layout.nb_channels;
    if (source && (source->sample_rate != audio.codec->sample_rate ||
        source->ch_layout.nb_channels != channels))
      return Result::error;
    if (!audio_timed || (source && swr_get_delay(resample.get(), output_rate) == 0 &&
        timestamp != audio.next_timestamp)) {
      audio_timestamp = timestamp;
      audio_samples = 0;
      audio_timed = true;
    }
    const int input_samples = source ? source->nb_samples : 0;
    if (source)
      audio.next_timestamp = av_sat_add64(timestamp,
          av_rescale_q(input_samples, {1, source->sample_rate}, microseconds));

    const int capacity = swr_get_out_samples(resample.get(), input_samples);
    if (capacity < 0)
      return Result::error;
    frame.samples.resize(static_cast<std::size_t>(capacity) * channels);
    uint8_t *output[] = {reinterpret_cast<uint8_t *>(frame.samples.data())};
    const int produced = swr_convert(resample.get(), output, capacity,
        source ? const_cast<const uint8_t **>(source->extended_data) : nullptr, input_samples);
    if (produced < 0)
      return Result::error;
    if (!produced)
      return source ? Result::again : Result::end;

    const auto time_of_sample = [&](std::int64_t offset) {
      return av_sat_add64(audio_timestamp, av_rescale_q(offset, {1, output_rate}, microseconds));
    };
    frame.timestamp = time_of_sample(audio_samples);
    const auto skip = std::min<std::int64_t>(produced,
        frame.timestamp < discard_before
        ? av_rescale_rnd(av_sat_sub64(discard_before, frame.timestamp), output_rate, AV_TIME_BASE, AV_ROUND_UP)
        : 0);
    frame.samples.resize(static_cast<std::size_t>(produced) * channels);
    if (skip)
      frame.samples.erase(frame.samples.begin(), frame.samples.begin() + skip * channels);
    frame.timestamp = time_of_sample(audio_samples + skip);
    audio_samples += produced;
    return skip == produced ? Result::again : Result::audio;
  }
};

MovieDecoder::MovieDecoder() = default;
MovieDecoder::~MovieDecoder() = default;

bool MovieDecoder::open(const char *path) {
  static std::once_flag network_init;
  std::call_once(network_init, [] { avformat_network_init(); });

  std::unique_ptr<State> state(new State);
  AVFormatContext *input = nullptr;
  if (!path || avformat_open_input(&input, path, nullptr, nullptr) < 0)
    return false;
  state->input.reset(input);
  if (!state->packet || !state->decoded || !state->converted ||
      avformat_find_stream_info(input, nullptr) < 0 ||
      !state->video.open(input, AVMEDIA_TYPE_VIDEO))
    return false;
  const auto &video = *state->video.codec;
  if (video.width <= 0 || video.height <= 0 ||
      video.width > std::numeric_limits<uint16_t>::max() / 4 ||
      video.height > std::numeric_limits<uint16_t>::max())
    return false;

  state->width = video.width;
  state->height = video.height;

  if (state->audio.open(input, AVMEDIA_TYPE_AUDIO)) {
    if (state->audio.codec->sample_rate <= 0 ||
        !av_channel_layout_check(&state->audio.codec->ch_layout))
      return false;
    auto &layout = state->audio.codec->ch_layout;
    if (layout.order == AV_CHANNEL_ORDER_UNSPEC)
      av_channel_layout_default(&layout, layout.nb_channels);
    state->output_rate = state->audio.codec->sample_rate;
  }

  const auto rate = av_guess_frame_rate(input, state->video.info, nullptr);
  if (rate.num > 0 && rate.den > 0)
    state->rate = rate;
  if (input->start_time != AV_NOPTS_VALUE)
    state->origin = input->start_time;
  state_ = std::move(state);
  return true;
}

bool MovieDecoder::seek(std::int64_t timestamp) {
  auto &state = *state_;
  auto *video = state.video.info;
  const auto target = av_sat_add64(state.origin, timestamp);
  auto decode_start = av_rescale_q(target, microseconds, video->time_base);
  if (timestamp > 0 && state.video.codec->has_b_frames) {
    // Container indexes can use DTS. Begin one keyframe earlier so frames
    // displayed before the selected keyframe can decode with both references.
    const int index = av_index_search_timestamp(video, decode_start, AVSEEK_FLAG_BACKWARD);
    for (int i = index - 1; i >= 0; --i) {
      const auto *entry = avformat_index_get_entry(video, i);
      if (entry && (entry->flags & AVINDEX_KEYFRAME)) {
        decode_start = entry->timestamp;
        break;
      }
    }
    if (index < 0) {
      const auto preroll = av_rescale_q(16, av_inv_q(state.rate), microseconds);
      const auto earlier = av_sat_sub64(target, std::min(timestamp, preroll));
      decode_start = av_rescale_q(earlier, microseconds, video->time_base);
    }
  }
  if (av_seek_frame(state.input.get(), video->index, decode_start, AVSEEK_FLAG_BACKWARD) < 0)
    return false;
  av_packet_unref(state.packet.get());
  av_frame_unref(state.decoded.get());
  state.video.seek(timestamp);
  state.audio.seek(timestamp);
  state.resample.reset();
  state.receiving = nullptr;
  state.input_ended = state.packet_pending = state.audio_timed = false;
  state.discard_before = timestamp;
  return true;
}

MovieDecoder::Result MovieDecoder::next(Frame &frame) {
  auto &state = *state_;
  frame.pixels.clear();
  frame.samples.clear();
  // Bound calls into the demuxer per tick, including streams with no video packets.
  for (int work = 0; work < 128; ++work) {
    if (Stream *stream = state.receiving) {
      const int status = avcodec_receive_frame(stream->codec.get(), state.decoded.get());
      if (status == 0) {
        const auto &source = *state.decoded;
        const auto pts = source.best_effort_timestamp;
        const auto timestamp = pts == AV_NOPTS_VALUE ? stream->next_timestamp :
            av_sat_sub64(av_rescale_q(pts, stream->info->time_base, microseconds), state.origin);
        const auto result = stream == &state.video ? state.video_frame(frame, source, timestamp)
                                                  : state.audio_frame(frame, &source, timestamp);
        av_frame_unref(state.decoded.get());
        if (result != Result::again)
          return result;
        continue;
      }
      state.receiving = nullptr;
      if (status == AVERROR_EOF)
        stream->drained = true;
      else if (status != AVERROR(EAGAIN))
        return Result::error;
    }

    if (!state.input_ended) {
      if (!state.packet_pending) {
        const int status = av_read_frame(state.input.get(), state.packet.get());
        if (status == AVERROR(EAGAIN))
          return Result::again;
        if (status == AVERROR_EOF) {
          state.input_ended = true;
          continue;
        }
        if (status < 0)
          return Result::error;
        state.packet_pending = true;
      }
      const int index = state.packet->stream_index;
      Stream *stream = index == state.video.info->index ? &state.video :
          state.audio.info && index == state.audio.info->index ? &state.audio : nullptr;
      if (stream) {
        const int status = avcodec_send_packet(stream->codec.get(), state.packet.get());
        if (status < 0 && status != AVERROR(EAGAIN))
          return Result::error;
        state.receiving = stream;
        if (status == AVERROR(EAGAIN))
          continue;
      }
      av_packet_unref(state.packet.get());
      state.packet_pending = false;
      continue;
    }

    Stream *stream = !state.video.drained ? &state.video :
                     state.audio.codec && !state.audio.drained ? &state.audio : nullptr;
    if (!stream)
      return state.audio_frame(frame, nullptr, state.audio.next_timestamp);
    if (!stream->drain_sent) {
      const int status = avcodec_send_packet(stream->codec.get(), nullptr);
      if (status < 0 && status != AVERROR(EAGAIN) && status != AVERROR_EOF)
        return Result::error;
      stream->drain_sent = status != AVERROR(EAGAIN);
    }
    state.receiving = stream;
  }
  return Result::again;
}

int MovieDecoder::width() const { return state_->width; }
int MovieDecoder::height() const { return state_->height; }
double MovieDecoder::frame_rate() const { return av_q2d(state_->rate); }
int MovieDecoder::channels() const { return state_->audio.codec ? state_->audio.codec->ch_layout.nb_channels : 0; }
int MovieDecoder::sample_rate() const { return state_->audio.codec ? state_->audio.codec->sample_rate : 0; }
int MovieDecoder::output_sample_rate() const { return state_->output_rate; }
std::int64_t MovieDecoder::duration() const { return state_->input->duration; }

void MovieDecoder::set_output_sample_rate(int rate) {
  if (rate > 0 && rate != state_->output_rate) {
    state_->output_rate = rate;
    state_->resample.reset();
    state_->audio_timed = false;
  }
}

}
