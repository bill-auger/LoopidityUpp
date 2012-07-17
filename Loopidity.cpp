
#include "Loopidity.h"
#include "jackio.h"


/* public variables */

Loopidity* Loopidity::App = 0 ;


/* private constants */

const Color Loopidity::SCENE_COLOR_RECORDING = Red() ;
const Color Loopidity::SCENE_COLOR_PLAYING = Yellow() ;
const Color Loopidity::SCENE_COLOR_IDLE = LtGray() ;


/* public functions */

Loopidity* Loopidity::New()
{
	if (App) return App ;

	const Vector<String>& args = CommandLine() ; bool isConnect = (args[0] == CONNECT_ARG) ;
	CtrlLayout(*(App = new Loopidity()) , "Loopidity") ; App->init(isConnect) ; return App ;
}

// helpers

void Loopidity::setStatusL(const char* msg) { statusL.Set(msg) ; }

void Loopidity::setStatusR(const char* msg) { statusR.Set(msg) ; }

void Loopidity::setStatusRecord(unsigned int sceneN , bool isAutoRecord)
{
	Color activeColor = (isAutoRecord)? SCENE_COLOR_RECORDING : SCENE_COLOR_PLAYING ;
	verseLabel.SetInk((sceneN == 0)? activeColor : SCENE_COLOR_IDLE) ;
	chorusLabel.SetInk((sceneN == 1)? activeColor : SCENE_COLOR_IDLE) ;
	bridgeLabel.SetInk((sceneN == 2)? activeColor : SCENE_COLOR_IDLE) ;
}

void Loopidity::tempStatusR(const char* msg) { statusR.Temporary(msg) ; }


/* private functions */

void Loopidity::init(bool isConnect)
{
	Title("Loopidity").Sizeable() ; SetRect(0 , 0 , INIT_W , INIT_H) ;
	AddFrame(status) ; status.Set(" ") ; statusL.Set(" ") ; statusR.Set(" ") ;
	status.AddFrame(statusL.Left(STATUS_W)) ; status.AddFrame(statusR.Right(STATUS_W)) ;

#if DEBUG
dbgLabel0.SetText("LoopN") ;
dbgLabel1.SetText("FrameN") ;
dbgLabel2.SetText("NFrames") ;
dbgLabel3.SetText("NFramesPerPeriod") ;
dbgLabel4.SetText("FrameSize") ;
dbgLabel5.SetText("PeriodSize") ;
dbgLabel8.SetText("CurrentScene") ;
dbgLabel9.SetText("IsAutoRecord") ;
dbgLabel10.SetText("IsRecording") ;
dbgLabel11.SetText("IsPulseExist") ;
#endif

#if AUTOSTART
	if (!JackIO::Init(isConnect)) { PromptOK(JACK_FAIL_MSG) ; exit(1) ; }
#endif
}


// event handlers

void Loopidity::LeftDown(Point p , dword d) { if (!AUTOSTART && !JackIO::Init(false)) { PromptOK(JACK_FAIL_MSG) ; exit(1) ; } }

bool Loopidity::Key(dword key , int count)
{
	switch(key)
	{
case K_RETURN: case K_F5: exit(0) ; break ;

		case K_SPACE: JackIO::SetLoop() ; break ;
		default: return false ; return true ; // TODO: not sure what to return
	}
}


/* main entry point */

GUI_APP_MAIN { Loopidity::New()->Run() ; }
