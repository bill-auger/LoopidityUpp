
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
	unsigned int sampleRate = JackIO::GetSampleRate() ;
	sprintf(dbg , "%u" , GetCurrentSceneN()) ; app->dbgText11 = dbg ; app->dbgText102.SetText(dbg) ;
	sprintf(dbg , "%u" , NextSceneN) ; app->dbgText12 = dbg ; app->dbgText103.SetText(dbg) ;
	sprintf(dbg , "%u" , Scenes.GetCount()) ; app->dbgText13 = dbg ;
	sprintf(dbg , "%d" , IsRecording) ; app->dbgText14 = dbg ;
	sprintf(dbg , "%u %%" , GetLoopPos() / 10) ; app->dbgText22 = dbg ;

	sprintf(dbg , "%u" , JackIO::GetCurrentScene()->nLoops) ; app->dbgText0 = dbg ;
	sprintf(dbg , "%d (%ds)" , JackIO::GetCurrentScene()->frameN , JackIO::GetCurrentScene()->frameN / sampleRate) ; app->dbgText1 = dbg ;
	sprintf(dbg , "%u (%ds)" , JackIO::GetCurrentScene()->nFrames , JackIO::GetCurrentScene()->nFrames / sampleRate) ; app->dbgText2 = dbg ;
	sprintf(dbg , "%d" , JackIO::GetCurrentScene()->isSaveLoop) ; app->dbgText3 = dbg ;
	sprintf(dbg , "%d" , JackIO::GetCurrentScene()->isPulseExist) ; app->dbgText4 = dbg ;

	Scene* nextScene = Scenes.At(NextSceneN) ;
	sprintf(dbg , "%u" , nextScene->nLoops) ; app->dbgText15 = dbg ;
	sprintf(dbg , "%d (%ds)" , nextScene->frameN , nextScene->frameN / sampleRate) ; app->dbgText16 = dbg ;
	sprintf(dbg , "%u (%ds)" , nextScene->nFrames , nextScene->nFrames / sampleRate) ; app->dbgText17 = dbg ;
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
Vector<Scene*> Loopidity::Scenes ;


/* Scene Class public functions */

Scene::Scene(unsigned int nframes) :
		// buffer iteration
		nFrames(nframes) , frameN(0) , nLoops(0) ,
		// recording state
		isSaveLoop(true) , isPulseExist(false) { }

void Scene::reset()
{
	frameN = nLoops = 0 ; isSaveLoop = true ; isPulseExist = false ;
	for (unsigned int i = 1 ; i < loopBuffers1.GetCount() ; ++i)
		delete loopBuffers1.Pop() ; delete loopBuffers2.Pop() ;
}


// user actions

void Scene::setMode()
{
	if (!isPulseExist) { nFrames = frameN + JackIO::GetNFrames() ; isPulseExist = true ; }
	else isSaveLoop = !isSaveLoop ;
}


// getters/setters

unsigned int Scene::getLoopPos() { return (frameN * 1000) / nFrames ; }


/* Loopidity Class public functions */

// user actions

bool Loopidity::Init(unsigned int recordBufferSize)
{
	if (!recordBufferSize) { PromptOK("ERROR: initBufferSize size is zero") ; return false ; }

	unsigned int nframes = recordBufferSize / JackIO::GetFrameSize() ;
	unsigned int sceneN = N_SCENES ; while (sceneN--) Scenes.Add(new Scene(nframes)) ;
	return JackIO::Init(nframes , Scenes[0]) ;
}

void Loopidity::ToggleScene()
{
	JackIO::SetNextScene(Scenes.At(NextSceneN = (++NextSceneN) % N_SCENES)) ;
	LoopidityUpp::GetApp()->setMode() ;
}

void Loopidity::SetMode()
{
	if (!IsRecording) IsRecording = true ;
	else JackIO::GetCurrentScene()->setMode() ;
	LoopidityUpp::GetApp()->setMode() ;

if (DEBUG) { char dbg[256] ; sprintf(dbg , "Set mode: IsRecording:%d isPulseExist:%d isSaveLoop:%d" , IsRecording , GetIsPulseExist() , GetIsSaveLoop()) ; LoopidityUpp::GetApp()->tempStatusL(dbg) ; }
}


// getters/setters

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
	NextSceneN = 0 ; unsigned int n = N_SCENES ; while (n--) Scenes[n]->reset() ;
	LoopidityUpp::GetApp()->resetGUI() ; JackIO::Reset(Scenes[0]) ;
}
