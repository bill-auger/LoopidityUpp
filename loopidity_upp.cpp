
#include "loopidity_upp.h"


// DEBUG
bool LoopidityUpp::DbgShow = true ;


/* LoopidityUpp class private variables */

LoopidityUpp* LoopidityUpp::App = 0 ;
SceneUpp* scenes[N_SCENES] = {0} ;
Vector<SAMPLE>* inPeaks = 0 ;
Vector<SAMPLE>* outPeaks = 0 ;
SAMPLE* transientPeaks = 0 ;

/* LoopidityUpp class private constants */

const Color LoopidityUpp::STATUS_COLOR_RECORDING = Color(255 , 0 , 0) ;
const Color LoopidityUpp::STATUS_COLOR_PLAYING = Color(191 , 191 , 0) ;
const Color LoopidityUpp::STATUS_COLOR_IDLE = Color(127 , 127 , 127) ;


/* LoopidityUpp class public functions */

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

void LoopidityUpp::alert(String msg) { PromptOK(msg) ; }


/* private functions */

// setup

void LoopidityUpp::init()
{
	// initialize GUI
#if DRAW_WIDGETS
	Title("Loopidity").Sizeable().SetRect(0 , 0 , INIT_W , INIT_H) ;
#else
	Title("Loopidity").Sizeable().Maximize() ;
#endif
	AddFrame(status) ; status.Set(" ") ; statusL.Set(" ") ; statusR.Set(" ") ;
	status.AddFrame(statusL.Left(STATUS_W)) ; status.AddFrame(statusR.Right(STATUS_W)) ;

if (DEBUG) Loopidity::SetDbgLabels() ;

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
	// initialize Loopidity JackIO and SceneUpp classes
	Scene** aScenes = Loopidity::Init(recordBufferSize) ;
	for (unsigned int sceneN = 0 ; sceneN < N_SCENES ; ++sceneN)
		scenes[sceneN] = new SceneUpp(aScenes[sceneN] , sceneN) ;

	// get handles to scope and VU peaks caches
	inPeaks = Loopidity::GetInPeaksCache() ; outPeaks = Loopidity::GetOutPeaksCache() ;
	transientPeaks = Loopidity::GetTransientPeaksCache() ;

#if DRAW_WIDGETS
	resetGUI() ;
	SetTimeCallback(-GUI_UPDATE_INTERVAL_SHORT , THISBACK(updateGUIFast)) ;
	SetTimeCallback(-GUI_UPDATE_INTERVAL_LONG , THISBACK(updateGUISlow)) ;
#else
	showHideCtrls() ;
	SetTimeCallback(-GUI_UPDATE_INTERVAL_SHORT , THISBACK(updateDrawFast)) ;
#endif
}


// event handlers

// DEBUG
void LoopidityUpp::showHideCtrls()
{
	DbgShow = !DbgShow ; Ctrl* child = GetFirstChild() ;
	do child->Show(DbgShow) ; while (child = child->GetNext()) ;
}
// DEBUG end

bool LoopidityUpp::Key(dword key , int count)
{
	switch(key)
	{
// DEBUG
case K_F5: exit(0) ; break ;
case K_RETURN: showHideCtrls() ; break ;
// DEBUG end

		case K_SPACE: Loopidity::SetMode() ; break ;
		case K_NUMPAD0: case KP0_NLON: case KP0_NLOFF: Loopidity::ToggleScene() ; break ;
		default: return false ; return true ; // TODO: not sure what to return
	}
}

//void LoopidityUpp::LeftDown(Point p , dword d) {}


// drawing
bool IsBetterWayToDoThis = false ;
void LoopidityUpp::Paint(Draw& wd)
{
		Rect winRect = GetSize() ;

#if DRAW_BG_FULL
		wd.DrawRect(winRect , WIN_BG_COLOR) ;
#endif

#if DRAW_SCENES
		unsigned int currentSceneN = Loopidity::GetCurrentSceneN() ;
		SceneUpp* scene = scenes[currentSceneN] ;

#if ! DRAW_BG_FULL
if (!IsBetterWayToDoThis) { wd.DrawRect(winRect , WIN_BG_COLOR) ; IsBetterWayToDoThis = true ; }
else wd.DrawRect(scene->sceneX , scene->sceneY , scene->sceneW , scene->sceneH , WIN_BG_COLOR) ;
#endif

		// draw current scene
		scene->setDims(winRect , true) ; scene->drawScene(wd) ;
		for (unsigned int sceneN = 0 ; sceneN < N_SCENES ; ++sceneN)
		{
			if (sceneN == currentSceneN) continue ;

			// draw inactive scenes
			scene = scenes[sceneN] ; scene->setDims(winRect , false) ;
			Image img = scene->createSceneImgCached(winRect.Width() , winRect.Height()) ;
			wd.DrawImage(scene->sceneX , scene->sceneY , img) ;
		}
#endif

#if DRAW_SCOPES
	drawScopes(wd , winRect) ;
#endif

#if DRAW_SCENES && DRAW_DEBUG_TEXT
char dbg[255] ; scenes[currentSceneN]->getMainDbgText(dbg) ; wd.DrawRect(MAIN_DEBUG_TEXT_POS , winRect.Width() , winRect.Height() , WIN_BG_COLOR) ; wd.DrawText(MAIN_DEBUG_TEXT_POS , dbg , Roman(18) , White) ;
#endif
}

