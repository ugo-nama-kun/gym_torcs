/***************************************************************************

    file        : grscreen.h
    created     : Thu May 15 22:11:19 CEST 2003
    copyright   : (C) 2003 by Eric Espié
    email       : eric.espie@torcs.org 
    version     : $Id: grscreen.h,v 1.13 2006/05/01 21:37:05 berniw Exp $
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _GRSCREEN_H_
#define _GRSCREEN_H_

#include <car.h>
#include "grcam.h"
#include "grboard.h"

class cGrScreen {
	protected:
		int id;
	
		tCarElt	*curCar;		// Current car viewed.
		tCarElt	**cars;			// List of cars.
	
		int	curCamHead;							// The current camera list.
		tGrCamHead		cams[10];				// From F2 to F11.
	
		class cGrPerspCamera *curCam;			// The current camera.
		class cGrCarCamMirror *mirrorCam;		// The mirror camera.
		class cGrPerspCamera *dispCam;			// The display camera.
		class cGrOrthoCamera *boardCam;			// The board camera.
		class cGrBackgroundCam *bgCam;			// The background camera.
	
		class cGrBoard *board;					// The board.
	
		int drawCurrent;						// Should the current car be drawn.
	
		int scrx, scry, scrw, scrh;
		float viewRatio;
	
		int active;								// Is the screen activated.
	
		int selectNextFlag;
		int selectPrevFlag;
		int mirrorFlag;
	
		void loadParams(tSituation *s);			// Load from parameters files.

	public:
		cGrScreen(int id);
		~cGrScreen();
	
		void activate(int x, int y, int w, int h);
		void desactivate(void);
		void setZoom(long zoom);
		int  isInScreen(int x, int y);
		void update(tSituation *s, float Fps);
		void camDraw(tSituation *s);
		void updateCurrent(tSituation *s);

		void selectCamera(long cam);
		void selectBoard(long brd);	
		void selectTrackMap();
	
		void setCurrentCar(tCarElt *newCurCar);
		void initCams(tSituation *s);
		void initBoard(void);
	
		void selectNextCar(void);
		void selectPrevCar(void);
		void switchMirror(void);
	
		tCarElt *getCurrentCar(void) { return curCar; }
		cGrCamera* getCurCamera(void) { return curCam; }
		float getViewRatio(void) { return viewRatio; }
		int getCurCamHead(void) { return curCamHead; }
		int isActive(void) { return active; }
		int getId(void) { return id; }
		int getScrX (void) { return scrx; }
		int getScrY (void) { return scry; }
		int getScrW (void) { return scrw; }
		int getScrH (void) { return scrh; }
};


#endif /* _GRSCREEN_H_ */




