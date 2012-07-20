
#include "loopidity.h"


// DEBUG
void Loopidity::SetDbgLabels()
{
#if DEBUG
LoopidityUpp* app = LoopidityUpp::GetApp() ;
app->dbgLabel100.SetText("Loopidity::") ;
app->dbgLabel11.SetText("CurrentSceneN") ;
app->dbgLabel12.SetText("NextSceneN") ;
app->dbgLabel13.SetText("NScenes") ;
app->dbgLabel14.SetText("IsRecording") ;
app->dbgLabel22.SetText("Progress %%") ;

app->dbgLabel102.SetText("Current Scene::") ;
app->dbgLabel0.SetText("LoopN") ;
app->dbgLabel1.SetText("FrameN") ;
app->dbgLabel2.SetText("NFrames") ;
app->dbgLabel3.SetText("IsAutoRecord") ;
app->dbgLabel4.SetText("IsPulseExist") ;
app->dbgLabel5.SetText("") ;
app->dbgLabel6.SetText("") ;

app->dbgLabel103.SetText("Next Scene::") ;
app->dbgLabel15.SetText("LoopN") ;
app->dbgLabel16.SetText("FrameN") ;
app->dbgLabel17.SetText("NFrames") ;
app->dbgLabel18.SetText("IsAutoRecord") ;
app->dbgLabel19.SetText("IsPulseExist") ;
app->dbgLabel20.SetText("") ;
app->dbgLabel21.SetText("") ;

app->dbgLabel101.SetText("JackIO::") ;
app->dbgLabel8.SetText("NFramesPerPeriod") ;
app->dbgLabel9.SetText("FrameSize") ;
app->dbgLabel10.SetText("PeriodSize") ;
#endif
}

void Loopidity::Vardump() {
#if DEBUG
	char dbg[256] ; LoopidityUpp* app = LoopidityUpp::GetApp() ;
	sprintf(dbg , "%u" , GetCurrentSceneN()) ; app->dbgText11 = dbg ; app->dbgText102.SetText(dbg) ;
	sprintf(dbg , "%u" , NextSceneN) ; app->dbgText12 = dbg ; app->dbgText103.SetText(dbg) ;
	sprintf(dbg , "%u" , Scenes.GetCount()) ; app->dbgText13 = dbg ;
	sprintf(dbg , "%d" , IsRecording) ; app->dbgText14 = dbg ;
	sprintf(dbg , "%u %%" , GetLoopPos() / 10) ; app->dbgText22 = dbg ;

	sprintf(dbg , "%u" , JackIO::GetCurrentScene()->loopN) ; app->dbgText0 = dbg ;
	sprintf(dbg , "%d" , JackIO::GetCurrentScene()->frameN) ; app->dbgText1 = dbg ;
	sprintf(dbg , "%u (%ds)" , JackIO::GetCurrentScene()->nFrames , JackIO::GetCurrentScene()->nFrames / JackIO::GetSampleRate()) ; app->dbgText2 = dbg ;
	sprintf(dbg , "%d" , JackIO::GetCurrentScene()->isSaveLoop) ; app->dbgText3 = dbg ;
	sprintf(dbg , "%d" , JackIO::GetCurrentScene()->isPulseExist) ; app->dbgText4 = dbg ;

	Scene* nextScene = Scenes.At(NextSceneN) ;
	sprintf(dbg , "%u" , nextScene->loopN) ; app->dbgText15 = dbg ;
	sprintf(dbg , "%d" , nextScene->frameN) ; app->dbgText16 = dbg ;
	sprintf(dbg , "%u (%ds)" , nextScene->nFrames , nextScene->nFrames / JackIO::GetSampleRate()) ; app->dbgText17 = dbg ;
	sprintf(dbg , "%d" , nextScene->isSaveLoop) ; app->dbgText18 = dbg ;
	sprintf(dbg , "%d" , nextScene->isPulseExist) ; app->dbgText19 = dbg ;

	sprintf(dbg , "%u" , JackIO::GetNFrames()) ; app->dbgText8 = dbg ;
	sprintf(dbg , "%u" , JackIO::GetFrameSize()) ; app->dbgText9 = dbg ;
	sprintf(dbg , "%u" , JackIO::GetPeriodSize()) ; app->dbgText10 = dbg ;
#endif
} // DEBUG end


