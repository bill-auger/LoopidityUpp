#ifndef _LOOPIDITY_UPP_H_
#define _LOOPIDITY_UPP_H_


#include <CtrlLib/CtrlLib.h>
using namespace Upp ;
#define LAYOUTFILE <Loopidity/Loopidity.lay>
#include <CtrlCore/lay.h>

#include "loopidity.h"


#define INIT_W 600
#define INIT_H 400
#define STATUS_W 300
#define GUI_UPDATE_INTERVAL 125

#define CONNECT_ARG "--connect"
#define JACK_FAIL_MSG "Could not register JACK client - quitting"


struct MemoryDialog : public TopWindow
{
/* TODO: user defined buffer sizes
	unsigned int bufferSizes[N_SCENES] = { 1,2,3 } ;
	Loopidity::SetBufferSizes(bufferSizes) ;
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
		void setStatusL(const char* msg) ;
		void setStatusR(const char* msg) ;
		void setMode() ;
		void tempStatusR(const char* msg) ;

	private:

		// GUI
		static LoopidityUpp* App ; // singleton Loopidity instance
		MemoryDialog memDlg ;
		StatusBar status ;
		InfoCtrl statusL , statusR ;

		// constants
		static const Color STATUS_COLOR_RECORDING ;
		static const Color STATUS_COLOR_PLAYING ;
		static const Color STATUS_COLOR_IDLE ;

		// setup
		void init() ;
		void startLoopidity() ;

		// event handlers
		bool Key(dword key , int count) ;
		void LeftDown(Point p , dword d) ;

		// callbacks
		void openMemoryDialog() ;
		void updateProgress() ;
}	;


#endif
