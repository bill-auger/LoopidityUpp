#ifndef _JACKIOMONOPASSTHRU_H_
#define _JACKIOMONOPASSTHRU_H_


#include <jack/jack.h>

#include "Loopidity.h"


class JackIOmonopassthru
{
  public:

    static int Init(Loopidity* app) ;
    static int Reset() ;
    static void Close() ;

  private:

    static jack_port_t* InputPort ;
    static jack_port_t* OutputPort ;
    static jack_client_t* Client ;
		static Loopidity* App ;

    static int Process(jack_nframes_t nframes , void* arg) ;
    static void JackShutdown(void* arg) ;
} ;

#endif // _JACKIO_H_
