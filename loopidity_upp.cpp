
#include <SysExec/SysExec.h>

#include "loopidity_upp.h"
#include "loopidity.h"


/* private variables */

LoopidityUpp* LoopidityUpp::App = 0 ;
//Vector<jack_default_audio_sample_t> inPeaks1 , inPeaks2 ; // TODO: make these class memebers
//Vector<jack_default_audio_sample_t> outPeaks1 , outPeaks2 ; // TODO: make these class memebers

/* private constants */

const Color LoopidityUpp::STATUS_COLOR_RECORDING = Color(255 , 0 , 0) ;
const Color LoopidityUpp::STATUS_COLOR_PLAYING = Color(191 , 191 , 0) ;
const Color LoopidityUpp::STATUS_COLOR_IDLE = Color(127 , 127 , 127) ;


/* public functions */

LoopidityUpp* LoopidityUpp::New()
{
	if (!App) { CtrlLayout(*(App = new LoopidityUpp()) , "Loopidity") ; App->init() ; }
	return App ;
}

LoopidityUpp* LoopidityUpp::GetApp() { return App ; }

void LoopidityUpp::resetGUI() { setMode() ; }


// getters/setters

void LoopidityUpp::setStatusL(const char* msg) { statusL.Set(msg) ; }

void LoopidityUpp::setStatusR(const char* msg) { statusR.Set(msg) ; }

void LoopidityUpp::setMode()
{
	unsigned int currentSceneN = Loopidity::GetCurrentSceneN() ;
	unsigned int nextSceneN = Loopidity::GetNextSceneN() ;
	bool isSaveLoop = Loopidity::GetIsSaveLoop() ;
	bool isRecording = Loopidity::GetIsRecording() ;
	bool isPulseExist = Loopidity::GetIsPulseExist() ;

	// current scene indicator
	Color activeColor = (isSaveLoop)? STATUS_COLOR_RECORDING : STATUS_COLOR_PLAYING ;
	verseLabel.SetInk((currentSceneN == 0)? activeColor : STATUS_COLOR_IDLE) ;
	chorusLabel.SetInk((currentSceneN == 1)? activeColor : STATUS_COLOR_IDLE) ;
	bridgeLabel.SetInk((currentSceneN == 2)? activeColor : STATUS_COLOR_IDLE) ;

	// next scene indicator
	verseLabel.SetFrame((nextSceneN == 0)? BlackFrame() : NullFrame()) ;
	chorusLabel.SetFrame((nextSceneN == 1)? BlackFrame() : NullFrame()) ;
	bridgeLabel.SetFrame((nextSceneN == 2)? BlackFrame() : NullFrame()) ;

	// progress bar
	if (isRecording && !isPulseExist) loopProgress.SetColor(STATUS_COLOR_RECORDING) ;
	else loopProgress.SetColor(STATUS_COLOR_IDLE) ;

if (DEBUG) { char dbg[256] ; sprintf(dbg , "Set mode: %d %d %d" , isSaveLoop , isRecording , isPulseExist) ; tempStatusR(dbg) ; }
}

void LoopidityUpp::tempStatusR(const char* msg) { statusR.Temporary(msg) ; }


/* private functions */

// setup

void LoopidityUpp::init()
{
	// initialize GUI
	Title("Loopidity").Sizeable() ; SetRect(0 , 0 , INIT_W , INIT_H) ;
	AddFrame(status) ; status.Set(" ") ; statusL.Set(" ") ; statusR.Set(" ") ;
	status.AddFrame(statusL.Left(STATUS_W)) ; status.AddFrame(statusR.Right(STATUS_W)) ;

	// check free memory
	String out ; SysExec("free" , "" , out) ; int idx = out.Find("Swap:" , 0) ;
	out.Remove(idx , out.GetLength() - idx) ; out.Remove(0 , out.ReverseFind(' ')) ;
	unsigned int nFrames = DEFAULT_BUFFER_SIZE * N_INPUT_CHANNELS * N_SCENES ;
	unsigned int buffersSize = (nFrames * JackIO::GetFrameSize()) / 1000 ;
	unsigned int availableMemory = atoi(out) ;
	if (!buffersSize || !availableMemory) { PromptOK(FREEMEM_FAIL_MSG) ; exit(1) ; }

#if DEBUG
dbgLabel7.SetText("buff / mem") ; char dbg[256] ; sprintf(dbg , "%u / %u" , buffersSize , availableMemory) ; dbgText7 = dbg ;
Loopidity::SetDbgLabels() ;
#endif

	// initialize peaks cache
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
	{
		inPeaks1.push_back(0.0) ; inPeaks2.push_back(0.0) ;
		outPeaks1.push_back(0.0) ; outPeaks2.push_back(0.0) ;
	}

#if AUTOSTART
#if MEMORY_CHECK
	if (buffersSize < availableMemory) startLoopidity() ;
#if STATIC_BUFFER_SIZE
	else if (DEBUG) { sprintf(dbg , "DEFAULT_BUFFER_SIZES too large - quitting - %u / %u" , buffersSize , availableMemory) ; PromptOK(dbg) ; exit(1) ; }
#else
		// TODO: do we have a 'GUI running' callback?
	else SetTimeCallback(1000 , THISBACK(openMemoryDialog)) ;
#endif
#else
	startLoopidity() ;
#endif
#endif
}

