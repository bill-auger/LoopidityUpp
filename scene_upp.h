
#ifndef _SCENE_UPP_H_
#define _SCENE_UPP_H_


#include <CtrlLib/CtrlLib.h>

#include "loopidity.h"
class Scene ;


// testing
//#define N_PEAKS 1000 / GUI_UPDATE_INTERVAL_SHORT * 10
#define HISTOGRAM_Y_SCALE (HISTOGRAM_HEIGHT) / (float)(LOOP_SIZE)
#define HISTOGRAM_SAMPLE_WIDTH 1

// loop magnitudes
#define LOOP_IMG_RES PEAK_MAX * 2 // TODO: fixed to VAL_MAX for now
#define LOOP_BG_IMG_RES 256 // must be 256
#define LOOP_SIZE LOOP_IMG_RES // TODO: make these elements scalable
#define Y_OFFSET (float)(LOOP_SIZE) * 0.25
#define X_PADDING (float)(LOOP_SIZE) * 0.125
#define Y_PADDING (float)(LOOP_SIZE) * 0.333
#define HISTOGRAM_HEIGHT Y_PADDING * 0.5
#define PIE_SLICE_DEGREES 3600 / N_PEAKS
#define PIE_12_OCLOCK -900

// colors
#define WIN_BG_COLOR Black
#define SCENE_BG_COLOR WIN_BG_COLOR
#define PEAK_MAX_COLOR Red
#define PEAK_ZERO_COLOR Color(128 , 128 , 128)
#define CURRENT_SAMPLE_COLOR Yellow
#define ACTIVE_LOOP_COLOR Green // TODO: for histogram may not need
#define INACTIVE_LOOP_COLOR Color(0 , 64 , 0) // TODO: for histogram may not need
#define LOOP_CURRENT_PEAK_COLOR White
#define LOOP_MAX_PEAK_COLOR Red
#define LOOP_IMG_MASK_COLOR Color(128 , 128 , 128)

// string constants
#define LOOP_GRADIENT_IMAGE_KEY "LOOP_GRADIENT_IMAGE_KEY"
#define LOOP_BG_GRADIENT_IMAGE_KEY "LOOP_BG_GRADIENT_IMAGE_KEY"
#define SCENE_FADE_IMAGE_KEY "SCENE_FADE_IMAGE_KEY"


class SceneUpp
{
	friend class LoopidityUpp ;
	friend struct LoopGradientImgMaker ;
	friend struct LoopImgMaker ;
	friend struct PeaksBgGradientImgMaker ;
	friend struct SceneImgMaker ;

	private:

		SceneUpp(Scene* scene , unsigned int sceneNum) :
				scene(scene) , sceneN(sceneNum) , xPadding(0) , loopW(0) ,
				sceneW(0) , sceneH(0) , sceneX(0) , sceneY(0) , sceneL(0) , sceneT(0) ,
				histogramTop(0) , histogramBottom(0) , maxPeak(0) ,
				zeroPeak(0) , minPeak(0) {}

		// audio/peaks data
		Scene* scene ;
		unsigned int sceneN ;

		// drawing dimensions
		unsigned int xPadding ;
		unsigned int loopW ;
		unsigned int sceneW ;
		unsigned int sceneH ;
		unsigned int sceneX ;
		unsigned int sceneY ;
		unsigned int sceneL ;
		unsigned int sceneT ;
		unsigned int histogramTop ;
		unsigned int histogramBottom ;
		unsigned int maxPeak ;
		unsigned int zeroPeak ;
		unsigned int minPeak ;

		// drawing
		void drawScene(Draw& d) ;

		// drawing helpers
		void setDims(Rect winRect , bool isFullScreen) ;
		Image createLoopPeaksMask(unsigned int loopN , unsigned int peakN) ;
		static Image CreateLoopGradientImg() ;
		static Image CreateLoopGradientImgCached() ;
		Image createLoopImg(unsigned int loopN , unsigned int peakN) ;
		Image createLoopImgCached(unsigned int loopN , unsigned int peakN) ;
		static Image CreatePeaksBgGradientImg() ;
		static Image CreatePeaksBgGradientImgCached() ;
		Image createSceneImg() ;
		Image createSceneImgCached(unsigned int w , unsigned int h) ;

// DEBUG
void getMainDbgText(char* dbg) ;
// DEBUG end
} ;


#endif