/* Loopidity Class private varables */

// recording state
bool Loopidity::IsRecording = false ;
unsigned int Loopidity::NextSceneN = 0 ;

// audio data
unsigned int Loopidity::InitBufferSizes[N_SCENES] = DEFAULT_BUFFER_SIZES ;
Vector<Scene*> Loopidity::Scenes ;


/* Scene Class public functions */

Scene::Scene(unsigned int initBufferSize) :
		// buffer iteration
		frameN(0) , loopN(0) , nFrames(initBufferSize / JackIO::GetFrameSize()) ,
		// recording state
		isSaveLoop(true) , isPulseExist(false)
{
if (!initBufferSize) { PromptOK("ERROR: initBufferSize size is zero") ; exit(1) ; }

	loopBuffers1.push_back(recordBuffer1 = new jack_default_audio_sample_t[initBufferSize]()) ;
	loopBuffers2.push_back(recordBuffer2 = new jack_default_audio_sample_t[initBufferSize]()) ;
}


// user actions

void Scene::setMode()
{
/*	if (!isRecording) { frameN = 0 ; isRecording = true ; }
	else*/ if (!isPulseExist) { nFrames = frameN + JackIO::GetNFrames() ; isPulseExist = true ; }
	else isSaveLoop = !isSaveLoop ;
}


// getters/setters

unsigned int Scene::getLoopPos() { return (frameN * 1000) / nFrames ; }


/* Loopidity Class public functions */

// user actions

int Loopidity::Init()
{
	for (unsigned int sceneN = 0 ; sceneN < N_SCENES ; ++sceneN)
		Scenes.push_back(new Scene(InitBufferSizes[sceneN])) ;
	return JackIO::Init(Scenes.At(0)) ;
}

void Loopidity::ToggleScene()
{
	JackIO::SetNextScene(Scenes.At(NextSceneN = (++NextSceneN) % N_SCENES)) ;
	LoopidityUpp::GetApp()->setMode() ;
}

void Loopidity::SetMode()
{
	if (!IsRecording) { JackIO::StartRecording() ; IsRecording = true ; }
	else JackIO::GetCurrentScene()->setMode() ;
	LoopidityUpp::GetApp()->setMode() ;
}


// getters/setters

int Loopidity::SetBufferSizes(unsigned int* initBufferSizes)
{
if (DEBUG) { char dbg[256] ; sprintf(dbg , "InitBufferSizes in=%d %d %d" , InitBufferSizes[0] , InitBufferSizes[1] , InitBufferSizes[2]) ; PromptOK(dbg) ; }
if (DEBUG) { char dbg[256] ; sprintf(dbg , "initBufferSizes=%d %d %d" , initBufferSizes[0] , initBufferSizes[1] , initBufferSizes[2]) ; PromptOK(dbg) ; }

	memcpy(InitBufferSizes , initBufferSizes , N_SCENES * sizeof(unsigned int) ) ;

if (DEBUG) { char dbg[256] ; sprintf(dbg , "InitBufferSizes out=%d %d %d" , InitBufferSizes[0] , InitBufferSizes[1] , InitBufferSizes[2]) ; PromptOK(dbg) ; }
}

unsigned int Loopidity::GetCurrentSceneN()
{ // TODO: the U++ way - class Index has a Find() method but i couldnt get it to work
	unsigned int sceneN = N_SCENES ; Scene* currentScene = JackIO::GetCurrentScene() ;
	while (sceneN-- && Scenes.At(sceneN) != currentScene) ; return sceneN ;
}

unsigned int Loopidity::GetNextSceneN() { return NextSceneN ; }

unsigned int Loopidity::GetLoopPos() { return JackIO::GetCurrentScene()->getLoopPos() ; }

bool Loopidity::GetIsSaveLoop() { return JackIO::GetCurrentScene()->isSaveLoop ; }

bool Loopidity::GetIsRecording() { return IsRecording ; }

bool Loopidity::GetIsPulseExist() { return JackIO::GetCurrentScene()->isPulseExist ; }


/* Loopidity Class private functions */

// setup

void Loopidity::Reset()
{
	for (unsigned int i = 0 ; i < N_SCENES ; ++i) delete Scenes.Pop() ;
	NextSceneN = 0 ;
	LoopidityUpp::GetApp()->resetGUI() ; JackIO::Reset() ; Init() ;
}
