#ifndef _JACKIO_H_
#define _JACKIO_H_


#include <vector>

#include <jack/jack.h>

#include "Loopidity.h"

// DEBUG
#define PASSTHRU 0
// DEBUG end

#define JACK_CLIENT_NAME "Loopidity"
#define MAX_BUFFER 33554432 // 1024 * 1024 * 32

#define BUFF_SIZE_MSG "JACK buffer size has changed - quitting"


class JackIO
{
  public:

		// user actions
    static int Init(bool isConnectPorts) ;
    static void Reset() ;
    static void SetLoop() ;

  private:

		// JACK handles
    static jack_client_t* Client ;
    static jack_port_t* InputPort1 ;
    static jack_port_t* InputPort2 ;
    static jack_port_t* OutputPort1 ;
    static jack_port_t* OutputPort2 ;

		// audio data
		static jack_default_audio_sample_t* RecordBuffer1 ;
		static jack_default_audio_sample_t* RecordBuffer2 ;
		static Vector<jack_default_audio_sample_t*> LoopBuffers1 ;
		static Vector<jack_default_audio_sample_t*> LoopBuffers2 ;

		// buffer iteration
		static unsigned int FrameSize ;
		static unsigned int PeriodSize ;
		static unsigned int NFramesPerPeriod ;
		static unsigned int NFrames ;
		static unsigned int FrameN ;
		static unsigned int LoopN ;

		// state flags
		static unsigned int CurrentScene ;
		static bool IsAutoRecord ;
		static bool IsRecording ;
		static bool IsPulseExist ;

		// helpers
		static void CalculatePeriodSize(unsigned int nFrames) ;

		// JACK callbacks
    static int ProcessCallback(jack_nframes_t nframes , void* arg) ;
    static int SetBufferSizeCallback(jack_nframes_t nframes , void* arg) ;
    static void ShutdownCallback(void* arg) ;

// DEBUG
static void Vardump() ;
// DEBUG end
} ;


#endif // _JACKIO_H_
