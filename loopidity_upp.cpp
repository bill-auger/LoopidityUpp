
#include <SysExec/SysExec.h>

#include "loopidity_upp.h"
#include "loopidity.h"


/* private variables */

LoopidityUpp* LoopidityUpp::App = 0 ;
//Vector<jack_default_audio_sample_t> inPeaks1 , inPeaks2 ; // TODO:
//Vector<jack_default_audio_sample_t> outPeaks1 , outPeaks2 ; // TODO:


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

void LoopidityUpp::tempStatusL(const char* msg) { statusL.Temporary(msg) ; }

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
}


/* private functions */

// setup

void LoopidityUpp::init()
{
	// initialize GUI
	Title("Loopidity").Sizeable() ; SetRect(0 , 0 , INIT_W , INIT_H) ;
	AddFrame(status) ; status.Set(" ") ; statusL.Set(" ") ; statusR.Set(" ") ;
	status.AddFrame(statusL.Left(STATUS_W)) ; status.AddFrame(statusR.Right(STATUS_W)) ;

if (DEBUG) Loopidity::SetDbgLabels() ;

	// initialize peaks cache
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
		{ inPeaks.push_back(0.0) ; outPeaks.push_back(0.0) ; }

#if MEMORY_CHECK
	// check free memory
	unsigned int nFrames = DEFAULT_BUFFER_SIZE * N_INPUT_CHANNELS * N_SCENES ;
	unsigned int buffersSize = (nFrames * JackIO::GetFrameSize()) / 1000 ;
	unsigned int availableMemory = getAvailableMemory() ;
	if (!availableMemory || !buffersSize) { PromptOK(FREEMEM_FAIL_MSG) ; exit(1) ; }

if (DEBUG) dbgLabel7.SetText("buff / mem") ; char dbg[256] ; sprintf(dbg , "%u / %u" , buffersSize , availableMemory) ; dbgText7 = dbg ;

	if (buffersSize < availableMemory) { if (INIT_LOOPIDITY) startLoopidity(DEFAULT_BUFFER_SIZE) ; }
#if ! STATIC_BUFFER_SIZE // TODO: eliminate STATIC_BUFFER_SIZE
	else SetTimeCallback(1000 , THISBACK(openMemoryDialog)) ; // TODO: do we have a 'GUI running' callback?
#else
	else { sprintf(dbg , "DEFAULT_BUFFER_SIZE too large - quitting - %u / %u" , buffersSize , availableMemory) ; PromptOK(dbg) ; exit(1) ; }
#endif

#else
	if (INIT_LOOPIDITY) startLoopidity(DEFAULT_BUFFER_SIZE) ;
#endif
}

