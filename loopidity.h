
#ifndef _LOOPIDITY_H_
#define _LOOPIDITY_H_


#include <vector>

#include "loopidity_upp.h"
#include "jack_io.h"


// DEBUG
#define DEBUG 1
#define AUTOSTART 1
#define CLICKSTART 1
#define INIT_JACK 1
#define LOOP_COUNTER 1
#define DSP 1
#define PASSTHRU 0
// TODO: user defined buffer sizes
#define STATIC_BUFFER_SIZE 1
// DEBUG end


#define JACK_CLIENT_NAME "Loopidity"
//#define MAX_BUFFER 33554432 // 1024 * 1024 * 32 (approx 3 min / chan @ 48k)
//#define MAX_BUFFER 16777216 // 2097152
#define DEFAULT_BUFFER_SIZE 1048576
#define DEFAULT_BUFFER_SIZES { DEFAULT_BUFFER_SIZE , DEFAULT_BUFFER_SIZE , DEFAULT_BUFFER_SIZE }
#define N_SCENES 3

#define FREEMEM_FAIL_MSG "could not determine available memory - quitting"

// K_NUMPAD0 is defined as 130992 and K_0 is defined as 65712 in CtrlCore/X11Leys.h 
// on my system the actual keycodes are 130915 NLoff and 48 NLon
#define KP0_NLOFF 130915
#define KP0_NLON 48


class Scene
{
// TODO: some protection
	public:

		typedef Scene CLASSNAME ;
		Scene(unsigned int initBufferSize) ;
		virtual ~Scene() {}

		// audio data
		jack_default_audio_sample_t* recordBuffer1 ;
		jack_default_audio_sample_t* recordBuffer2 ;
		Vector<jack_default_audio_sample_t*> loopBuffers1 ;
		Vector<jack_default_audio_sample_t*> loopBuffers2 ;

		// buffer iteration
		unsigned int loopN ;
		unsigned int frameN ;
		unsigned int nFrames ;

		// recording state
		bool isSaveLoop ;
		bool isPulseExist ;

		// user actions
    void setMode() ;

		// query state
    unsigned int getLoopPos() ;
} ;


class Loopidity
{
	public:

		// user actions
    static int Init() ;
		static void ToggleScene() ;
    static void SetMode() ;

		// getters/setters
		static int SetBufferSizes(unsigned int* initBufferSizes) ;
		static unsigned int GetCurrentSceneN() ;
		static unsigned int GetNextSceneN() ;
    static unsigned int GetLoopPos() ;
		static bool GetIsSaveLoop() ;
    static bool GetIsRecording() ;
		static bool GetIsPulseExist() ;

	private:

		// recording state
		static bool IsRecording ;
		static unsigned int NextSceneN ;

		// audio data
		static unsigned int InitBufferSizes[N_SCENES] ;
		static Vector<Scene*> Scenes ;

		// setup
    static void Reset() ;

// DEBUG
public: static void SetDbgLabels() ; static void Vardump() ;
//static void dbgAddScene() { Scenes.push_back(new Scene(DEFAULT_BUFFER_SIZE)) ; }
// DEBUG end
} ;


#endif
