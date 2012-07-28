// TODO: u++ (in DEBUG cfg) says jack_client_open and jack_client_close cause a heap leak

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
Scene* JackIO::NextScene = 0 ;
jack_default_audio_sample_t* JackIO::RecordBuffer1 = 0 ;
jack_default_audio_sample_t* JackIO::RecordBuffer2 = 0 ;

// server state
unsigned int JackIO::NFramesPerPeriod = 0 ;
const unsigned int JackIO::FRAME_SIZE = sizeof(jack_default_audio_sample_t) ;
unsigned int JackIO::PeriodSize = 0 ;

// misc flags
bool JackIO::IsMonitorInputs = true ;


/* public functions */

// setup

bool JackIO::Init(unsigned int nFrames , Scene* currentScene)
{
if (! INIT_JACK) return true ;

	// set initial state
	Reset(currentScene) ;
	const Vector<String>& args = CommandLine() ;
	for (unsigned int argN = 0 ; argN < args.GetCount() ; ++argN)
		if (!strcmp(args[argN] , MONITOR_ARG)) IsMonitorInputs = false ;

	// initialize JACK client
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

	// initialize record buffers
	RecordBuffer1 = new jack_default_audio_sample_t[nFrames]() ;
	RecordBuffer2 = new jack_default_audio_sample_t[nFrames]() ;

	DBG("JACK initialized") ; return true ;
}

void JackIO::Reset(Scene* currentScene)
	{ CurrentScene = NextScene = currentScene ; PeriodSize = NFramesPerPeriod = 0 ; }


// getters/setters

jack_port_t* JackIO::GetInPort1() { return InputPort1 ; }

jack_port_t* JackIO::GetInPort2() { return InputPort2 ; }

jack_port_t* JackIO::GetOutPort1() { return OutputPort1 ; }

jack_port_t* JackIO::GetOutPort2() { return OutputPort2 ; }

unsigned int JackIO::GetNFrames() { return NFramesPerPeriod ; }

const unsigned int JackIO::GetFrameSize() { return FRAME_SIZE ; }

Scene* JackIO::GetCurrentScene() { return CurrentScene ; }

void JackIO::SetNextScene(Scene* nextScene) { NextScene = nextScene ; }


/* private functions */


// JACK callbacks

int JackIO::ProcessCallback(jack_nframes_t nFrames , void* arg)
{
#if DSP
	// get JACK buffers
	jack_default_audio_sample_t* in1 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort1 , nFrames) ;
	jack_default_audio_sample_t* out1 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort1 , nFrames) ;
	jack_default_audio_sample_t* in2 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort2 , nFrames) ;
	jack_default_audio_sample_t* out2 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort2 , nFrames) ;
#if ! PASSTHRU
	if (!Loopidity::GetIsRecording()) // TODO: we could avoid this call if all CurrentScene->nFrames were initially 1
#endif
	{
		if (IsMonitorInputs) { memcpy(out1 , in1 , PeriodSize) ; memcpy(out2 , in2 , PeriodSize) ; }
		return 0 ;
	}

	// index into the record buffers and mix out
	jack_default_audio_sample_t* currBuff1 = &(RecordBuffer1[CurrentScene->frameN]) ;
	jack_default_audio_sample_t* currBuff2 = &(RecordBuffer2[CurrentScene->frameN]) ;
	for (unsigned int frameNin = 0 ; frameNin < nFrames ; ++frameNin)
	{
		// write input to mix buffers
		if (!IsMonitorInputs) currBuff1[frameNin] = currBuff2[frameNin] = 0 ;
		else { currBuff1[frameNin] = in1[frameNin] ; currBuff2[frameNin] = in2[frameNin] ; }

		// mix unmuted tracks into mix buffers
		unsigned int frameNout = CurrentScene->frameN + frameNin ;
		for (unsigned int loopN = 0 ; loopN < CurrentScene->nLoops ; ++loopN)
		{
			currBuff1[frameNin] += CurrentScene->loopBuffers1[loopN][frameNout] ;
			currBuff2[frameNin] += CurrentScene->loopBuffers2[loopN][frameNout] ;
		}
	}
	// write mix buffers to outputs and write input to record buffers
	memcpy(out1 , currBuff1 , PeriodSize) ; memcpy(out2 , currBuff2 , PeriodSize) ;
	memcpy(currBuff1 , in1 , PeriodSize) ; memcpy(currBuff2 , in2 , PeriodSize) ;
#endif

#if LOOP_COUNTER
	// increment sample rollover - ASSERT: ((nFrames == NFramesPerPeriod) && !(frameN % CurrentScene->nFrames))
	if (!(CurrentScene->frameN = (CurrentScene->frameN + NFramesPerPeriod) % CurrentScene->nFrames))
	{
		// copy record buffers and increment nLoops
		if (CurrentScene->isSaveLoop)
		{
#if DSP
			jack_default_audio_sample_t* newBuffer1 = new jack_default_audio_sample_t[CurrentScene->nFrames]() ;
			jack_default_audio_sample_t* newBuffer2 = new jack_default_audio_sample_t[CurrentScene->nFrames]() ;
			unsigned int nBytes = FRAME_SIZE * CurrentScene->nFrames ;
			memcpy(newBuffer1 , RecordBuffer1 , nBytes) ;
			memcpy(newBuffer2 , RecordBuffer2 , nBytes) ;
			CurrentScene->loopBuffers1.push_back(newBuffer1) ;
			CurrentScene->loopBuffers2.push_back(newBuffer2) ;
#endif

			++(CurrentScene->nLoops) ;

if (DEBUG) { char dbg[255] ; sprintf(dbg , "NEW LOOP scene:%d loopN:%d" , Loopidity::GetCurrentSceneN() , CurrentScene->nLoops) ; LoopidityUpp::GetApp()->tempStatusL(dbg) ; }
		}

		// switch to NextScene if necessary
		CurrentScene = NextScene ; LoopidityUpp::GetApp()->setMode() ;
	}
#endif 

	return 0 ;
}

int JackIO::SetBufferSizeCallback(jack_nframes_t nFrames , void* arg)
	{ PeriodSize = FRAME_SIZE * (NFramesPerPeriod = nFrames) ; }

void JackIO::ShutdownCallback(void* arg)
{
	// close client and free resouces
	if (Client) { jack_client_close(Client) ; free(Client) ; Client = 0 ; }
	if (InputPort1) { free(InputPort1) ; InputPort1 = 0 ; }
	if (InputPort2) { free(InputPort2) ; InputPort2 = 0 ; }
	if (OutputPort1) { free(OutputPort1) ; OutputPort1 = 0 ; }
	if (OutputPort2) { free(OutputPort2) ; OutputPort2 = 0 ; }
	if (RecordBuffer1) { delete RecordBuffer1 ; RecordBuffer1 = 0 ; }
	if (RecordBuffer2) { delete RecordBuffer2 ; RecordBuffer2 = 0 ; }
	exit(1) ;
}