void LoopidityUpp::startLoopidity()
{
	if (!Loopidity::Init()) { PromptOK(JACK_FAIL_MSG) ; exit(1) ; }

	InPort1 = JackIO::GetInPort1() ; InPort2 = JackIO::GetInPort2() ;
	OutPort1 = JackIO::GetOutPort1() ; OutPort2 = JackIO::GetOutPort2() ;

	resetGUI() ; SetTimeCallback(-GUI_UPDATE_INTERVAL , THISBACK(updateGUI)) ;
}


// event handlers

void LoopidityUpp::LeftDown(Point p , dword d) {}

bool LoopidityUpp::Key(dword key , int count)
{
	switch(key)
	{
case K_RETURN: case K_F5: exit(0) ; break ;

		case K_SPACE: Loopidity::SetMode() ; break ;
		case K_NUMPAD0: case KP0_NLON: case KP0_NLOFF: Loopidity::ToggleScene() ; break ;
		default: return false ; return true ; // TODO: not sure what to return
	}
}
/*
virtual void RightDown(Point, dword)
{
    ProgressInfo f(status); f.Text("Progress:") ;
    for(int i = 0 ; i < 50 ; i++) { f.Set(i , 50) ; Sleep(20) ; }
}
*/


// callbacks

void LoopidityUpp::openMemoryDialog() { memDlg.RunAppModal() ; startLoopidity() ; }
//memDlg.Open(this) ;
//memDlg.Open() ;
//memDlg.Run() ;
//PromptOK("ok");//exit(0) ;

void LoopidityUpp::updateGUI() { updateLoopProgress() ; updateVUMeters() ; }


// helpers

void LoopidityUpp::updateLoopProgress() { loopProgress.Set(Loopidity::GetLoopPos() , 1000) ; }

void LoopidityUpp::updateVUMeters()
{
	Scene* currentScene = JackIO::GetCurrentScene() ;
	unsigned int nFrames = JackIO::GetNFrames() ;	
	unsigned int frameN = (currentScene->frameN)? currentScene->frameN - nFrames : 0 ;
	jack_default_audio_sample_t* inBuff1 = (jack_default_audio_sample_t*)jack_port_get_buffer(InPort1 , nFrames) ;
	jack_default_audio_sample_t* inBuff2 = (jack_default_audio_sample_t*)jack_port_get_buffer(InPort2 , nFrames) ;
	jack_default_audio_sample_t* outBuff1 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutPort1 , nFrames) ;
	jack_default_audio_sample_t* outBuff2 = (jack_default_audio_sample_t*)jack_port_get_buffer(OutPort2 , nFrames) ;
	jack_default_audio_sample_t inHi1 , inHi2 , inPeak1 , inPeak2 ;
	jack_default_audio_sample_t outHi1 , outHi2 , outPeak1 , outPeak2 ;	
	inHi1 = inHi2 = inPeak1 = inPeak2 = outHi1 = outHi2 = outPeak1 = outPeak2 = 0 ;

	for (unsigned int frameNin = 0 ; frameNin < nFrames ; ++frameNin)
	{
		// output VU meter
		float s1 = fabs(outBuff1[frameNin]) ; float s2 = fabs(outBuff2[frameNin]) ;
		if (s1 > outHi1) outHi1 = s1 ; if (s2 > outHi2) outHi2 = s2 ;
		// output peaks
		outPeaks1.Pop() ; outPeaks1.push_back(s1) ; outPeaks2.Pop() ; outPeaks2.push_back(s2) ;
		// input VU meter
		s1 = fabs(inBuff1[frameNin]) ; s2 = fabs(inBuff2[frameNin]) ;
		if (s1 > inHi1) inHi1 = s1 ; if (s2 > inHi2) inHi2 = s2 ;
		// input peaks
		inPeaks1.Pop() ; inPeaks1.push_back(s1) ; inPeaks2.Pop() ; inPeaks2.push_back(s2) ;
	}
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
	{
		// output peaks
		float peak1 = fabs(outPeaks1[peakN]) ; float peak2 = fabs(outPeaks2[peakN]) ;
		if (peak1 > outPeak1) outPeak1 = peak1 ; if (peak2 > outPeak2) outPeak2 = peak2 ;
		// input peaks
		peak1 = fabs(inPeaks1[peakN]) ; peak2 = fabs(inPeaks2[peakN]) ;
		if (peak1 > inPeak1) inPeak1 = peak1 ; if (peak2 > inPeak2) inPeak2 = peak2 ;
	}
	inputProgress1.Set(inPeak1 * 10000 , 100) ; inputProgress2.Set(inPeak2 * 10000 , 100) ;
	outputProgress1.Set(outPeak1 * 10000 , 100) ; outputProgress2.Set(outPeak2 * 10000 , 100) ;
// TODO: static peaks display

Loopidity::Vardump() ;
}


/* main entry point */

GUI_APP_MAIN { LoopidityUpp::New()->Run() ; }
