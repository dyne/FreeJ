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

#include <config.h>
#ifdef WITH_AUDIO
#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <audio_collector.h>
#include <audio_jack.h>
#include <logging.h>

static const int XRanges[NUM_BARS + 1] = {
  0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 74, 101, 137, 187, 255
};

static int checked_length(int length)
{
  if (length <= 0)
    throw std::invalid_argument("FFT length must be positive");
  return length;
}

static float window_seconds(int length, unsigned int rate)
{
  if (!rate)
    throw std::invalid_argument("sample rate must be positive");
  return length / static_cast<float>(rate);
}

FFT::FFT(int length) : m_FFTLength(checked_length(length)), m_In(length),
#ifndef __FFTWFLOAT__
  m_Spectrum(new fftw_complex[bins()]),
  m_Plan(fftw_plan_dft_r2c_1d(length, m_In.data(), m_Spectrum.get(), FFTW_ESTIMATE))
#else
  m_Spectrum(new fftwf_complex[bins()]),
  m_Plan(fftwf_plan_dft_r2c_1d(length, m_In.data(), m_Spectrum.get(), FFTW_ESTIMATE))
#endif
{
  if (!m_Plan)
    throw std::bad_alloc();
}

FFT::~FFT()
{
#ifndef __FFTWFLOAT__
  fftw_destroy_plan(m_Plan);
#else
  fftwf_destroy_plan(m_Plan);
#endif
}

void FFT::Impulse2Freq(float *imp, float *out)
{
  std::copy_n(imp, m_FFTLength, m_In.begin());
#ifndef __FFTWFLOAT__
  fftw_execute(m_Plan);
#else
  fftwf_execute(m_Plan);
#endif
  for (int i = 0; i < bins(); ++i)
    out[i] = m_Spectrum[i][0];
}

AudioCollector::AudioCollector(int length, unsigned int rate, JackClient *jack, int)
  : samplerate(rate), buffersize(length), attached(jack && jack->IsAttached()), Jack(jack),
    m_Session(attached ? jack->Session() : 0),
    m_BufferTime(window_seconds(length, rate)), m_FFT(length),
    m_Pending(length), m_Buffer(length), m_WorkSamples(length), m_AudioBuffer(length), m_FFTBuffer(m_FFT.bins())
{
  m_Capture = std::make_shared<AudioCapture>(length);
  if (attached && jack->m_SampleRate.load()) {
    samplerate = jack->m_SampleRate.load();
    m_BufferTime = window_seconds(length, samplerate);
  }
}

AudioCollector::AudioCollector(char *port, int length, unsigned int rate, int fft_buffers)
  : AudioCollector(length, rate, JackClient::Get(), fft_buffers)
{
  if (!attached) {
    attached = Jack->Attach("freej");
    m_OwnsConnection = attached;
  }
  if (attached) {
    m_Session = Jack->Session();
    if (Jack->m_SampleRate.load()) {
      samplerate = Jack->m_SampleRate.load();
      m_BufferTime = window_seconds(length, samplerate);
    }
    m_InputPort = AddAnalysisInput();
    if (m_InputPort >= 0) {
      Jack->ConnectInput(m_InputPort, port);
    } else {
      if (m_OwnsConnection) Jack->Detach();
      attached = false;
      error("Could not register JACK analysis input");
    }
  } else {
    error("Could not attach to jack");
  }
}

AudioCollector::~AudioCollector()
{
  if (m_Worker.joinable()) {
    m_Stop = true;
    m_Capture->wake();
    m_Worker.join();
  }
  if (!attached || !Jack || m_Session != Jack->Session()) return;
  if (m_OwnsConnection)
    Jack->Detach();
  else if (m_InputPort >= 0)
    Jack->RemoveInputPort(m_InputPort);
}

bool AudioCollector::IsConnected()
{
  return attached && Jack->IsAttached() && Jack->Session() == m_Session;
}

