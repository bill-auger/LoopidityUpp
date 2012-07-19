
#include <SysExec/SysExec.h>

#include "loopidity_upp.h"
#include "loopidity.h"


/* private variables */

LoopidityUpp* LoopidityUpp::App = 0 ;


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
	bool isAutoRecord = Loopidity::IsAutoRecord() ;
	bool isRecording = Loopidity::IsRecording() ;
	bool isPulseExist = Loopidity::IsPulseExist() ;

	// scene indicators
	unsigned int sceneN = Loopidity::GetSceneN() ;
	Color activeColor = (isAutoRecord)? STATUS_COLOR_RECORDING : STATUS_COLOR_PLAYING ;
	verseLabel.SetInk((sceneN == 0)? activeColor : STATUS_COLOR_IDLE) ;
	chorusLabel.SetInk((sceneN == 1)? activeColor : STATUS_COLOR_IDLE) ;
	bridgeLabel.SetInk((sceneN == 2)? activeColor : STATUS_COLOR_IDLE) ;

	// progress bar
	if (isRecording && !isPulseExist) loopProgress.SetColor(STATUS_COLOR_RECORDING) ;
	else loopProgress.SetColor(STATUS_COLOR_IDLE) ;

if (DEBUG) { char dbg[256] ; sprintf(dbg , "Set mode: %d %d %d" , isAutoRecord , isRecording , isPulseExist) ; tempStatusR(dbg) ; }
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
	unsigned int buffersSize = DEFAULT_BUFFER_SIZE * N_SCENES / JackIO::GetFrameSize() ;
	unsigned int availableMemory = atoi(out) ;
	if (!availableMemory) { PromptOK(FREEMEM_FAIL_MSG	) ; exit(1) ; }

#if DEBUG
dbgLabel0.SetText("LoopN") ;
dbgLabel1.SetText("FrameN") ;
dbgLabel2.SetText("NFrames") ;
dbgLabel3.SetText("LoopPos") ;
dbgLabel4.SetText("FrameSize") ;
dbgLabel5.SetText("NFramesPerPeriod") ;
dbgLabel6.SetText("PeriodSize") ;
dbgLabel7.SetText("buff/mem") ; char dbg[256] ; sprintf(dbg , "%u/%u" , buffersSize , availableMemory) ; dbgText7 = dbg ;

dbgLabel8.SetText("SceneN") ;
dbgLabel9.SetText("IsAutoRecord") ;
dbgLabel10.SetText("IsRecording") ;
dbgLabel11.SetText("IsPulseExist") ;
#endif

#if AUTOSTART

	if (buffersSize < availableMemory) startLoopidity() ;
#if STATIC_BUFFER_SIZE
	else { PromptOK(dbg) ; exit(1) ; }
#else
		// TODO: do we have a 'GUI running' callback?
	else SetTimeCallback(1000 , THISBACK(openMemoryDialog)) ;
#endif

#endif
}

void LoopidityUpp::startLoopidity()
{
	if (!Loopidity::Init()) { PromptOK(JACK_FAIL_MSG) ; exit(1) ; }

	resetGUI() ; SetTimeCallback(-GUI_UPDATE_INTERVAL , THISBACK(updateProgress)) ;
}


// event handlers

void LoopidityUpp::LeftDown(Point p , dword d) {}

bool LoopidityUpp::Key(dword key , int count)
{
if (DEBUG) { char dbg[256] ; sprintf(dbg , "key=%d" , key) ; tempStatusR(dbg) ; }

	switch(key)
	{
case K_RETURN: case K_F5: exit(0) ; break ;

		case K_SPACE: Loopidity::SetMode() ; break ;
		case K_NUMPAD0: Loopidity::ToggleScene() ; break ;
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

void LoopidityUpp::updateProgress()
	{ loopProgress.Set((Loopidity::IsRecording())? Loopidity::GetLoopPos() : 0 , 1000) ; }


/* main entry point */

GUI_APP_MAIN { LoopidityUpp::New()->Run() ; }