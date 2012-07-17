#ifndef _LOOPIDITY_H_
#define _LOOPIDITY_H_


#include <CtrlLib/CtrlLib.h>


using namespace Upp ;


#define LAYOUTFILE <Loopidity/Loopidity.lay>
#include <CtrlCore/lay.h>

// DEBUG
#define DEBUG 1
#define AUTOSTART 1
// DEBUG end

#define INIT_W 600
#define INIT_H 400
#define STATUS_W 300

#define CONNECT_ARG "--connect"
#define JACK_FAIL_MSG "Could not register JACK client - quitting"


class Loopidity : public WithLoopidityLayout<TopWindow>
{
	public:

		typedef Loopidity CLASSNAME ;
		Loopidity() {}
		virtual ~Loopidity() {}

		static Loopidity* App ; // singleton Loopidity instance

		// singleton 'constructor'
		static Loopidity* New() ;

		// helpers
		void setStatusL(const char* msg) ;
		void setStatusR(const char* msg) ;
		void setStatusRecord(unsigned int sceneN , bool isAutoRecord) ;
		void tempStatusR(const char* msg) ;

	private:

		StatusBar status ;
		InfoCtrl statusL , statusR ;

		// constants
		static const Color SCENE_COLOR_RECORDING ;
		static const Color SCENE_COLOR_PLAYING ;
		static const Color SCENE_COLOR_IDLE ;

		// setup
		void init(bool isConnect) ;

		// event handlers
		bool Key(dword key , int count) ;
		void LeftDown(Point p , dword d) ;

		// helpers
/*
virtual void RightDown(Point, dword)
{
    ProgressInfo f(status); f.Text("Progress:") ;
    for(int i = 0 ; i < 50 ; i++) { f.Set(i , 50) ; Sleep(20) ; }
}
*/
}	;


#endif