int AudioCollector::AddAnalysisInput()
{
  if (!IsConnected() || m_InputPort >= 0) return m_InputPort;
  if (!m_Capture->enable_wakeup()) return -1;
  try {
    m_Worker = std::thread(&AudioCollector::analysis_loop, this);
  } catch (const std::system_error &) {
    return -1;
  }
  m_InputPort = Jack->AddInputPort(m_Capture);
  if (m_InputPort < 0) {
    m_Stop = true;
    m_Capture->wake();
    m_Worker.join();
    m_Stop = false;
  }
  return m_InputPort;
}

void AudioCollector::SubmitSamples(const float *samples, size_t count)
{
  std::lock_guard<std::mutex> analysis(m_AnalysisMutex);
  while (count) {
    const size_t n = std::min(count, m_Pending.size() - m_PendingSize);
    std::copy_n(samples, n, m_Pending.begin() + m_PendingSize);
    samples += n;
    count -= n;
    m_PendingSize += n;
    if (m_PendingSize == m_Pending.size()) {
      Bands power;
      analyze(m_Pending.data(), power);
      std::lock_guard<std::mutex> lock(m_Mutex);
      m_Pending.swap(m_Buffer);
      m_LatestPower = power;
      m_LatestPosition.generation = m_Capture->generation();
      m_LatestLive = false;
      m_PendingSize = 0;
    }
  }
}

void AudioCollector::analysis_loop()
{
  while (m_Capture->wait() && !m_Stop.load()) {
    AudioCapture::Position position;
    if (!m_Capture->read(m_WorkSamples.data(), position)) continue;
    Bands power;
    {
      std::lock_guard<std::mutex> analysis(m_AnalysisMutex);
      analyze(m_WorkSamples.data(), power);
    }
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_WorkSamples.swap(m_Buffer);
    m_LatestPower = power;
    m_LatestPosition = position;
    m_LatestLive = true;
  }
}

void AudioCollector::analyze(float *samples, Bands &power)
{
  m_FFT.Impulse2Freq(samples, m_FFTBuffer.data());
  for (int band = 0; band < NUM_BARS; ++band) {
    float sum = 0;
    const int end = std::min(XRanges[band + 1], m_FFT.bins());
    for (int bin = XRanges[band]; bin < end; ++bin) sum += m_FFTBuffer[bin];
    power[band] = sum * sum;
  }
}

float AudioCollector::GetHarmonic(int h)
{
  const int index = h % NUM_BARS;
  return m_FFTOutput[index < 0 ? index + NUM_BARS : index];
}

float *AudioCollector::GetFFT()
{
  const bool connected = IsConnected();
  const auto generation = m_Capture->generation();
  if (generation != m_Position.generation || (m_HasCapture && !connected)) {
    std::fill_n(m_FFTOutput, NUM_BARS, 0.f);
    m_Power.fill(0);
    std::fill(m_AudioBuffer.begin(), m_AudioBuffer.end(), 0.f);
    m_HasCapture = false;
    m_Position.generation = generation;
  }
  {
    std::unique_lock<std::mutex> lock(m_Mutex, std::try_to_lock);
    if (lock && m_LatestPosition.generation == generation && (!m_LatestLive || connected)) {
      m_Power = m_LatestPower;
      m_Position = m_LatestPosition;
      m_HasCapture = m_LatestLive;
      std::copy(m_Buffer.begin(), m_Buffer.end(), m_AudioBuffer.begin());
      if (m_HasCapture) {
        samplerate = m_Position.rate;
        m_BufferTime = window_seconds(buffersize, samplerate);
      }
    }
  }
  for (int band = 0; band < NUM_BARS; ++band) {
    float value = m_Power[band];
    value *= m_Gain * 0.025;
    m_FFTOutput[band] = ((m_FFTOutput[band] * m_SmoothingBias) +
                         value * (1 / m_SmoothingBias)) / 2.0f;
  }
  return m_FFTOutput;
}

void AudioCollector::get_audio(void *dest)
{
  const bool connected = IsConnected();
  std::unique_lock<std::mutex> lock(m_Mutex, std::try_to_lock);
  if (lock) {
    if (m_LatestLive && (!connected || m_LatestPosition.generation != m_Capture->generation()))
      std::memset(dest, 0, m_Buffer.size() * sizeof(float));
    else
      std::memcpy(dest, m_Buffer.data(), m_Buffer.size() * sizeof(float));
  }
}

#endif
