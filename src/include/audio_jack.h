// Copyright (C) 2003 David Griffiths <dave@pawfal.org>
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

#ifndef JACK_CLIENT
#define JACK_CLIENT

#include <array>
#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
#include <iostream>
#include <jack/jack.h>
#include <ringbuffer.h>
#include <audio_capture.h>

typedef jack_default_audio_sample_t sample_t;

const int MAX_INPUTPORTS = 256;
const int MAX_OUTPUTPORTS = 256;

struct RecordingRings {
	using Ring = std::unique_ptr<ringbuffer_t, decltype(&ringbuffer_free)>;
	Ring input{ringbuffer_create(4096 * 512 * 4), ringbuffer_free};
	Ring mix{ringbuffer_create(4096 * 512 * 4), ringbuffer_free};
	std::atomic<unsigned> rate{0}; // nonzero while an encoder owns the read cursors
	std::atomic<unsigned> channels{1};
};

class JackClient
{
public:
	static JackClient *Get()      { if(!m_Singleton) m_Singleton=new JackClient; return m_Singleton; }
	static void PackUpAndGoHome() { if(m_Singleton)  { delete m_Singleton; m_Singleton=NULL; } }
	
	bool   Attach(const std::string &ClientName);
	void   Detach();
	bool   IsAttached();
	unsigned Session() const { return session_.load(); }
	void   SetCallback(void(*run)(void*, unsigned int), void *context);
	void   ClearCallback(void *context);
	void   GetPortNames(std::vector<std::string> &InputNames,std::vector<std::string> &OutputNames);
	void   ConnectInput(int n, const std::string &JackPort);
	void   ConnectOutput(int n, const std::string &JackPort);
	void   DisconnectInput(int n);
	void   DisconnectOutput(int n);
	std::string GetInputName(int id) {
		std::lock_guard<std::mutex> lock(input_mutex_);
		auto port = m_InputPortMap.find(id);
		return port == m_InputPortMap.end() ? "" : port->second->Name;
	}
	std::string GetOutputName(int id) {
		std::lock_guard<std::mutex> lock(output_mutex_);
		auto port = m_OutputPortMap.find(id);
		return port == m_OutputPortMap.end() ? "" : port->second->Name;
	}
	void   SetInputBuf(int ID, float* s);
	void   SetOutputBuf(int ID, float* s);
	int AddInputPort(std::shared_ptr<AudioCapture> capture = {});
	void RemoveInputPort(int id);
         int 	 AddOutputPort();
	
	int SetRingbufferPtr(ringbuffer_t *rb, int samplerate, int channels);
	static void ResetRingbuffer(ringbuffer_t *rb, bool disconnect);
	static std::atomic<unsigned long> m_BufferSize, m_SampleRate;
	static std::atomic<bool> m_Attached;
	ringbuffer_t *audio_mix_ring;	//ringbuffer to be streamed
	ringbuffer_t *first;
	std::atomic<int> m_ringbufferchannels;
	void	isEncoded(bool isEnc);
	std::shared_ptr<RecordingRings> BeginRecording();
	
protected:
	JackClient();
	~JackClient();

	static int  Process(jack_nframes_t nframes, void *o);
	static int  OnSRateChange(jack_nframes_t n, void *o);
	static void OnJackShutdown(void *o);

private:


	class JackPort
	{		
		public:
		JackPort() :
			Connected(false),Buf(NULL),Port(NULL) {}
		
		std::string         Name;
		bool           Connected;
		float*         Buf;
		jack_port_t*   Port;
		std::string         ConnectedTo;
		std::shared_ptr<AudioCapture> capture;
	};

	ringbuffer_t*      m_ringbuffer;
	std::mutex output_mutex_;
	std::vector<char> output_buffer_;
	

	static JackClient*        m_Singleton;
	static jack_client_t*     m_Client;
	static std::map<int,JackPort*> m_InputPortMap;
	static std::map<int,JackPort*> m_OutputPortMap;
	int m_NextInputID;
	// Append under output_mutex_. The callback reads only the published prefix.
	std::array<jack_port_t *, MAX_OUTPUTPORTS> output_ports_{};
	std::atomic<int> output_port_count_{0};
	struct InputPorts {
		std::array<JackPort *, MAX_INPUTPORTS> ports{};
		size_t count = 0;
	};
	// Only the control thread changes the inactive list. The JACK process
	// callback reads one immutable, registration-ordered list per cycle.
	std::mutex input_mutex_;
	std::array<InputPorts, 2> input_lists_;
	std::atomic<unsigned> active_inputs_{0};
	std::atomic<unsigned> input_epoch_{0};
	void publish_inputs_locked();
	std::atomic<unsigned> session_{0};
	std::atomic<bool> shutdown_pending_{false};
	std::recursive_mutex callback_mutex_;
	void dispatch_shutdown();
	int add_output_port_locked();
	void detach_locked();
	
	static void(*RunCallback)(void*, unsigned int bufsize);
	static void *RunContext;
	std::atomic<bool> m_Encoded;
	std::shared_ptr<RecordingRings> recording_;
};

#endif
