// Copyright (C) 2003 David Griffiths <dave@pawfal.org>
// adapted to FreeJ by jaromil
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

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <math.h>

#include <audio_jack.h>

#include <jutils.h>
#include <algorithm>
#include <cerrno>

JackClient*       JackClient::m_Singleton  = NULL;
bool              JackClient::m_Attached   = false;
long unsigned int JackClient::m_BufferSize = 0;
long unsigned int JackClient::m_SampleRate = 0;
void            (*JackClient::RunCallback)(void*, unsigned int BufSize)=NULL;
void             *JackClient::RunContext   = NULL;	
jack_client_t    *JackClient::m_Client     = NULL;
std::map<int,JackClient::JackPort*> JackClient::m_InputPortMap;
std::map<int,JackClient::JackPort*> JackClient::m_OutputPortMap;

///////////////////////////////////////////////////////

JackClient::JackClient() :
audio_mix_ring(nullptr),
first(nullptr),
m_ringbufferchannels(0),
m_ringbuffer(nullptr),
m_NextInputID(0),
m_Encoded(false)
{
}

/////////////////////////////////////////////////////////////////////////////////////////////

JackClient::~JackClient()	
{
	Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::isEncoded(bool enc)
{
  m_Encoded = enc;
}

bool JackClient::Attach(const std::string &ClientName)
{
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (m_Attached) return true;
	// A server shutdown can leave an inactive client to retire before reopening.
	if (m_Client)
	  detach_locked();

/* jack_client_new is now deprecated (Oct 2010)
   m_Client = jack_client_new(ClientName.c_str());
   
   if (!m_Client)
   {
   error("jack server not running? client_new returns %p", m_Client);
   return false;
   } */
        m_Client = 
            jack_client_open(ClientName.c_str(),
                             (jack_options_t) (JackNullOption | JackNoStartServer), NULL);
        if (!m_Client) {
            error("jack server not running?", m_Client);
            return false;
        }

	jack_set_process_callback(m_Client, JackClient::Process, this);
	jack_set_sample_rate_callback (m_Client, JackClient::OnSRateChange, 0);
	jack_on_shutdown (m_Client, JackClient::OnJackShutdown, this);

	m_InputPortMap.clear();
	m_OutputPortMap.clear();
	
	audio_mix_ring = ringbuffer_create(4096 * 512 * 4);
	first = ringbuffer_create(4096 * 512 * 4);
	if (!audio_mix_ring || !first || jack_activate(m_Client)) {
	  error("cannot activate JACK client");
	  detach_locked();
	  return false;
	}
	m_Attached = true;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::Detach()
{
	std::lock_guard<std::mutex> lock(output_mutex_);
	detach_locked();
}

void JackClient::detach_locked()
{
	if (m_Client)
	{
	  act("Detaching from JACK");
		jack_client_close(m_Client);
		m_Client=NULL;
		m_Attached=false;
	}
	if(audio_mix_ring) ringbuffer_free(audio_mix_ring);
	if(first) ringbuffer_free(first);
	audio_mix_ring = first = nullptr;
	m_ringbuffer = nullptr;
	m_ringbufferchannels = 0;
	output_buffer_.clear();
	for (auto &port : m_OutputPortMap)
	  delete port.second;
	m_OutputPortMap.clear();
	// JACK close has stopped callback execution before retiring the published ports.
	output_port_count_.store(0, std::memory_order_relaxed);
	
	// tells ssm to go back to non callback mode
	//if (RunCallback) RunCallback(RunContext, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <jack/jack.h>
// m_inbuf, out, channels, j, nframes
void deinterleave(void * _in, jack_default_audio_sample_t *out
		, int num_channels, int channel, int num_samples)
{
  int j;
  float * in;
  in = ((float*)_in) + channel;
  for(j = 0; j < num_samples; j++)
  {
    out[j] = (*in);
    in += num_channels;
  }
}

} // end extern "C"

/*
int JackClient::Process (jack_nframes_t nframes, void *arg)
{
	jack_port_t* input_port = m_InputPortMap[0]->Port;
	jack_port_t* output_port = m_OutputPortMap[0]->Port;
      jack_default_audio_sample_t *out = (jack_default_audio_sample_t *) jack_port_get_buffer (output_port, nframes);
      jack_default_audio_sample_t *in = (jack_default_audio_sample_t *) jack_port_get_buffer (input_port, nframes);

      memcpy (out, in, sizeof (jack_default_audio_sample_t) * nframes);
      
      return 0;      
}
*/


int JackClient::Process(jack_nframes_t nframes, void *self)
{	
  int j = 0;
  auto &client = *static_cast<JackClient *>(self);
  
	for (std::map<int,JackPort*>::iterator i=m_InputPortMap.begin();
		i!=m_InputPortMap.end(); i++)
	{
		if (jack_port_connected(i->second->Port))
		{
		  sample_t *in = (sample_t *) jack_port_get_buffer(i->second->Port, nframes);
// 		  memcpy (i->second->Buf, in, sizeof (sample_t) * m_BufferSize); //m_BufferSize -> 2nd AudioCollector parameter
			//Buff attribué par SetInputBuf dans le construcAteur de AudioCollector
		  if (client.m_Encoded)	//Added this to write in the buffer only if
		  {			//the encoder is in action
		    if (!j)	//only streams the 1st Jack Input port
		    {
		      if (ringbuffer_write_space (((JackClient*) self)->first) >= (sizeof (sample_t) * nframes))
		      {	
			size_t rf = ringbuffer_write (((JackClient*) self)->first, (char *)in, (sizeof (sample_t) * nframes));
		      }
/*		    else
		    {
		      std::cerr << "-----------Pas suffisament de place dans audio_fred !!!" << std::endl; 
		    }*/
		      j++;
		    }
		  }
		}
	}

	{
	  // A control thread may change the route. The callback never waits for it.
	  std::unique_lock<std::mutex> lock(client.output_mutex_, std::try_to_lock);
	  const int channels = lock ? client.m_ringbufferchannels.load(std::memory_order_relaxed) : 0;
	  const std::size_t frame_bytes = channels * sizeof(float);
	  const std::size_t period_bytes = nframes * frame_bytes;
	  bool output_available = false;
	  if (frame_bytes && client.m_ringbuffer && period_bytes <= client.output_buffer_.size()) {
	    auto *samples = client.output_buffer_.data();
	    const auto bytes = std::min(period_bytes, ringbuffer_read_space(client.m_ringbuffer));
	    const auto received = ringbuffer_read(client.m_ringbuffer, samples, bytes - bytes % frame_bytes);
	    if (received) {
	      // The encoder receives complete source samples, including the tail
	      // of a clip. Playback alone gets zero filling to the JACK period.
	      if (client.m_Encoded && ringbuffer_write_space(client.audio_mix_ring) >= received)
	        ringbuffer_write(client.audio_mix_ring, samples, received);
	      memset(samples + received, 0, period_bytes - received);
	      output_available = true;
	    }
	  }

	  // Port slots are append-only while JACK can call Process. An acquired
	  // count makes their handles visible even when the route lock is held.
	  const int ports = client.output_port_count_.load(std::memory_order_acquire);
	  for (int channel = 0; channel < ports; ++channel) {
	    auto *output = static_cast<sample_t *>(jack_port_get_buffer(client.output_ports_[channel], nframes));
	    if (!output)
	      continue;
	    if (output_available && channel < channels)
	      deinterleave(client.output_buffer_.data(), output, channels, channel, nframes);
	    else
	      memset(output, 0, nframes * sizeof(*output));
	  }
	}
	m_BufferSize=nframes;
			
// 	if(RunCallback&&RunContext)
// 	{
// 		// do the work
// 		RunCallback(RunContext, nframes);
// 	}
	
	return 0;
}

int JackClient::SetRingbufferPtr(ringbuffer_t *rb, int, int channels) {
	if (!rb || channels <= 0 || channels > MAX_OUTPUTPORTS)
	  return EINVAL;
	std::lock_guard<std::mutex> lock(output_mutex_);
	if (!m_Attached)
	  return EINVAL;
	while (output_port_count_.load(std::memory_order_relaxed) < channels) {
	  if (add_output_port_locked() < 0)
	    return ENODEV;
	}
	const std::size_t period = std::max(4096u, jack_get_buffer_size(m_Client));
	output_buffer_.resize(period * channels * sizeof(float));
	ringbuffer_reset(rb);
	m_ringbufferchannels = channels;
	m_ringbuffer = rb;
	return 0;
}

void JackClient::ResetRingbuffer(ringbuffer_t *rb, bool disconnect) {
	if (auto *client = m_Singleton) {
	  std::lock_guard<std::mutex> lock(client->output_mutex_);
	  if (client->m_ringbuffer == rb) {
	    ringbuffer_reset(rb);
	    if (disconnect) {
	      client->m_ringbuffer = nullptr;
	      // Preserve the recording format until a different supplier binds.
	    }
	  }
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::AddInputPort()
{
	char Name[256];
	sprintf(Name,"In%d",m_NextInputID);
	
	JackPort *NewPort = new JackPort;
	NewPort->Name=Name;
	NewPort->Buf=NULL;		
	NewPort->Port = jack_port_register (m_Client, Name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
	m_InputPortMap[m_NextInputID]=NewPort;
	m_NextInputID++;
	return m_NextInputID-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::AddOutputPort()
{
	std::lock_guard<std::mutex> lock(output_mutex_);
	return add_output_port_locked();
}

int JackClient::add_output_port_locked()
{
	static_assert(ATOMIC_INT_LOCK_FREE == 2, "JACK output publication must be lock-free");
	const int channel = output_port_count_.load(std::memory_order_relaxed);
	if (!m_Attached || channel == MAX_OUTPUTPORTS)
	  return -1;

	char name[32];
	snprintf(name, sizeof(name), "Out%d", channel);
	auto *port = new JackPort;
	port->Name = name;
	port->Port = jack_port_register(m_Client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if (!port->Port) {
	  delete port;
	  return -1;
	}
	m_OutputPortMap[channel] = port;
	output_ports_[channel] = port->Port;
	output_port_count_.store(channel + 1, std::memory_order_release);
	return channel;
}


/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::OnSRateChange(jack_nframes_t n, void *o)
{
	m_SampleRate=n;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::OnJackShutdown(void *o)
{
  act("Audio Jack Shutdown");
  m_Attached=false;
  // tells ssm to go back to non callback mode
  RunCallback(RunContext, false);
  return;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::GetPortNames(std::vector<std::string> &InputNames, std::vector<std::string> &OutputNames)
{
	InputNames.clear();
	OutputNames.clear();

	if (!m_Attached) return;

	//Outputs first
	const char **PortNameList=jack_get_ports(m_Client,NULL,NULL,JackPortIsOutput);	
	
	int n=0;
	while(PortNameList[n]!=NULL)
	{		
		OutputNames.push_back(PortNameList[n]);
		n++;
	}	
	
	delete PortNameList;
	
	//Inputs second
	PortNameList=jack_get_ports(m_Client,NULL,NULL,JackPortIsInput);
	
	n=0;
	while(PortNameList[n]!=NULL)
	{		
		InputNames.push_back(PortNameList[n]);
		n++;
	}
	
	delete PortNameList;		
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Input means input of SSM, so this connects jack sources to the plugin outputs
void JackClient::ConnectInput(int n, const std::string &JackPort)
{
  if (!IsAttached()) return;
  
  //cerr<<"JackClient::ConnectInput: connecting source ["<<JackPort<<"] to dest ["<<m_InputPortMap[n]->Name<<"]"<<endl;
  
  if (m_InputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, m_InputPortMap[n]->ConnectedTo.c_str(), jack_port_name(m_InputPortMap[n]->Port)))
	error("Audio Jack ConnectInput: cannot disconnect input port [%s] from [%s]",
	      m_InputPortMap[n]->ConnectedTo.c_str(), m_InputPortMap[n]->Name.c_str());
    }
  
  m_InputPortMap[n]->ConnectedTo = JackPort;
  
  if (jack_connect (m_Client, JackPort.c_str(),
		    jack_port_name(m_InputPortMap[n]->Port)))

    error("JackClient::ConnectInput: cannot connect input port [%s] to [%s]",
	  JackPort.c_str(), m_InputPortMap[n]->Name.c_str());
  
	m_InputPortMap[n]->Connected=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Output means output of SSM, so this connects plugin inputs to a jack destination
void JackClient::ConnectOutput(int n, const std::string &JackPort)
{
  if (!IsAttached()) return;
  std::cerr<<"JackClient::ConnectOutput: connecting source ["<<m_OutputPortMap[n]->Name<<"] \
  	to dest ["<<JackPort<<"]"<<std::endl;
  
  if (m_OutputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), m_OutputPortMap[n]->ConnectedTo.c_str()))
	error("JackClient::ConnectOutput: cannot disconnect output port [%s] to [%s]",
	      m_OutputPortMap[n]->ConnectedTo.c_str(),
	      m_OutputPortMap[n]->Name.c_str());
    }
  
  m_OutputPortMap[n]->ConnectedTo = JackPort;
  if (jack_connect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), JackPort.c_str()))
    error("JackClient::ConnectOutput: cannot connect output port [%s] to [%s]",
	  m_OutputPortMap[n]->Name.c_str(), JackPort.c_str());
  m_OutputPortMap[n]->Connected=true;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Input means input of SSM, so this connects jack sources to the plugin outputs
void JackClient::DisconnectInput(int n)
{
  if (!IsAttached()) return;
	//cerr<<"JackClient::DisconnectInput: Disconnecting input "<<n<<endl;

  if (m_InputPortMap[n]->ConnectedTo!="")
    {
      if (jack_disconnect (m_Client, m_InputPortMap[n]->ConnectedTo.c_str(), jack_port_name(m_InputPortMap[n]->Port)))
	error("JackClient::ConnectInput: cannot disconnect input port [%s] from [%s]",
	      m_InputPortMap[n]->ConnectedTo.c_str(),
	      m_InputPortMap[n]->Name.c_str());
    }
  
  m_InputPortMap[n]->Connected=false;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Output means output of SSM, so this connects plugin inputs to a jack destination
void JackClient::DisconnectOutput(int n)
{
	if (!IsAttached()) return;
	//cerr<<"JackClient::DisconnectInput: Disconnecting input "<<n<<endl;

	if (m_OutputPortMap[n]->ConnectedTo!="")
	{
	  if (jack_disconnect (m_Client, jack_port_name(m_OutputPortMap[n]->Port), m_OutputPortMap[n]->ConnectedTo.c_str()))
	    error("JackClient::ConnectOutput: cannot disconnect output port [%s] from [%s]",
		  m_OutputPortMap[n]->ConnectedTo.c_str(),
		  m_OutputPortMap[n]->Name.c_str());
	}

	m_OutputPortMap[n]->Connected=false;
}
/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::SetInputBuf(int ID, float* s)
{
	if(m_InputPortMap.find(ID)!=m_InputPortMap.end()) m_InputPortMap[ID]->Buf=s;
	else std::cerr<<"Could not find port ID "<<ID<<std::endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////
	
void JackClient::SetOutputBuf(int ID, float* s)
{
	if(m_OutputPortMap.find(ID)!=m_OutputPortMap.end()) m_OutputPortMap[ID]->Buf=s;
	else error("Could not find port ID %u", ID);
}


#endif
