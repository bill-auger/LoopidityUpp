#ifndef _LOOPIDITY_UPP_H_
#define _LOOPIDITY_UPP_H_


#include <CtrlLib/CtrlLib.h>
#include <SysExec/SysExec.h>
using namespace Upp ;
#define LAYOUTFILE <Loopidity/Loopidity.lay>
#include <CtrlCore/lay.h>


#include "loopidity.h"
class Scene ;
#include "scene_upp.h"
class SceneUpp ;


// intervals
#define GUI_UPDATE_INTERVAL_SHORT 125
#define GUI_UPDATE_INTERVAL_LONG 1000

// GUI magnitudes
#define INIT_W 600
#define INIT_H 400
#define STATUS_W 300

// scope magnitudes
#define SCOPE_Y 200
#define SCOPE_MAX 100
#define SCOPE_OPTIMAL 0.8
#define SCOPE_SCALE 100
#define VU_SCALE 100

// colors
#define SCOPE_BG_COLOR Red
#define INSCOPE_LO_COLOR Green
#define INSCOPE_MID_COLOR Yellow
#define INSCOPE_HI_COLOR Red
#define OUTSCOPE_LO_COLOR Green
#define OUTSCOPE_MID_COLOR Yellow
#define OUTSCOPE_HI_COLOR Red

// keycodes
// K_NUMPAD0 is defined as 130992 and K_0 is defined as 65712 in CtrlCore/X11Leys.h 
// on my system the actual keycodes for KP0 are 130915 NLoff and 48 NLon
#define KP0_NLOFF 130915
#define KP0_NLON 48


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

		// error messages
		void alert(String msg) ;

	private:

		// GUI elements
		static LoopidityUpp* App ; // singleton Loopidity instance
		MemoryDialog memDlg ;
		StatusBar status ;
		InfoCtrl statusL , statusR ;
		SceneUpp* scenes[N_SCENES] ;

		// scope peaks cache
		Vector<SAMPLE>* inPeaks ;
		Vector<SAMPLE>* outPeaks ;
		SAMPLE* transientPeaks ;

		// constants
		static const Color STATUS_COLOR_RECORDING ;
		static const Color STATUS_COLOR_PLAYING ;
		static const Color STATUS_COLOR_IDLE ;

		// setup
		void init() ;
		void startLoopidity(unsigned int initBufferSize) ;

		// event handlers
		bool Key(dword key , int count) ;
//		void LeftDown(Point p , dword d) ;

		// drawing
		void Paint(Draw& w) ;
		void drawScopes(Draw& w , Rect winRect) ;
		void drawScene(Draw& d) ;

		// callbacks
		void openMemoryDialog() ;
#if DRAW_WIDGETS
		void updateGUIFast() ;
		void updateGUISlow() ;
#else
		void updateDrawFast() ;
#endif

		// helpers
		String makeTime(unsigned int seconds) ;
		unsigned int getAvailableMemory() ;
		void updateMemory() ;
		void updateLoopProgress() ;
		void updateVUMeters() ;

// DEBUG
static bool DbgShow ;
void showHideCtrls() ;
// DEBUG end
}	;


#endif
