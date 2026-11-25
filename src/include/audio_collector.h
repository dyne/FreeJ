// Copyright (C) 2005 Dave Griffiths
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef AUDIO_COLLECTOR
#define AUDIO_COLLECTOR

#include <fftw3.h>
#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <audio_capture.h>

class JackClient;

static const int NUM_BARS = 16;

class FFT {
public:
  explicit FFT(int length);
  ~FFT();
  FFT(const FFT &) = delete;
  FFT &operator=(const FFT &) = delete;

  int bins() const { return m_FFTLength / 2 + 1; }
  void Impulse2Freq(float *imp, float *out);

private:
  const int m_FFTLength;
#ifndef __FFTWFLOAT__
  std::vector<double> m_In;
  std::unique_ptr<fftw_complex[]> m_Spectrum;
  fftw_plan m_Plan;
#else
  std::vector<float> m_In;
  std::unique_ptr<fftwf_complex[]> m_Spectrum;
  fftwf_plan m_Plan;
#endif
};

class AudioCollector {
public:
  AudioCollector(char *port, int BufferLength, unsigned int Samplerate, int FFTBuffers = 1);
  AudioCollector(int BufferLength, unsigned int Samplerate, JackClient *, int FFTBuffers = 1);
  ~AudioCollector();

  // Called by an analysis consumer outside the device callback. Only complete
  // non-overlapping windows become visible to GetFFT and get_audio.
  void SubmitSamples(const float *samples, size_t count);
  float *GetFFT();
  float *GetAudioBuffer() { return m_AudioBuffer.data(); }
  float GetHarmonic(int h);
  bool IsConnected();
  int AddAnalysisInput();
  bool CapturePosition(AudioCapture::Position &position) const {
    if (!m_HasCapture) return false;
    position = m_Position;
    return true;
  }
  void SetGain(float s) { m_Gain = s; }
  void SetSmoothingBias(float s) { if (s < 2 && s > 0) m_SmoothingBias = s; }
  bool IsProcessing() { return false; }
  float BufferTime() { return m_BufferTime; }
  void get_audio(void *buffer);

  int samplerate;
  int buffersize;
  bool attached;
  JackClient *Jack;

private:
  using Bands = std::array<float, NUM_BARS>;
  void analyze(float *samples, Bands &power);
  void analysis_loop();
  bool m_OwnsConnection = false;
  int m_InputPort = -1;
  unsigned m_Session = 0;
  std::shared_ptr<AudioCapture> m_Capture;
  AudioCapture::Position m_Position;
  bool m_HasCapture = false;
  AudioCapture::Position m_LatestPosition;
  bool m_LatestLive = false;
  std::thread m_Worker;
  std::atomic<bool> m_Stop{false};
  float m_Gain = 1;
  float m_SmoothingBias = 1.2f;
  float m_BufferTime;
  FFT m_FFT;
  std::mutex m_Mutex;
  std::mutex m_AnalysisMutex;
  std::vector<float> m_Pending;
  size_t m_PendingSize = 0;
  std::vector<float> m_Buffer;
  std::vector<float> m_WorkSamples;
  std::vector<float> m_AudioBuffer;
  std::vector<float> m_FFTBuffer;
  Bands m_LatestPower{}, m_Power{};
  float m_FFTOutput[NUM_BARS] = {};
};

#endif
