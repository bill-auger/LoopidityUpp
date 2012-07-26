
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
    static int Init(Scene* currentScene) ;
		static void Reset() ;
		static void StartRecording() ;

		// getters/setters
		static jack_port_t* GetInPort1() ;
		static jack_port_t* GetInPort2() ;
		static jack_port_t* GetOutPort1() ;
		static jack_port_t* GetOutPort2() ;
		static unsigned int GetNFrames() ;
		static const unsigned int GetFrameSize() ;
		static Scene* GetCurrentScene() ;
		static void SetNextScene(Scene* nextScene) ;

  private:

		// JACK handles
    static jack_client_t* Client ;
    static jack_port_t* InputPort1 ;
    static jack_port_t* InputPort2 ;
    static jack_port_t* OutputPort1 ;
    static jack_port_t* OutputPort2 ;

		// audio data
		static Scene* CurrentScene ;
		static Scene* NextScene ;

		// server state
		static unsigned int NFramesPerPeriod ;
		static const unsigned int FRAME_SIZE ;
		static unsigned int PeriodSize ;

		// recording state
		static bool IsRecording ;

		// JACK callbacks
    static int ProcessCallback(jack_nframes_t nframes , void* arg) ;
    static int SetBufferSizeCallback(jack_nframes_t nframes , void* arg) ;
    static void ShutdownCallback(void* arg) ;

// DEBUG
public:
static unsigned int GetPeriodSize() { return PeriodSize ; }
static unsigned int GetSampleRate() { return (int)jack_get_sample_rate(Client) ; }
// DEBUG
} ;


#endif // _JACKIO_H_
