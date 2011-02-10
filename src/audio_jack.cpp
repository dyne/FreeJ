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
m_NextInputID(0),
m_NextOutputID(0),
m_inbuf(NULL),
m_ringbuffer(NULL),
audio_mix_ring(NULL)
{
  m_MixBufferSize = 4096 * 512 * sizeof(float);
  m_MixBuffer = (float*) malloc(m_MixBufferSize);
  m_MixBufferOperation = (float*) malloc(m_MixBufferSize*2);
}

/////////////////////////////////////////////////////////////////////////////////////////////

JackClient::~JackClient()	
{
  if (m_MixBuffer)free(m_MixBuffer);
  if (m_MixBufferOperation)free(m_MixBufferOperation);
	Detach();
}

/////////////////////////////////////////////////////////////////////////////////////////////


bool JackClient::Attach(const std::string &ClientName)
{
	if (m_Attached) return true;

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
	
    // tell the JACK server that we are ready to roll
	if (jack_activate (m_Client))
	{
	  error("cannot activate client");
		return false;
	}

	m_Attached=true;
	
	std::cerr << "----- sizeof jack_default_audio_sample_t : " << sizeof(jack_default_audio_sample_t) << std::endl;
	
	audio_mix_ring = ringbuffer_create(4096 * 512 * 4);		//1024 not enought, must be the same size_t
								// as buf_fred set up in OggTheoraEncoder::init
	first = ringbuffer_create(4096 * 512 * 4);
	second = ringbuffer_create(4096 * 512 * 4);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void JackClient::Detach()
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
	if(second) ringbuffer_free(second);
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

bool JackClient::Mux(int nfr)
{
  int size[3] = {0, 0, 0};
  int sizeMax = 0, c = 0;
  float *ringPtr, *mixPtr;
 
  memset (m_MixBuffer, 0, m_MixBufferSize);
  memset (m_MixBufferOperation, 0, m_MixBufferSize*2);
  
  for (std::map<int,JackPort*>::iterator i=m_InputPortMap.begin();
	  i!=m_InputPortMap.end(); i++)
  {
    if (c == 0 && i->second->connected)
    {
      if (size[c] = ringbuffer_read_space(i->second->in_ring))
      {
	if (size[c] == ((sizeof (float) * nfr)))
	{
	  size_t rv = ringbuffer_read(i->second->in_ring, (char *)m_MixBuffer, size[c]);
	  if (sizeMax < rv)
	    sizeMax = rv;
/*	  ringPtr = m_MixBufferOperation;
	  mixPtr = m_MixBuffer;
	  for (int j=0; j < rv/sizeof(float); j++, mixPtr++, ringPtr++)	//interleave
	  {
	    *mixPtr++ = *ringPtr;
	  }*/
	}
	else
	{
	  std::cerr << "----- Problems reading the in_ring size[" \
	      << c << "] :" << size[c] << std::endl;
	  return (false);
	}
      }  
      else
      {
	  std::cerr << "----- space == 0 in the " << c << " in_ring" << std::endl;
	  return (false);
      }
      c++;
    }
    else if(c == 0 && !i->second->connected) c++;
/*    else if (c == 1 && i->second->connected)	//add the first VideoLayer channel to the left
    {
      if (size[c] = ringbuffer_read_space(i->second->in_ring))	//
      {
	size_t rv = ringbuffer_read(i->second->in_ring, (char *)m_MixBufferOperation, size[c]);
	if (sizeMax < rv)
	  sizeMax = rv;
	if (rv != size[c])
	{
	  std::cerr << "----- Problems reading the " << c << " in_ring" << std::endl;
	  return (false);
	}
	ringPtr = m_MixBufferOperation;
	mixPtr = m_MixBuffer;
	for (int j=0; j < rv/sizeof(float); j++)
	{
	  *mixPtr++ += *ringPtr++; 
	}
      }
      c++;
    }*/
//     else if(c == 1 && !i->second->connected) c++;
/*    else if (c == 1 && i->second->connected)	//interleaving the second VideoLayer channel
    {
      if (size[c] = ringbuffer_read_space(i->second->in_ring))	//
      {
	size_t rv = ringbuffer_read(i->second->in_ring, (char *)m_MixBufferOperation, size[c]);
	if (rv)
	  sizeMax += rv;
	if (rv != size[c])
	{
	  std::cerr << "----- Problems reading the " << c << " in_ring" << std::endl;
	  return (false);
	}
	ringPtr = m_MixBufferOperation;
	mixPtr = m_MixBuffer;
	mixPtr++;
	for (int j=0; j < rv/sizeof(float); j++, mixPtr++, ringPtr++)
	{
	  *mixPtr++ = *ringPtr;
	}
      }
      c++;
    }*/
/*    else if(c == 2 && !i->second->connected) c++;
    else if (c > 2)
    {
      std::cerr << "----- 3 jack audio input ports maximum !!" << std::endl;
    }*/
  }
  if (sizeMax)
  {
    if (ringbuffer_write_space (audio_mix_ring) >= sizeMax)
    {
      size_t rv = ringbuffer_write (audio_mix_ring, (char *)m_MixBuffer, sizeMax*2);
      if (rv != sizeMax)
      {
	std::cerr << "---" << rv << " : au lieu de :" << sizeMax*2 \
	    << " octets ecrits dans le ringbuffer !!" << std::endl;
	return (false);
      }
//       std::cerr << "---- wrote :" << sizeMax << std::endl << std::flush;
    }
    else
    {
      std::cerr << "------ not enough memory in audio_mix_ring buffer !!!" << std::endl;
      return (false);
    }
  }
  return (true);
}

int JackClient::Process(jack_nframes_t nframes, void *self)
{	
  bool data_in = false;
  int	j = 0;
  
	for (std::map<int,JackPort*>::iterator i=m_InputPortMap.begin();
		i!=m_InputPortMap.end(); i++, j++)
	{
		if (jack_port_connected(i->second->Port))
		{
		  i->second->connected = true;
		  sample_t *in = (sample_t *) jack_port_get_buffer(i->second->Port, nframes);
// 		  memcpy (i->second->Buf, in, sizeof (sample_t) * m_BufferSize); //m_BufferSize -> 2nd AudioCollector parameter
			//Buff attribué par SetInputBuf dans le constructeur de AudioCollector
/*		  if (ringbuffer_write_space (i->second->in_ring) >= (sizeof (sample_t) * nframes))
		  {
		    size_t rf = ringbuffer_write (i->second->in_ring, (char *)in, (sizeof (sample_t) * nframes));
		    data_in = true;
		  }*/
		  if (j == 0)
		  {
		    if (ringbuffer_write_space (((JackClient*) self)->first) >= (sizeof (sample_t) * nframes))
		    {
		      size_t rf = ringbuffer_write (((JackClient*) self)->first, (char *)in, (sizeof (sample_t) * nframes));
		      data_in = true;
		    }
		    else
		    {
		      std::cerr << "-----------Pas suffisament de place dans audio_fred !!!" << std::endl; 
		    }
		  }
		  else if (j == 1)
		  {
		    if (ringbuffer_write_space (((JackClient*) self)->second) >= (sizeof (sample_t) * nframes))
		    {
		      size_t rf = ringbuffer_write (((JackClient*) self)->second, (char *)in, (sizeof (sample_t) * nframes));
		      data_in = true;
		    }
		    else
		    {
		      std::cerr << "-----------Pas suffisament de place dans audio_fred !!!" << std::endl; 
		    }
		  }
	      }
	      else
		i->second->connected = false;
	}

	int channels = ((JackClient*) self)->m_ringbufferchannels;

/*	if (data_in)
	  if (!((JackClient*) self)->Mux(nframes))
	    std::cerr << "----- Muxing problem !!" << std::endl << std::flush;*/
	
	bool output_available = false;
//m_ringbuffer created by ViewPort::add_audio
//1024*512 rounded up to the next power of two.
	if (((JackClient*) self)->m_ringbuffer) 
	{
	  static int firsttime = 1 + ceil(4096/nframes); // XXX pre-buffer  TODO decrease this and compensate latency
	
	  if (ringbuffer_read_space(((JackClient*) self)->m_ringbuffer) >= 
			      firsttime * channels * nframes * sizeof(float)) 
	  {
		  firsttime=1;
		  size_t rv = ringbuffer_read(((JackClient*) self)->m_ringbuffer, 
					      ((JackClient*) self)->m_inbuf,
					      channels*nframes*sizeof(float));
/*		  if (ringbuffer_write_space (((JackClient*) self)->audio_fred) >= rv)
		  {*/
// 		    unsigned char *aPtr = (unsigned char *)((JackClient*) self)->m_inbuf;
// 		    printf("----:\n");
// 		    for (int i=0; i < 24; i++, aPtr++)
// 		    {
// 		      printf ("%02x ", *aPtr);
// 		    }
// 		    printf ("\n----;\n");
// 		    fflush(stdout);
/*		    size_t rf = ringbuffer_write (((JackClient*) self)->audio_fred, ((JackClient*) self)->m_inbuf, rv);
		    if (rf != rv)
			std::cerr << "---" << rf << " : au lieu de :" << rv << " octets ecrits dans le ringbuffer !!"\
									       << std::endl;
		  }
		  else
		  {
		     std::cerr << "-----------Pas suffisament de place dans audio_fred !!!" << std::endl; 
		  }*/
//reads m_ringbuffer and puts it in m_inbuf
//m_inbuf created in SetRingbufferPtr called by add_audio
//4096 * channels * sizeof(float)
  		if (rv >= channels * nframes * sizeof(float))
		{
  			output_available = true;
		}
	  }
#if 0
	  else if (firsttime==1)
	  	fprintf(stderr, "AUDIO BUFFER UNDERRUN: %i samples < %i\n", ringbuffer_read_space(((JackClient*) self)->m_ringbuffer) / sizeof(float) / channels, nframes);
#endif
	}

	j=0;
	for (std::map<int,JackPort*>::iterator i=m_OutputPortMap.begin();
		i!=m_OutputPortMap.end(); i++)
	{
		if (output_available && j < channels) 
		{
			sample_t *out = (sample_t *) jack_port_get_buffer(i->second->Port, nframes);
			memset (out, 0, sizeof (jack_default_audio_sample_t) * nframes);
			deinterleave(((JackClient*) self)->m_inbuf, out, channels
						, j, nframes);
//writes nframes of channels m_inbuf to out
//two times if stereo (shifted by the channel number)
#if 0  			// test-noise:
			int i; for (i=0; i< nframes; i++) out[i]=(float) i/(float)nframes;
#endif
		}
		else // no output availaible, clear
		{
			sample_t *out = (sample_t *) jack_port_get_buffer(i->second->Port, nframes);
			memset (out, 0, sizeof (sample_t) * nframes);
		}
		j++;
	}
	
	m_BufferSize=nframes;
			
// 	if(RunCallback&&RunContext)
// 	{
// 		// do the work
// 		RunCallback(RunContext, nframes);
// 	}
	
	return 0;
}

int JackClient::SetRingbufferPtr(ringbuffer_t *rb, int rate, int channels) {
	int i;
	m_ringbuffer = NULL;

	func ("jack-client ringbuffer set for %i channels", channels);
std::cout << "SetRingbufferPtr, channels :" << channels << std::endl;
	for (i=m_NextOutputID; i<channels; i++) {
		AddOutputPort();
// 		AddInputPort(); no input port
		if (i == 0)	//connects output ports to system input ports (fred_99)
		{
// 			this->ConnectOutput(i, "system:playback_1");
// 			this->ConnectOutput(i, "freej:In0");
		}
		//else if (i == 1)
			//this->ConnectOutput(i, "system:playback_2"); //see to test if there is two system channels
	}

	if(m_inbuf) free(m_inbuf);
	m_inbuf = (char*) malloc(4096 * channels * sizeof(float));
	m_ringbufferchannels = channels;
	m_ringbuffer = rb;
	return (0);
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
	NewPort->in_ring = ringbuffer_create(4096 * 512 * 4);		//1024 not enought, must be the same size_t
	NewPort->connected = false;
	m_NextInputID++;
	return m_NextInputID-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int JackClient::AddOutputPort()
{
	char Name[256];
	sprintf(Name,"Out%d",m_NextOutputID);
	
	JackPort *NewPort = new JackPort;
	NewPort->Name=Name;
	NewPort->Buf=NULL;		
	NewPort->Port = jack_port_register (m_Client, Name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	m_OutputPortMap[m_NextOutputID]=NewPort;
	
	m_NextOutputID++;
	return m_NextOutputID-1;
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
