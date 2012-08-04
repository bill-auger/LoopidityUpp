
#ifndef _LOOPIDITY_H_
#define _LOOPIDITY_H_


// DEBUG
#define DEBUG 1
#define MEMORY_CHECK 1 // if 0 choose DEFAULT_BUFFER_SIZE wisely
#define INIT_LOOPIDITY 1
#define INIT_JACK 1
#define LOOP_COUNTER 1
#define DSP 1
#define PASSTHRU 0
// TODO: user defined buffer sizes
#define STATIC_BUFFER_SIZE 1
// DEBUG end

// GUI DEBUG
#define DRAW_BG_FULL 1
#define DRAW_SCENES 0
#define DRAW_SCOPES 1
#define DRAW_WIDGETS 1
#define DRAW_PEAK_BARS 1
#define DRAW_HISTOGRAMS 1
#define DRAW_LOOP_PEAKS 1
#define DRAW_LOOPS 1
#define DRAW_LOOP_MASKS 1
#define DRAW_LOOP_GRADIENTS 1
#define DRAW_SCENE_FADE 1

#define DRAW_DEBUG_TEXT 1
#define MAIN_DEBUG_TEXT_POS 0 , winRect.Height() - 20
#define SCENE_DEBUG_TEXT_POS sceneL , sceneT

#define MOCK_PEAKS_DATA 0
// GUI DEBUG end


// quantities
//#define DEFAULT_BUFFER_SIZE 33554432 // 2^25 (approx 3 min @ 48k)
//#define DEFAULT_BUFFER_SIZE 25165824 // 1024 * 1024 * 24 (approx 135 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 16777216 // 2^24 (approx 90 sec @ 48k)
#define DEFAULT_BUFFER_SIZE 8388608 // 2^23 (approx 45 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 2097152 // 2^21 (approx 10 sec @ 48k)
//#define DEFAULT_BUFFER_SIZE 1048576 // 2^20 (approx 5 sec @ 48k)

// TODO: implement setting N_CHANNELS via cmd line arg - GetTransientPeaks and updateVUMeters are especially brittle now
#define N_INPUT_CHANNELS 2 // TODO: nyi - only used for memory check and scope cache
#define N_OUTPUT_CHANNELS 2 // TODO: nyi - only used for N_PORTS
#define N_PORTS N_INPUT_CHANNELS + N_OUTPUT_CHANNELS // TODO: nyi - only used for scope cache
#define N_SCENES 3
#define N_LOOPS 9 // N_LOOPS_PER_SCENE
#define N_PEAKS 100 // should be divisible into 3600
#define PEAK_MAX 50
#define VAL_INIT PEAK_MAX / 2 // TODO: testing

// string constants
//#define CONNECT_ARG "--connect"
#define JACK_CLIENT_NAME "Loopidity"
#define MONITOR_ARG "--nomon"
#define JACK_FAIL_MSG "ERROR: Could not register JACK client - quitting"
#define FREEMEM_FAIL_MSG "ERROR: Could not determine available memory - quitting"
#define ZERO_BUFFER_SIZE_MSG "ERROR: initBufferSize is zero - quitting"
#define INSUFFICIENT_MEMORY_MSG "ERROR: Insufficient memory - quitting"

// aliases
#define SAMPLE jack_default_audio_sample_t


#include <vector>

#include "jack_io.h"
#include "loopidity_upp.h"


class Loop
{
	public:
		// initialize peaks cache
#if MOCK_PEAKS_DATA
		Loop() { for (unsigned int i = 0 ; i < N_PEAKS ; ++i) peaks[i] = Random(VAL_INIT) ; }
#else
		Loop() { for (unsigned int i = 0 ; i < N_PEAKS ; ++i) peaks[i] = 0 ; }
#endif

		unsigned int peaks[N_PEAKS] ;
} ;


class Scene
{
	friend class JackIO ;
	friend class Loopidity ;
	friend class SceneUpp ;

	private:

		typedef Scene CLASSNAME ;
		Scene(unsigned int initBufferSize) ;
		virtual ~Scene() {}

		// audio data
		Vector<SAMPLE*> loopBuffers1 ;
		Vector<SAMPLE*> loopBuffers2 ;

		// peaks cache
		// TODO: if all we use the Loop class for is holding a peaks array
		// then just integrate it into unsigned int peaks[N_LOOPS][N_PEAKS] ;
Loop* loops[N_LOOPS] ;
//		Vector<unsigned int*> peaks[N_PEAKS] ;
// hiCurrentSample = the loudest of the currently playing samples in the current scene
// hiLoopSamples[] = the loudest of all samples for each loop of the current scene (nyi)
// highestLoopSample = the loudest of all samples in all loops of the current scene (nyi)
		unsigned int hiScenePeaks[N_PEAKS] ;
		unsigned int hiLoopPeaks[N_LOOPS] ;
		unsigned int highestScenePeak ;

		// buffer iteration
		unsigned int nFrames ;
		unsigned int frameN ;
		unsigned int nLoops ;

		// recording state
		bool isSaveLoop ;
		bool isPulseExist ;

		// setup
		void reset() ;

		// peaks cache
		void scanPeaks(Loop* loop , unsigned int loopN) ;
		void rescanPeaks() ;

		// recording state
    void setMode() ;

		// getters/setters
    unsigned int getLoopPos() ;

public: unsigned int getNFrames() { return nFrames ; } // for GUI ctrls may not need
public: unsigned int getFrameN() { return frameN ; } // for GUI ctrls may not need
} ;


class Loopidity
{
	public:

		// user actions
    static Scene** Init(unsigned int recordBufferSize) ;
		static void ToggleScene() ;
    static void SetMode() ;

		// getters/setters
		static unsigned int GetCurrentSceneN() ;
		static unsigned int GetNextSceneN() ;
    static unsigned int GetLoopPos() ;
		static bool GetIsSaveLoop() ;
    static bool GetIsRecording() ;
		static bool GetIsPulseExist() ;
		static void SetNFramesPerPeriod(unsigned int nFrames) ;
		static Vector<SAMPLE>* GetInPeaksCache() ;
		static Vector<SAMPLE>* GetOutPeaksCache() ;
		static SAMPLE* GetTransientPeaksCache() ;
		static void ScanTransientPeaks() ;

	private:

		// recording state
		static bool IsRecording ;
		static unsigned int NextSceneN ;

		// audio data
		static Vector<Scene*> Scenes ;
		static unsigned int NFramesPerPeriod ;

		// transient sample data
		static jack_port_t* InPort1 ;
		static jack_port_t* InPort2 ;
		static jack_port_t* OutPort1 ;
		static jack_port_t* OutPort2 ;
		static Vector<SAMPLE> InPeaks ;
		static Vector<SAMPLE> OutPeaks ;
		static SAMPLE TransientPeaks[N_PORTS] ;

		// setup
    static void Reset() ;

		// helpers
		static SAMPLE GetTransientPeak(jack_port_t* port) ;

// DEBUG
public: static void SetDbgLabels() ; static void Vardump() ;
//static void dbgAddScene() { Scenes.push_back(new Scene(DEFAULT_BUFFER_SIZE)) ; }
// DEBUG end
} ;


#endif
