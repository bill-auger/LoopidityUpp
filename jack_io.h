
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
    static bool Init(unsigned int nFrames , Scene* currentScene) ;
		static void Reset(Scene* currentScene) ;

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
		static jack_default_audio_sample_t* RecordBuffer1 ;
		static jack_default_audio_sample_t* RecordBuffer2 ;

		// server state
		static unsigned int NFramesPerPeriod ;
		static const unsigned int FRAME_SIZE ;
		static unsigned int PeriodSize ;

		// misc flags
		static bool IsMonitorInputs ;

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