void LoopidityUpp::startLoopidity(unsigned int recordBufferSize)
{
	if (!Loopidity::Init(recordBufferSize)) { PromptOK(JACK_FAIL_MSG) ; exit(1) ; }

	InPort1 = JackIO::GetInPort1() ; InPort2 = JackIO::GetInPort2() ;
	OutPort1 = JackIO::GetOutPort1() ; OutPort2 = JackIO::GetOutPort2() ;

	resetGUI() ;
	SetTimeCallback(-GUI_UPDATE_INTERVAL_SHORT , THISBACK(updateGUIFast)) ;
	SetTimeCallback(-GUI_UPDATE_INTERVAL_LONG , THISBACK(updateGUISlow)) ;
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

void LoopidityUpp::Paint(Draw& w) { drawScopes(w) ; }

void LoopidityUpp::openMemoryDialog() { memDlg.RunAppModal() ; }
//memDlg.Open(this) ;
//memDlg.Open() ;
//memDlg.Run() ;
//PromptOK("ok");//exit(0) ;

void LoopidityUpp::updateGUIFast() { updateLoopProgress() ; updateVUMeters() ; }

void LoopidityUpp::updateGUISlow() { updateMemory() ; }


// helpers

String LoopidityUpp::makeTime(unsigned int seconds)
{
	unsigned int  s = seconds % 60 ; unsigned int m = seconds / 60 ;
	char time[64] ; sprintf(time , "%s%u:%s%u" , (m > 9)? "" : "0" , m , (s > 9)? "" : "0" , s) ;
	return String(time) ;
}

unsigned int LoopidityUpp::getAvailableMemory()
{
	String out ; SysExec("free" , "" , out) ; int idx = out.Find("Swap:" , 0) ;
	out.Remove(idx , out.GetLength() - idx) ; out.Remove(0 , out.ReverseFind(' ')) ;
	return atoi(out) ;
}

void LoopidityUpp::updateMemory()
{
	String scenePos = makeTime(JackIO::GetCurrentScene()->frameN / JackIO::GetSampleRate()) ;
	String sceneLen = makeTime(JackIO::GetCurrentScene()->nFrames / JackIO::GetSampleRate()) ;
	String remaining = makeTime(getAvailableMemory() * 1024 / JackIO::GetFrameSize() / JackIO::GetSampleRate()) ;
char mem[16] ; sprintf(mem , "%ukb" , getAvailableMemory()) ; // TODO: we wont need this
	setStatusR(scenePos + " / " + sceneLen + " - (" + remaining + " remaining) <" + mem + '>') ;
}

void LoopidityUpp::drawScopes(Draw& w)
{
	Rect winRect = GetSize() ; w.DrawRect(winRect , Black()) ;
	unsigned int center = winRect.Width() / 2 ;
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
	{
		unsigned int inX , outX ; inX = center + N_PEAKS - peakN ; outX = center - peakN ;
		unsigned int inY , outY ; inY = inPeaks[peakN] ; outY = outPeaks[peakN] ;
		Color inColor , outColor ; inColor = outColor = Green() ;
		if (inY > SCOPE_MAX * SCOPE_OPTIMAL) inColor = Yellow() ;
		if (outY > SCOPE_MAX * SCOPE_OPTIMAL) inColor = Yellow() ;
		if (inY > SCOPE_MAX) { inY = SCOPE_MAX ; inColor = Red() ; }
		if (outY > SCOPE_MAX) { outY = SCOPE_MAX ; outColor = Red() ; }
		w.DrawLine(inX , INSCOPE_Y - inY , inX , INSCOPE_Y + inY , 1 , inColor) ;
		w.DrawLine(outX , OUTSCOPE_Y - outY , outX , OUTSCOPE_Y + outY , 1 , outColor) ;
	}
}

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
	jack_default_audio_sample_t inHi1 , inHi2 ;
	jack_default_audio_sample_t outHi1 , outHi2 ;
	inHi1 = inHi2 = outHi1 = outHi2 = 0 ;
	for (unsigned int frameNin = 0 ; frameNin < nFrames ; ++frameNin)
	{
		float in1 = fabs(inBuff1[frameNin]) ; float in2 = fabs(inBuff2[frameNin]) ;
		float out1 = fabs(outBuff1[frameNin]) ; float out2 = fabs(outBuff2[frameNin]) ;
		if (in1 > inHi1) inHi1 = in1 ; if (in2 > inHi2) inHi2 = in2 ;
		if (out1 > outHi1) outHi1 = out1 ; if (out2 > outHi2) outHi2 = out2 ;
	}
	inPeaks.Pop() ; inPeaks.Insert(0 , (inHi1 *= SCOPE_SCALE) + (inHi2 *= SCOPE_SCALE)) ;
	outPeaks.Pop() ; outPeaks.Insert(0 , (outHi1 *= SCOPE_SCALE) + (outHi2 *= SCOPE_SCALE)) ;
	inputProgress1.Set(inHi1 , VU_SCALE) ; inputProgress2.Set(inHi2 , VU_SCALE) ;
	outputProgress1.Set(outHi1 , VU_SCALE) ; outputProgress2.Set(outHi2 , VU_SCALE) ;
Refresh() ;
Loopidity::Vardump() ;
}


/* main entry point */

GUI_APP_MAIN { LoopidityUpp::New()->Run() ; }
