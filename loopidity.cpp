
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

	Scene* nextScene = Scenes[NextSceneN] ;
	sprintf(dbg , "%u" , nextScene->nLoops) ; app->dbgText15 = dbg ;
	sprintf(dbg , "%d (%ds)" , nextScene->frameN , nextScene->frameN / sampleRate) ; app->dbgText16 = dbg ;
	sprintf(dbg , "%u (%ds)" , nextScene->nFrames , nextScene->nFrames / sampleRate) ; app->dbgText17 = dbg ;
	sprintf(dbg , "%d" , nextScene->isSaveLoop) ; app->dbgText18 = dbg ;
	sprintf(dbg , "%d" , nextScene->isPulseExist) ; app->dbgText19 = dbg ;

	sprintf(dbg , "%u" , JackIO::GetNFramesPerPeriod()) ; app->dbgText8 = dbg ;
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
unsigned int Loopidity::NFramesPerPeriod = 0 ;
jack_port_t* Loopidity::InPort1 = 0 ;
jack_port_t* Loopidity::InPort2 = 0 ;
jack_port_t* Loopidity::OutPort1 = 0 ;
jack_port_t* Loopidity::OutPort2 = 0 ;
Vector<SAMPLE> Loopidity::InPeaks ;
Vector<SAMPLE> Loopidity::OutPeaks ;
SAMPLE Loopidity::TransientPeaks[N_PORTS] = {0} ;


/* Scene Class public functions */

Scene::Scene(unsigned int nframes) :
		// peaks cache
		hiScenePeaks() , hiLoopPeaks(), highestScenePeak(0) ,
		// buffer iteration
		nFrames(nframes) , frameN(0) , nLoops(0) ,
		// recording state
		isSaveLoop(true) , isPulseExist(false)
{
	// fill testing loops array and preload hiPeaks cache
	for (unsigned int loopN = 0 ; loopN < N_LOOPS ; ++loopN) loops[loopN] = new Loop() ;
#if MOCK_PEAKS_DATA
	rescanPeaks() ;
#endif
}

void Scene::reset()
{
	frameN = nLoops = 0 ; isSaveLoop = true ; isPulseExist = false ;
	for (unsigned int i = 1 ; i < loopBuffers1.GetCount() ; ++i)
		delete loopBuffers1.Pop() ; delete loopBuffers2.Pop() ;
}


// peaks cache
//TODO: on add new loop scanPeaks()
void Scene::scanPeaks(Loop* loop , unsigned int loopN)
{
	unsigned int* peaks = loop->peaks ; unsigned int aPeak ;
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
	{
		aPeak = peaks[peakN] ;
		// find the loudest peak for this loop
		if (hiLoopPeaks[loopN] < aPeak) hiLoopPeaks[loopN] = aPeak ;
		
		// find the loudest of each peak from all loops of the current scene
		if (hiScenePeaks[peakN] < aPeak) hiScenePeaks[peakN] = aPeak ;
	}
	// find the loudest of all peaks of all loops of the current scene
	if (highestScenePeak < hiLoopPeaks[loopN]) highestScenePeak = hiLoopPeaks[loopN] ;
}

//TODO: on delete loop rescanPeaks()
void Scene::rescanPeaks()
{
	ZeroArray(hiLoopPeaks) ; ZeroArray(hiScenePeaks) ; highestScenePeak = 0 ;
	for (unsigned int i = 0 ; i < N_LOOPS ; ++i) scanPeaks(loops[i] , i) ;
}


// recording state

void Scene::setMode()
{
	if (isPulseExist) isSaveLoop = !isSaveLoop ;
	else { nFrames = frameN + JackIO::GetNFramesPerPeriod() ; isPulseExist = true ; }
}


// getters/setters

unsigned int Scene::getLoopPos() { return (frameN * 1000) / nFrames ; }


/* Loopidity Class public functions */

// user actions

