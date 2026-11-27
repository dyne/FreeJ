#include <audio_collector.h>
#include <audio_pipewire.h>
#include <algorithm>
#include <array>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Supply the audio device control boundary so both public constructors and disconnect
// can be exercised without a device. PCM enters on the analysis thread.
PipeWireClient *PipeWireClient::m_Singleton = nullptr;
std::atomic<bool> PipeWireClient::m_Attached{false};
std::atomic<unsigned long> PipeWireClient::m_SampleRate{48000}, PipeWireClient::m_BufferSize{256};
static bool server_available = false;
static int ports = 0, removed = 0;
struct PipeWireClient::Graph {};
PipeWireClient::PipeWireClient() {}
PipeWireClient::~PipeWireClient() {}
bool PipeWireClient::IsAttached() { return m_Attached.load(); }
bool PipeWireClient::Attach(const std::string &) { return m_Attached = server_available; }
void PipeWireClient::Detach() { m_Attached = false; }
int PipeWireClient::AddInputPort(std::shared_ptr<AudioCapture>) { return ports++; }
void PipeWireClient::ConnectInput(int, const std::string &) {}
void PipeWireClient::RemoveInputPort(int) { ++removed; }
void error(const char *, ...) {}

namespace {
constexpr int edges[] = {0,1,2,3,5,7,10,14,20,28,40,54,74,101,137,187,255};
int checks = 0;

void check(bool pass, const char *label) {
  ++checks;
  if (!pass) {
    std::fprintf(stderr, "FAIL %s at check %d\n", label, checks);
    std::exit(1);
  }
}

void response(AudioCollector &collector, int period) {
  const int n = collector.buffersize;
  const double tau = 2 * std::acos(-1.);
  std::array<double, NUM_BARS> before{};
  float gain = 1, bias = 1.2f;
  // Include fractional and quarter turns: taking FFT magnitude instead of
  // the real component changes these cases even when the tone bin is right.
  for (int window = 0; window < 8; ++window) {
    std::vector<float> pcm(n, 0);
    int tone = window == 5 ? 28 : 20;
    const double phase = (window - 3) * tau / 8;
    if (window == 1 || window == 2)
      pcm[0] = 1;
    else if (window >= 3 && window < 7)
      for (int s = 0; s < n; ++s)
        pcm[s] = std::cos(tau * tone * s / n + phase);
    if (window == 6) {
      gain = 1.7f;
      bias = 0.75f;
      collector.SetGain(gain);
      collector.SetSmoothingBias(bias);
      collector.SetSmoothingBias(0); // rejected, keep 0.75
    }
    for (int pos = 0; pos < n; pos += period) {
      const auto count = std::min(period, n - pos);
      collector.SubmitSamples(pcm.data() + pos, count);
      if (pos + count < n) {
        std::vector<float> published(n);
        collector.get_audio(published.data());
        if (window == 1)
          check(std::all_of(published.begin(), published.end(), [](float x) { return x == 0; }),
                "incomplete impulse stays unpublished");
      }
    }
    auto *bands = collector.GetFFT();
    for (int band = 0; band < NUM_BARS; ++band) {
      const int last = std::min(edges[band + 1], n / 2 + 1);
      double sum = 0;
      if (window == 1 || window == 2)
        sum = std::max(0, last - edges[band]);
      else if (window >= 3 && window < 7 && edges[band] <= tone && tone < last)
        sum = n * std::cos(phase) / 2.;
      const double expected = (before[band] * bias + sum * sum * gain * 0.025 / bias) / 2;
      check(std::abs(bands[band] - expected) <= 1e-5 * std::max(1., std::abs(expected)),
            "16-band analytical response");
      check(collector.GetHarmonic(band + 16) == bands[band] &&
            collector.GetHarmonic(band - 32) == bands[band], "harmonic wrap");
      before[band] = expected;
    }
    check(collector.GetHarmonic(INT_MIN) == bands[0] &&
          collector.GetHarmonic(INT_MAX) == bands[15], "extreme indices");
  }
}
}

int main() {
  for (int n : {1, 127, 256, 2048}) {
    FFT fft(n);
    std::vector<float> pcm(n), bins(n / 2 + 2, -17);
    pcm[0] = 1;
    fft.Impulse2Freq(pcm.data(), bins.data());
    check(bins.back() == -17, "one-past-bin sentinel");
    for (int b = 0; b < n / 2 + 1; ++b)
      check(std::abs(bins[b] - 1) <= 1e-6, "unit impulse");
  }
  auto *jack = PipeWireClient::Get();
  char port[] = "test:pulse";
  {
    AudioCollector offline(port, 127, 48000);
    check(!offline.attached && !offline.IsConnected(), "offline startup");
    response(offline, 32);
  }
  server_available = true;
  {
    int connections = 0;
    for (int period : {2048, 256}) {
      const int rate = period == 2048 ? 44100 : 48000;
      {
        AudioCollector owned(port, 2048, rate);
        check(owned.IsConnected() && ports == ++connections, "owned connection");
        check(jack->m_BufferSize == 256 && jack->m_SampleRate == 48000, "server clock preserved");
        response(owned, period);
        {
          AudioCollector borrowed(2048, rate, jack);
          response(borrowed, period);
          AudioCollector on_existing(port, 2048, rate);
          check(on_existing.IsConnected() && ports == ++connections, "existing connection");
        }
        check(jack->IsAttached() && removed == connections / 2, "borrowers leave owner attached");
      }
      check(!jack->IsAttached(), "owner released connection");
    }
  }
  jack->Attach("shared");
  {
    AudioCollector borrowed(1024, 48000, jack);
    response(borrowed, 256);
    jack->Detach();
    check(!borrowed.IsConnected(), "borrower observes external disconnect");
  }
  PipeWireClient::PackUpAndGoHome();
  std::printf("PASS %d collector checks\n", checks);
}
