// TODO: upp++ says jack_client_open and jack_client_close cause a heap leak

#include "jackio.h"
#include "debug.h"


/* private varables */

// JACK handles
jack_client_t* JackIO::Client = 0 ;
jack_port_t* JackIO::InputPort1 = 0 ;
jack_port_t* JackIO::InputPort2 = 0 ;
jack_port_t* JackIO::OutputPort1 = 0 ;
jack_port_t* JackIO::OutputPort2 = 0 ;

// audio data
jack_default_audio_sample_t* JackIO::RecordBuffer1 = new jack_default_audio_sample_t[MAX_BUFFER]() ;
jack_default_audio_sample_t* JackIO::RecordBuffer2 = new jack_default_audio_sample_t[MAX_BUFFER]() ;
Vector<jack_default_audio_sample_t*> JackIO::LoopBuffers1 ;
Vector<jack_default_audio_sample_t*> JackIO::LoopBuffers2 ;

// buffer iteration
unsigned int JackIO::FrameSize = sizeof(jack_default_audio_sample_t) ;
unsigned int JackIO::PeriodSize = 0 ;
unsigned int JackIO::NFramesPerPeriod = 0 ;
unsigned int JackIO::NFrames = MAX_BUFFER / FrameSize ;
unsigned int JackIO::FrameN = 0 ;
unsigned int JackIO::LoopN = 0 ;

// state flags
unsigned int JackIO::CurrentScene = 0 ;
bool JackIO::IsAutoRecord = true ;
bool JackIO::IsRecording = false ;
bool JackIO::IsPulseExist = false ;


/* public functions */

// user actions

int JackIO::Init(bool isConnectPorts)
{
	DBG((Client)? "JACK resetting" : "JACK initializing") ;

	const char* client_name = JACK_CLIENT_NAME ;
	jack_options_t options = JackNullOption ;
	jack_status_t status ;
	const char* server_name = NULL ;

	// register client
	Reset() ;
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

	// connect ports
	if (isConnectPorts)
	{
		const char** portsIn = jack_get_ports(Client , NULL , NULL , JackPortIsPhysical | JackPortIsOutput) ;
		const char** portsOut = jack_get_ports(Client , NULL , NULL , JackPortIsPhysical | JackPortIsInput) ;
		if (portsIn == NULL) { DBG("no physical capture ports") ; return false ; }
		if (portsOut == NULL) { DBG("no physical playback ports") ; return false ; }
		bool isInput = (!jack_connect(Client , portsIn[0] , jack_port_name(InputPort1)) &&
										!jack_connect(Client , portsIn[1] , jack_port_name(InputPort2))) ;
		free(portsIn) ; if (!isInput) { DBG("cannot connect input ports") ; return false ; }
		bool isOutput = (!jack_connect(Client , jack_port_name(OutputPort1) , portsOut[0]) &&
										!jack_connect(Client , jack_port_name(OutputPort2) , portsOut[1])) ;
		free(portsOut) ; if (!isOutput) { DBG("cannot connect output ports") ; return false ; }
	}

	LoopBuffers1.push_back(RecordBuffer1) ; LoopBuffers2.push_back(RecordBuffer2) ;

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

// TODO: purge all but initial LoopBuffers

	// reset state
	NFrames = MAX_BUFFER / FrameSize ; PeriodSize = NFramesPerPeriod = CurrentScene = 0 ;
	IsAutoRecord = true ; IsRecording = IsPulseExist = false ;

	// reset GUI
	Loopidity::App->setStatusRecord(CurrentScene , IsAutoRecord) ;
}

void JackIO::SetLoop()
{
	if (!IsRecording) { FrameN = 0 ; IsRecording = true ; }
	else if (!IsPulseExist) NFrames = FrameN + ((NFramesPerPeriod - (FrameN % NFramesPerPeriod)) % NFramesPerPeriod) ;
	else Loopidity::App->setStatusRecord(CurrentScene , IsAutoRecord = !IsAutoRecord) ;

Vardump() ;
}


/* private functions */

// helpers