Scene** Loopidity::Init(unsigned int recordBufferSize)
{
	LoopidityUpp* app = LoopidityUpp::GetApp() ;
	if (!recordBufferSize) { app->alert(ZERO_BUFFER_SIZE_MSG) ; exit(1) ; }

	unsigned int nFrames = recordBufferSize / JackIO::GetFrameSize() ;
	unsigned int sceneN = N_SCENES ; while (sceneN--) Scenes.Add(new Scene(nFrames)) ;
	if (Scenes.GetCount() != N_SCENES) { app->alert(INSUFFICIENT_MEMORY_MSG) ; exit(1) ; }
	if (!JackIO::Init(nFrames , Scenes[0])) { app->alert(JACK_FAIL_MSG) ; exit(1) ; }

	// initialize scope peaks cache
	for (unsigned int peakN = 0 ; peakN < N_PEAKS ; ++peakN)
		{ InPeaks.push_back(0.0) ; OutPeaks.push_back(0.0) ; }

	// get handles on JACK ports for scope peaks
	InPort1 = JackIO::GetInPort1() ; InPort2 = JackIO::GetInPort2() ;
	OutPort1 = JackIO::GetOutPort1() ; OutPort2 = JackIO::GetOutPort2() ;

	return Scenes ;
}

void Loopidity::ToggleScene()
{
	JackIO::SetNextScene(Scenes[NextSceneN = (++NextSceneN) % N_SCENES]) ;
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
	while (sceneN-- && Scenes[sceneN] != currentScene) ; return sceneN ;
}

unsigned int Loopidity::GetNextSceneN() { return NextSceneN ; }

unsigned int Loopidity::GetLoopPos() { return JackIO::GetCurrentScene()->getLoopPos() ; }

bool Loopidity::GetIsSaveLoop() { return JackIO::GetCurrentScene()->isSaveLoop ; }

bool Loopidity::GetIsRecording() { return IsRecording ; }

bool Loopidity::GetIsPulseExist() { return JackIO::GetCurrentScene()->isPulseExist ; }

void Loopidity::SetNFramesPerPeriod(unsigned int nFrames) { NFramesPerPeriod = nFrames ; }


Vector<SAMPLE>* Loopidity::GetInPeaksCache() { return &InPeaks ; }

Vector<SAMPLE>* Loopidity::GetOutPeaksCache() { return &OutPeaks ; }

SAMPLE* Loopidity::GetTransientPeaksCache() { return TransientPeaks ; }

void Loopidity::ScanTransientPeaks()
{
	SAMPLE inPeak1 = GetTransientPeak(InPort1) ; SAMPLE inPeak2 = GetTransientPeak(InPort2) ;
	SAMPLE outPeak1 = GetTransientPeak(OutPort1) ; SAMPLE outPeak2 = GetTransientPeak(OutPort2) ;
	// load scope peaks
	InPeaks.Pop() ; InPeaks.Insert(0 , (inPeak1 *= SCOPE_SCALE) + (inPeak2 *= SCOPE_SCALE)) ;
	OutPeaks.Pop() ; OutPeaks.Insert(0 , (outPeak1 *= SCOPE_SCALE) + (outPeak2 *= SCOPE_SCALE)) ;
	// load VU peaks
	TransientPeaks[0] = inPeak1 ; TransientPeaks[1] = inPeak2 ;	
	TransientPeaks[2] = outPeak1 ; TransientPeaks[3] = outPeak2 ;
}


/* Loopidity Class private functions */

// setup

void Loopidity::Reset()
{
	NextSceneN = 0 ; unsigned int n = N_SCENES ; while (n--) Scenes[n]->reset() ;
	LoopidityUpp::GetApp()->resetGUI() ; JackIO::Reset(Scenes[0]) ;
}


// helpers

SAMPLE Loopidity::GetTransientPeak(jack_port_t* port)
{
	SAMPLE peak = 0 , *buff = (SAMPLE*)jack_port_get_buffer(port , NFramesPerPeriod) ;
	for (unsigned int frameNin = 0 ; frameNin < NFramesPerPeriod ; ++frameNin)
		{ float sample = fabs(buff[frameNin]) ; if (peak < sample) peak = sample ; }

	return peak ;
}