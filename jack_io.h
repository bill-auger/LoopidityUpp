
#ifndef _JACK_IO_H_
#define _JACK_IO_H_


#include <vector>

#include <jack/jack.h>

#include "loopidity.h"
class Scene ;


class JackIO
{
  public:

		// setup
    static int Init() ;
		static void Reset() ;

		// getters/setters
		static unsigned int GetNFrames() ;
		static const unsigned int GetFrameSize() ;
		static void SetCurrentScene(Scene* currentScene) ;

// DEBUG
static unsigned int GetPeriodSize() { return PeriodSize ; }
// DEBUG

  private:

		// JACK handles
    static jack_client_t* Client ;
    static jack_port_t* InputPort1 ;
    static jack_port_t* InputPort2 ;
    static jack_port_t* OutputPort1 ;
    static jack_port_t* OutputPort2 ;

		// audio data
		static Scene* CurrentScene ;

		// server state
		static unsigned int NFramesPerPeriod ;
		static const unsigned int FRAME_SIZE ;
		static unsigned int PeriodSize ;

		// helpers
		static void CalculatePeriodSize(unsigned int nFrames) ;

		// JACK callbacks
    static int ProcessCallback(jack_nframes_t nframes , void* arg) ;
    static int SetBufferSizeCallback(jack_nframes_t nframes , void* arg) ;
    static void ShutdownCallback(void* arg) ;

} ;


#endif // _JACKIO_H_