void LoopidityUpp::drawScopes(Draw& d , Rect winRect)
{
	unsigned int center = winRect.Width() / 2 ;
#if ! DRAW_BG_FULL
	unsigned int scopeL = center - N_PEAKS , scopeT = SCOPE_Y - SCOPE_MAX ; 
	unsigned int scopeW = N_PEAKS * 2 , scopeH = SCOPE_Y + SCOPE_MAX ;
	d.DrawRect(scopeL , scopeT , scopeW , scopeH , SCOPE_BG_COLOR) ;
#endif
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
	{
		unsigned int inX = center + N_PEAKS - peakN , outX = center - peakN ;
		unsigned int inH = (unsigned int)(*inPeaks)[peakN] ; 
		unsigned int outH = (unsigned int)(*outPeaks)[peakN] ;
		Color inColor , outColor ;
		if (inH > SCOPE_MAX) { inH = SCOPE_MAX ; inColor = INSCOPE_HI_COLOR ; }		
		else if (inH > SCOPE_MAX * SCOPE_OPTIMAL) inColor = INSCOPE_MID_COLOR ;
		else inColor = INSCOPE_LO_COLOR ;
		if (outH > SCOPE_MAX) { outH = SCOPE_MAX ; outColor = OUTSCOPE_HI_COLOR ; }
		else if (outH > SCOPE_MAX * SCOPE_OPTIMAL) outColor = OUTSCOPE_MID_COLOR ;
		else outColor = OUTSCOPE_LO_COLOR ;
		d.DrawLine(outX , SCOPE_Y - outH , outX , SCOPE_Y + outH , 1 , outColor) ;
		d.DrawLine(inX , SCOPE_Y - inH , inX , SCOPE_Y + inH , 1 , inColor) ;
	}
}


// callbacks

void LoopidityUpp::openMemoryDialog() { memDlg.RunAppModal() ; }
//memDlg.Open(this) ;
//memDlg.Open() ;
//memDlg.Run() ;
//PromptOK("ok");//exit(0) ;

#if DRAW_WIDGETS
void LoopidityUpp::updateGUIFast() { updateLoopProgress() ; updateVUMeters() ; Refresh() ; }

void LoopidityUpp::updateGUISlow() { updateMemory() ; }
#else
void LoopidityUpp::updateDrawFast() { Refresh() ; }
#endif


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
	Scene* currentScene = JackIO::GetCurrentScene() ; unsigned int sampleRate = JackIO::GetSampleRate() ;
	String scenePos = makeTime(currentScene->getFrameN() / sampleRate) ;
	String sceneLen = makeTime(currentScene->getNFrames() / sampleRate) ;
	String remaining = makeTime(getAvailableMemory() * 1024 / JackIO::GetFrameSize() / sampleRate) ;
char mem[16] ; sprintf(mem , "%ukb" , getAvailableMemory()) ; // TODO: we wont need this
	setStatusR(scenePos + " / " + sceneLen + " - (" + remaining + " remaining) <" + mem + '>') ;
}

void LoopidityUpp::updateLoopProgress() { loopProgress.Set(Loopidity::GetLoopPos() , 1000) ; }

void LoopidityUpp::updateVUMeters()
{
	Loopidity::ScanTransientPeaks() ;
	inputProgress1.Set(transientPeaks[0] , VU_SCALE) ;
	inputProgress2.Set(transientPeaks[1] , VU_SCALE) ;
	outputProgress1.Set(transientPeaks[2] , VU_SCALE) ;
	outputProgress2.Set(transientPeaks[3] , VU_SCALE) ;

Loopidity::Vardump() ;
}


/* main entry point */

GUI_APP_MAIN { LoopidityUpp::New()->Run() ; }
