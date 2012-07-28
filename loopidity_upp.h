#ifndef _LOOPIDITY_UPP_H_
#define _LOOPIDITY_UPP_H_


#include <CtrlLib/CtrlLib.h>
using namespace Upp ;
#define LAYOUTFILE <Loopidity/Loopidity.lay>
#include <CtrlCore/lay.h>


#include "loopidity.h"
#include "jack_io.h"


#define INIT_W 600
#define INIT_H 400
#define STATUS_W 300
#define INSCOPE_Y 300
#define OUTSCOPE_Y 300
#define GUI_UPDATE_INTERVAL_SHORT 125
#define GUI_UPDATE_INTERVAL_LONG 1000
#define N_PEAKS 1000 / GUI_UPDATE_INTERVAL_SHORT * 10
#define SCOPE_MAX 100
#define SCOPE_OPTIMAL 0.8
#define SCOPE_SCALE 100
#define VU_SCALE 100

//#define CONNECT_ARG "--connect"
#define MONITOR_ARG "--nomon"
#define JACK_FAIL_MSG "Could not register JACK client - quitting"


struct MemoryDialog : public TopWindow
{
/* TODO: user defined buffer sizes
	startLoopidity(unsigned int initBufferSize) ; }
*/
	Button okBtn ;

	void DoClose() { Close() ; }

	typedef MemoryDialog CLASSNAME ;

	MemoryDialog()
	{
		SetRect(0 , 0 , 200 , 50) ;
		Add(okBtn.SetLabel("OK").SizePos()) ;
		okBtn <<= THISBACK(DoClose) ;
	}
} ;


class LoopidityUpp : public WithLoopidityLayout<TopWindow>
{
	public:

		typedef LoopidityUpp CLASSNAME ;
		LoopidityUpp() {}
		virtual ~LoopidityUpp() {}

		// singleton 'constructor'
		static LoopidityUpp* New() ;

		// query state
		static LoopidityUpp* GetApp() ;
		void resetGUI() ;

		// getters/setters
		void tempStatusL(const char* msg) ;
		void setStatusL(const char* msg) ;
		void setStatusR(const char* msg) ;
		void setMode() ;

	private:

		// GUI
		static LoopidityUpp* App ; // singleton Loopidity instance
		MemoryDialog memDlg ;
		StatusBar status ;
		InfoCtrl statusL , statusR ;

		// sample data
		jack_port_t* InPort1 ; jack_port_t* InPort2 ;
		jack_port_t* OutPort1 ; jack_port_t* OutPort2 ;
		Vector<jack_default_audio_sample_t> inPeaks ;
		Vector<jack_default_audio_sample_t> outPeaks ;

		// constants
		static const Color STATUS_COLOR_RECORDING ;
		static const Color STATUS_COLOR_PLAYING ;
		static const Color STATUS_COLOR_IDLE ;

		// setup
		void init() ;
		void startLoopidity(unsigned int initBufferSize) ;

		// event handlers
		bool Key(dword key , int count) ;
		void LeftDown(Point p , dword d) ;

		// callbacks
		void Paint(Draw& w) ;
		void openMemoryDialog() ;
		void updateGUIFast() ;
		void updateGUISlow() ;

		// helpers
		String makeTime(unsigned int seconds) ;
		unsigned int getAvailableMemory() ;
		void updateMemory() ;
		void drawScopes(Draw& w) ;
		void updateLoopProgress() ;
		void updateVUMeters() ;
}	;


#endif
