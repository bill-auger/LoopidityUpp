
#include "loopidity.h"


/* Loopidity Class private varables */

unsigned int Loopidity::InitBufferSizes[N_SCENES] = DEFAULT_BUFFER_SIZES ;
unsigned int Loopidity::SceneN = 0 ;
Scene* Loopidity::CurrentScene = 0 ;
Vector<Scene*> Loopidity::Scenes ;


/* Scene Class public functions */

Scene::Scene(unsigned int initBufferSize) :
		frameN(0) , loopN(0) , nFrames(initBufferSize / JackIO::GetFrameSize()) ,
		isAutoRecord(true) , isRecording(false) , isPulseExist(false)
{
	loopBuffers1.push_back(recordBuffer1 = new jack_default_audio_sample_t[DEFAULT_BUFFER_SIZE]()) ;
	loopBuffers2.push_back(recordBuffer2 = new jack_default_audio_sample_t[DEFAULT_BUFFER_SIZE]()) ;
}


// user actions

void Scene::setMode()
{
	if (!isRecording) { frameN = 0 ; isRecording = true ; }
	else if (!isPulseExist) { nFrames = frameN + JackIO::GetNFrames() ; isPulseExist = true ; }
	else isAutoRecord = !isAutoRecord ;

	LoopidityUpp::GetApp()->setMode() ;

Loopidity::Vardump() ;
}


// getters/setters

unsigned int Scene::getLoopPos() { return (frameN * 1000) / nFrames ; }


/* Loopidity Class public functions */

// user actions

int Loopidity::Init()
{
	for (unsigned int sceneN = 0 ; sceneN < N_SCENES ; ++sceneN)
		Scenes.push_back(new Scene(InitBufferSizes[sceneN])) ;
	JackIO::SetCurrentScene(CurrentScene = Scenes[SceneN]) ; return JackIO::Init() ;
}

void Loopidity::ToggleScene()
	{ JackIO::SetCurrentScene(CurrentScene = Scenes[SceneN = (++SceneN) % N_SCENES]) ; }

void Loopidity::SetMode() { CurrentScene->setMode() ; }


// getters/setters

int Loopidity::SetBufferSizes(unsigned int* initBufferSizes)
{
if (DEBUG) { char dbg[256] ; sprintf(dbg , "InitBufferSizes in=%d %d %d" , InitBufferSizes[0] , InitBufferSizes[1] , InitBufferSizes[2]) ; PromptOK(dbg) ; }
if (DEBUG) { char dbg[256] ; sprintf(dbg , "initBufferSizes=%d %d %d" , initBufferSizes[0] , initBufferSizes[1] , initBufferSizes[2]) ; PromptOK(dbg) ; }

	memcpy(InitBufferSizes , initBufferSizes , N_SCENES * sizeof(unsigned int) ) ;

if (DEBUG) { char dbg[256] ; sprintf(dbg , "InitBufferSizes out=%d %d %d" , InitBufferSizes[0] , InitBufferSizes[1] , InitBufferSizes[2]) ; PromptOK(dbg) ; }
}

unsigned int Loopidity::GetSceneN() { return SceneN ; }

Scene* Loopidity::GetCurrentScene() { return CurrentScene ; } // TODO: just send JackIO a handle

unsigned int Loopidity::GetLoopPos() { return CurrentScene->getLoopPos() ; }

bool Loopidity::IsAutoRecord() { return CurrentScene->isAutoRecord ; }

bool Loopidity::IsRecording() { return CurrentScene->isRecording ; }

bool Loopidity::IsPulseExist() { return CurrentScene->isPulseExist ; }


/* Loopidity Class private functions */

// setup

void Loopidity::Reset()
{
	for (unsigned int i = 0 ; i < N_SCENES ; ++i) delete Scenes.Pop() ;
	CurrentScene = 0 ; SceneN = 0 ;
	LoopidityUpp::GetApp()->resetGUI() ; JackIO::Reset() ; Init() ;
}


// DEBUG
void Loopidity::Vardump() {
#if DEBUG
	char dbg[256] ; LoopidityUpp* app = LoopidityUpp::GetApp() ;
	sprintf(dbg , "%u" , CurrentScene->loopN) ; app->dbgText0 = dbg ;
	sprintf(dbg , "%u" , CurrentScene->frameN) ; app->dbgText1 = dbg ;
	sprintf(dbg , "%u (%ds)" , CurrentScene->nFrames , CurrentScene->nFrames / 48000) ; app->dbgText2 = dbg ;
	sprintf(dbg , "%u %%" , GetLoopPos() / 10) ; app->dbgText3 = dbg ;
	sprintf(dbg , "%u" , JackIO::GetFrameSize()) ; app->dbgText4 = dbg ;
	sprintf(dbg , "%u" , JackIO::GetNFrames()) ; app->dbgText5 = dbg ;
	sprintf(dbg , "%u" , JackIO::GetPeriodSize()) ; app->dbgText6 = dbg ;
//	sprintf(dbg , "%u" , Scenes.GetCount()) ; app->dbgText7 = dbg ;

	sprintf(dbg , "%u" , SceneN) ; app->dbgText8 = dbg ;
	sprintf(dbg , "%d" , CurrentScene->isAutoRecord) ; app->dbgText9 = dbg ;
	sprintf(dbg , "%d" , CurrentScene->isRecording) ; app->dbgText10 = dbg ;
	sprintf(dbg , "%d" , CurrentScene->isPulseExist) ; app->dbgText11 = dbg ;
#endif
} // DEBUG end