void JackIO::CalculatePeriodSize(unsigned int nFrames) { PeriodSize = FrameSize * (NFramesPerPeriod = nFrames) ;
if (NFrames % nFrames) { PromptOK("JackIO::Init ASSERTION FAILED: (NFrames MOD nFrames > 0)") ; exit(1) ; }
else DBG("JackIO::Init ASSERTION PASSED: (NFrames MOD nFrames == 0)") ; }

// JACK callbacks

int JackIO::ProcessCallback(jack_nframes_t nFrames , void* arg)
{
	// ASSERT: (nFrames == NFramesPerPeriod)
	if (!(FrameN = (FrameN + NFramesPerPeriod) % NFrames) && IsAutoRecord)
	{
		++LoopN ;
		LoopBuffers1.push_back(RecordBuffer1 = new jack_default_audio_sample_t[NFrames]()) ;
		LoopBuffers2.push_back(RecordBuffer2 = new jack_default_audio_sample_t[NFrames]()) ;
	}

if (DEBUG) { if (!FrameN) DBG("NEW LOOP %d" , FrameN) ; if (!(FrameN % 65536)) Vardump() ; }

	jack_default_audio_sample_t* in1 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort1 , nFrames) ;
	jack_default_audio_sample_t* out1 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort1 , nFrames) ;
	jack_default_audio_sample_t* in2 = (jack_default_audio_sample_t*)jack_port_get_buffer(InputPort2 , nFrames) ;
	jack_default_audio_sample_t* out2 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutputPort2 , nFrames) ;
	jack_default_audio_sample_t mix1[nFrames] , mix2[nFrames] ;

#if PASSTHRU

	// PeriodSize = sizeof(jack_default_audio_sample_t) * nFrames ;
	memcpy(out1 , in1 , PeriodSize) ; memcpy(out2 , in2 , PeriodSize) ;

#else

	for (unsigned int frameN = 0 ; frameN < nFrames ; ++frameN)
	{
		unsigned int frame = FrameN + frameN ; mix1[frameN] = mix2[frameN] = 0 ;
		for (unsigned int loopN = 0 ; loopN < LoopN ; ++loopN)
		{
			mix1[frameN] += LoopBuffers1[loopN][frame] ;
			mix2[frameN] += LoopBuffers2[loopN][frame] ;
		}
	}
	memcpy(out1 , mix1 , PeriodSize) ; memcpy(out2 , mix2 , PeriodSize) ;

#endif

	memcpy(&(RecordBuffer1[FrameN]) , in1 , PeriodSize) ;
	memcpy(&(RecordBuffer2[FrameN]) , in2 , PeriodSize) ;

	return 0 ;
}

int JackIO::SetBufferSizeCallback(jack_nframes_t nFrames , void* arg) { CalculatePeriodSize(nFrames) ; }

void JackIO::ShutdownCallback(void* arg) { Reset() ; exit(1) ; }


// DEBUG
void JackIO::Vardump() { char dbg[256] ;
	sprintf(dbg , "%d" , LoopN) ; Loopidity::App->dbgText0 = dbg ;
	sprintf(dbg , "%d" , FrameN) ; Loopidity::App->dbgText1 = dbg ;
	sprintf(dbg , "%d (%ds)" , NFrames , NFrames / 48000) ; Loopidity::App->dbgText2 = dbg ;
	sprintf(dbg , "%d" , NFramesPerPeriod) ; Loopidity::App->dbgText3 = dbg ;
	sprintf(dbg , "%d" , FrameSize) ; Loopidity::App->dbgText4 = dbg ;
	sprintf(dbg , "%d" , PeriodSize) ; Loopidity::App->dbgText5 = dbg ;
	sprintf(dbg , "%d" , CurrentScene) ; Loopidity::App->dbgText8 = dbg ;
	sprintf(dbg , "%d" , IsAutoRecord) ; Loopidity::App->dbgText9 = dbg ;
	sprintf(dbg , "%d" , IsRecording) ; Loopidity::App->dbgText10 = dbg ;
	sprintf(dbg , "%d" , IsPulseExist) ; Loopidity::App->dbgText11 = dbg ; }
// DEBUG end
