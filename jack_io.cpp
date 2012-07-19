// TODO: upp++ says jack_client_open and jack_client_close cause a heap leak

#include "jack_io.h"

#include "debug.h"


/* private varables */

// JACK handles
jack_client_t* JackIO::Client = 0 ;
jack_port_t* JackIO::InputPort1 = 0 ;
jack_port_t* JackIO::InputPort2 = 0 ;
jack_port_t* JackIO::OutputPort1 = 0 ;
jack_port_t* JackIO::OutputPort2 = 0 ;

// audio data
Scene* JackIO::CurrentScene = 0 ;

// server state
unsigned int JackIO::NFramesPerPeriod = 0 ;
const unsigned int JackIO::FRAME_SIZE = sizeof(jack_default_audio_sample_t) ;
unsigned int JackIO::PeriodSize = 0 ;


/* public functions */

// setup

int JackIO::Init()
{
if (! INIT_JACK) return true ;

	const char* client_name = JACK_CLIENT_NAME ;
	jack_options_t options = JackNullOption ;
	jack_status_t status ;
	const char* server_name = NULL ;

	// register client
	if (!(Client = jack_client_open(client_name , options , &status , server_name)))
		{ DBG("Unable to connect to JACK server") ; return false ; }

	// assign callbacks
	jack_set_process_callback(Client , ProcessCallback , 0) ;
	jack_set_buffer_size_callback(Client , SetBufferSizeCallback , 0) ;
	jack_on_shutdown(Client , ShutdownCallback , 0) ;

	// register ports and activate client
	InputPort1 = jack_port_register(Client , "input1" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsInput , 0) ;
	InputPort2 = jack_port_register(Client , "input2" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsInput , 0) ;
	OutputPort1 = jack_port_register(Client , "output1" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsOutput , 0) ;
	OutputPort2 = jack_port_register(Client , "output2" , JACK_DEFAULT_AUDIO_TYPE , JackPortIsOutput , 0) ;
	if (InputPort1 == NULL || InputPort2 == NULL) { DBG("no logical input ports") ; return false ; }
	if (OutputPort1 == NULL || OutputPort2 == NULL) { DBG("no logical output ports") ; return false ; }
	if (jack_activate(Client)) { DBG("cannot activate client") ; return false ; }

#if JACKIO_HAS_BUFFERS
	LoopBuffers1.push_back(RecordBuffer1) ; LoopBuffers2.push_back(RecordBuffer2) ;
#endif

	DBG("JACK initialized") ; return true ;
}

void JackIO::Reset()
{
	// close client and free resouces
// TODO: not sure but JACK may free these for us
	if (Client) jack_client_close(Client) ;
	if (InputPort1) { free(InputPort1) ; InputPort1 = 0 ; }
	if (InputPort2) { free(InputPort2) ; InputPort2 = 0 ; }
	if (OutputPort1) { free(OutputPort1) ; OutputPort1 = 0 ; }
	if (OutputPort2) { free(OutputPort2) ; OutputPort2 = 0 ; }
	if (Client) { free(Client) ; Client = 0 ; }

	// reset state
	PeriodSize = NFramesPerPeriod = 0 ;
}


// getters/setters

unsigned int JackIO::GetNFrames() { return NFramesPerPeriod ; }

const unsigned int JackIO::GetFrameSize() { return FRAME_SIZE ; }

void JackIO::SetCurrentScene(Scene* currentScene) { CurrentScene = currentScene ; }


/* private functions */

// helpers

void JackIO::CalculatePeriodSize(unsigned int nFrames) { PeriodSize = FRAME_SIZE * (NFramesPerPeriod = nFrames) ; }

// JACK callbacks

int JackIO::ProcessCallback(jack_nframes_t nFrames , void* arg)
{
#if LOOP_COUNTER
	// increment sample rollover - ASSERT: (nFrames == NFramesPerPeriod)
	if (!(CurrentScene->frameN = (CurrentScene->frameN + NFramesPerPeriod) % CurrentScene->nFrames) &&
			CurrentScene->isRecording && CurrentScene->isAutoRecord)
	{
		++(CurrentScene->loopN) ;
#if DSP
		CurrentScene->loopBuffers1.push_back(CurrentScene->recordBuffer1 = new jack_default_audio_sample_t[CurrentScene->nFrames]()) ;
		CurrentScene->loopBuffers2.push_back(CurrentScene->recordBuffer2 = new jack_default_audio_sample_t[CurrentScene->nFrames]()) ;
#endif
	}

if (DEBUG) { if (!CurrentScene->frameN) DBG("NEW LOOP %d" , CurrentScene->loopN) ; if (!(CurrentScene->frameN % 65536)) Loopidity::Vardump() ; }
#endif 
#if ! DSP
	return 0 ;
#endif

	jack_default_audio_sample_t* in1 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort1 , nFrames) ;
	jack_default_audio_sample_t* out1 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort1 , nFrames) ;
	jack_default_audio_sample_t* in2 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort2 , nFrames) ;
	jack_default_audio_sample_t* out2 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort2 , nFrames) ;
	jack_default_audio_sample_t* currBuff1 = &CurrentScene->recordBuffer1[CurrentScene->frameN] ;
	jack_default_audio_sample_t* currBuff2 = &CurrentScene->recordBuffer2[CurrentScene->frameN] ;


#if PASSTHRU
	// PeriodSize = sizeof(jack_default_audio_sample_t) * nFrames ;
	memcpy(out1 , in1 , PeriodSize) ; memcpy(out2 , in2 , PeriodSize) ;
	return 0 ;
#endif

	for (unsigned int frameNin = 0 ; frameNin < nFrames ; ++frameNin)
	{
bool isMonitorInputs = true ; // TODO:
		if (!isMonitorInputs) currBuff1[frameNin] = currBuff2[frameNin] = 0 ;
		else currBuff1[frameNin] = in1[frameNin] ; currBuff2[frameNin] = in2[frameNin] ;

		unsigned int frameNout = CurrentScene->frameN + frameNin ;
		for (unsigned int loopN = 0 ; loopN < CurrentScene->loopN ; ++loopN)
		{
			currBuff1[frameNin] += CurrentScene->loopBuffers1[loopN][frameNout] ;
			currBuff2[frameNin] += CurrentScene->loopBuffers2[loopN][frameNout] ;
		}
	}
	memcpy(out1 , currBuff1 , PeriodSize) ; memcpy(out2 , currBuff2 , PeriodSize) ;
	memcpy(currBuff1 , in1 , PeriodSize) ; memcpy(currBuff2 , in2 , PeriodSize) ;

	return 0 ;
}

int JackIO::SetBufferSizeCallback(jack_nframes_t nFrames , void* arg) { CalculatePeriodSize(nFrames) ; }

void JackIO::ShutdownCallback(void* arg) { Reset() ; exit(1) ; }
