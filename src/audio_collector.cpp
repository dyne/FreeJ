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

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
//#include <sndfile.h>
#include <audio_collector.h>
#include <audio_jack.h>

#include <jutils.h>

static const int MAX_FFT_LENGTH = 4096;
static int XRanges[NUM_BARS+1] = {0, 1, 2, 3, 5, 7, 10, 14, 20, 28, 40, 54, 74, 101, 137, 187, 255};

FFT::FFT(int length) :
m_FFTLength(length),
#ifndef __FFTWFLOAT__
m_In(new double[length]),
#else
m_In(new float[length]),
#endif

#ifndef __FFTWFLOAT__
m_Spectrum(new fftw_complex[length])
{
	m_Plan = fftw_plan_dft_r2c_1d(m_FFTLength, m_In, m_Spectrum, FFTW_ESTIMATE);
}
#else
m_Spectrum(new fftwf_complex[length])
{
	m_Plan = fftwf_plan_dft_r2c_1d(m_FFTLength, m_In, m_Spectrum, FFTW_ESTIMATE);
}
#endif

FFT::~FFT()
{
	delete[] m_In;
#ifndef __FFTWFLOAT__
	fftw_destroy_plan(m_Plan);
#else
	fftwf_destroy_plan(m_Plan);
#endif
}

void FFT::Impulse2Freq(float *imp, float *out)
{
  unsigned int i;

  for (i=0; i<m_FFTLength; i++)
  {
    m_In[i] = imp[i];
  }

#ifndef __FFTWFLOAT__
  fftw_execute(m_Plan);
#else
  fftwf_execute(m_Plan);
#endif

  for (i=0; i<m_FFTLength; i++)
  {
    out[i] = m_Spectrum[i][0];
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

AudioCollector::AudioCollector(char *port, int n_BufferLength, unsigned int n_Samplerate, int FFTBuffers) :
m_Gain(1),
m_SmoothingBias(1.2),
m_FFT(n_BufferLength),
m_FFTBuffers(FFTBuffers),
m_JackBuffer(NULL),
m_OSSBuffer(NULL),
m_OneOverSHRT_MAX(1/(float)SHRT_MAX),
m_Processing(false),
m_ProcessPos(0)
{
  buffersize = n_BufferLength;
  samplerate = n_Samplerate;
	m_BufferTime = buffersize/(float)samplerate;
	
	m_Buffer = (float*) malloc(buffersize * sizeof(float));
	memset(m_Buffer,0,buffersize*sizeof(float));
	
	m_FFTBuffer = (float*) malloc(buffersize*m_FFTBuffers*sizeof(float));
	memset(m_FFTBuffer,0,buffersize*sizeof(float));
	
	m_JackBuffer = (float*) malloc(buffersize*sizeof(float));
	memset(m_JackBuffer,0,buffersize*sizeof(float));
	
	m_AudioBuffer = (float*) malloc(buffersize*sizeof(float));
	memset(m_AudioBuffer,0,buffersize*sizeof(float));
	
	m_FFTOutput = new float[NUM_BARS];
	for (int n=0; n<NUM_BARS; n++) m_FFTOutput[n]=0;
	
	m_Mutex = new pthread_mutex_t;
	pthread_mutex_init(m_Mutex,NULL);
	
	Jack = JackClient::Get();
	Jack->SetCallback(AudioCallback,(void*)this);
	Jack->Attach("freej");
	if (Jack->IsAttached())
	{	
		int id=Jack->AddInputPort();
		Jack->SetInputBuf(id,m_JackBuffer);
		Jack->ConnectInput(id, port);
	}
	else
	{
	  error("Could not attach to jack");
	}
	Jack->m_SampleRate = samplerate;
	Jack->m_BufferSize = buffersize;
	attached = true;
	  
}

AudioCollector::~AudioCollector()
{
	JackClient::Get()->Detach();
	free(m_Buffer);
	free(m_FFTBuffer);
	free(m_JackBuffer);
	free(m_AudioBuffer);
}

bool AudioCollector::IsConnected()
{
	return JackClient::Get()->IsAttached();
}

float AudioCollector::GetHarmonic(int h)
{
	return  m_FFTOutput[h%NUM_BARS];
}

float *AudioCollector::GetFFT()
{
	if (m_Processing)
	{
		if (m_ProcessPos+buffersize<m_ProcessLength)
		{
			m_FFT.Impulse2Freq(m_ProcessBuffer+m_ProcessPos,m_FFTBuffer);
			m_ProcessPos+=buffersize;
		}
		else
		{
			// finished, so clean up...
			delete[] m_ProcessBuffer;
			m_ProcessPos=0;
			m_Processing=false;
		}
	}
	else
	{
		pthread_mutex_lock(m_Mutex);
		jmemcpy((void*)m_AudioBuffer,(void*)m_Buffer,buffersize*sizeof(float));
		pthread_mutex_unlock(m_Mutex);
		
		m_FFT.Impulse2Freq(m_AudioBuffer,m_FFTBuffer);
	}
	
	
	for (int n=0; n<NUM_BARS; n++)
	{
		float Value = 0;
	 	for (int i=XRanges[n]; i<XRanges[n+1]; i++)
	 	{			
			Value += m_FFTBuffer[i];
		}
		Value*=Value;
		Value*=m_Gain*0.025;
		m_FFTOutput[n]=((m_FFTOutput[n]*m_SmoothingBias)+Value*(1/m_SmoothingBias))/2.0f;
	}
	
	/*for (int n=1; n<NUM_BARS-1; n++)
	{
		m_FFTOutput[n]=(m_FFTOutput[n-1]+m_FFTOutput[n]+m_FFTOutput[n+1])/3.0f;
	} */
	
	return m_FFTOutput;
}
/*
void AudioCollector::Process(const string &filename)
{
	if (m_Processing) return;
	
	SF_INFO info;
	info.format=0;
	
	SNDFILE* file = sf_open (filename.c_str(), SFM_READ, &info) ;
	if (!file) {
	  error("Error opening [%s] : %s", filename, sf_strerror(file));
	}
	
	m_ProcessBuffer = new float[info.frames];
	memset((void*)m_ProcessBuffer,0,info.frames*sizeof(float));
	m_ProcessLength=info.frames;
	
	// mix down to mono if need be
	if (info.channels>1)
	{
		float *Buffer = new float[info.frames*info.channels];
		sf_readf_float(file,Buffer,info.frames*info.channels);
		int from=0;
		for (int n=0; n<info.frames; n++)
		{
			for (int c=0; c<info.channels; c++)
			{
				m_ProcessBuffer[n]=(m_ProcessBuffer[n]+Buffer[from++])/2.0f;
			}
		}
	}
	else
	{
		sf_readf_float(file, m_ProcessBuffer, info.frames);	
	}
	sf_close(file);
	
	m_Processing=true;
	m_ProcessPos=0;
}
*/
   
void AudioCollector::get_audio(void *dest) {
  if (!pthread_mutex_trylock(m_Mutex))
    {
      jmemcpy(dest, (void*)m_Buffer, buffersize*sizeof(float));
      pthread_mutex_unlock(m_Mutex);
    }
}
     
void AudioCollector::AudioCallback_i(unsigned int Size)
{
	if (Size<=buffersize && !pthread_mutex_trylock(m_Mutex))
	{
	  jmemcpy((void*)m_Buffer,(void*)m_JackBuffer,buffersize*sizeof(float));
	  pthread_mutex_unlock(m_Mutex);
	}
}

void AudioCollector::AudioCallback(void *Context, unsigned int Size)
{
	((AudioCollector*)Context)->AudioCallback_i(Size);
}

#endif
