/***************************************************************************

    file                 : grcam.h
    created              : Mon Aug 21 20:55:02 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grcam.h,v 1.28 2006/02/22 09:45:30 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GRCAM_H_
#define _GRCAM_H_

#include <track.h>
#include <car.h>
#include <raceman.h>
#ifdef WIN32
#include <float.h>
#endif

class cGrCamera;
class cGrScreen;

GF_TAILQ_HEAD(GrCamHead, cGrCamera);

/* Camera interface */
class cGrCamera 
{
 private:
    GF_TAILQ_ENTRY(cGrCamera) link;
    int			id;		/* Camera Id */
    int			drawCurrent;	/* flag to draw the current car */
    int			drawDriver;	/* flag to draw the driver */
    int			drawBackground;	/* flag to draw the background */
    int			mirrorAllowed;	/* flag to allox the display of mirror */

 protected:
    sgVec3 speed;
    sgVec3 eye;
    sgVec3 center;
    sgVec3 up;
    class cGrScreen	*screen;	/* screen where the camera is attached */
    
 public:
    cGrCamera(class cGrScreen *myscreen, int myid = 0, int mydrawCurrent = 0, int mydrawdrv = 0, int mydrawBackground = 0, int mymirrorAllowed = 0) {
		screen = myscreen;
		id = myid;
		drawCurrent = mydrawCurrent;
		drawDriver = mydrawdrv;
		drawBackground = mydrawBackground;
		mirrorAllowed = mymirrorAllowed;
		speed[0] = speed[1] = speed[2] = 0.0;
		eye[0] = eye[1] = eye[2] = 0.0;
		center[0] = center[1] = center[2] = 0.0;
		up[0] = up[1] = 0.0; up[2] = 1.0;
    }

    virtual ~cGrCamera() {};
    
    virtual void update(tCarElt *car, tSituation *s) = 0;	/* Change the camera if necessary */
    virtual void setProjection(void) = 0;
    virtual void setModelView(void) = 0;
    virtual void setZoom(int cmd) = 0;				/* Set the zoom with commands */
    virtual void loadDefaults(char *attr) = 0;			/* Load the default values from parameter file */
    virtual void onSelect(tCarElt *car, tSituation *s) = 0;	/* called when the camera is selected */

    virtual float getLODFactor(float x, float y, float z) = 0;	/* Get the LOD factor for an object located at x,y,z */

    /* Set the camera view */
    void action(void) {
	setProjection();
	setModelView();
    }
    
    /* Get the camera info */
    int getId(void)		{ return id; }
    int getDrawCurrent(void)	{ return drawCurrent; }
    int getDrawDriver(void)	{ return drawDriver; }
    int getDrawBackground(void)	{ return drawBackground; }
    int isMirrorAllowed(void)	{ return mirrorAllowed; }

    t3Dd *getPos(void) {
	static t3Dd pos;
	pos.x = eye[0];
	pos.y = eye[1];
	pos.z = eye[2];
	return &pos;
    }
    sgVec3 *getPosv(void) {
	return &eye;
    }
    sgVec3 *getSpeedv(void) {
        return &speed;
    }
    t3Dd *getCenter(void) {
	static t3Dd pos;
	pos.x = center[0];
	pos.y = center[1];
	pos.z = center[2];
	return &pos;
    }
    sgVec3 *getCenterv(void) {
	return &center;
    }
    t3Dd *getUp(void) {
	static t3Dd pos;
	pos.x = up[0];

	pos.y = up[1];
	pos.z = up[2];
	return &pos;
    }
    sgVec3 *getUpv(void) {
	return &up;
    }

    virtual float getFovY(void) {
	return 67.5; // override in perspective camera
    }
    
    /* Add the camera in the corresponding list */
    void add(tGrCamHead *head) {
	GF_TAILQ_INSERT_TAIL(head, this, link);
    }
    
    /* Remove the camera from the corresponding list */
    void remove(tGrCamHead *head) {
	GF_TAILQ_REMOVE(head, this, link);
    }

    /* Get the squared distance between the car and the camera */
    float getDist2(tCarElt *car);

    cGrCamera *next(void) {
	return GF_TAILQ_NEXT(this, link);
    }
};


class cGrPerspCamera : public cGrCamera
{
 protected:
    float fovy;
    float fovymin;
    float fovymax;
    float fovydflt;
    float fnear;
    float ffar;
    float fogstart;
    float fogend;
    
 public:
    cGrPerspCamera(class cGrScreen *myscreen, int id, int drawCurr, int drawDrv, int drawBG, int mirrorAllowed,
		   float myfovy, float myfovymin, float myfovymax,
		   float myfnear, float myffar = 1500.0, float myfogstart = 1400.0, float myfogend = 1500.0);
    
    virtual void update(tCarElt *car, tSituation *s) = 0;	/* Change the camera if necessary */
    void setProjection(void);
    void setModelView(void);
    void loadDefaults(char *attr);
    void setZoom(int cmd);
    float getLODFactor(float x, float y, float z);
    float getFogStart(void) { return fogstart; }
    float getFogEnd(void) { return fogend; }

    cGrPerspCamera *next(void) {
	return (cGrPerspCamera *)cGrCamera::next();
    }

   void limitFov(void)  {}
   void onSelect(tCarElt *car, tSituation *s) {}

   virtual float getFovY(void) {
       return fovy;
   }

};



class cGrOrthoCamera : public cGrCamera
{
 protected:
    float left;
    float right;
    float bottom;
    float top;

 public:
    cGrOrthoCamera(class cGrScreen *myscreen, float myleft, float myright, float mybottom, float mytop)
	: cGrCamera(myscreen) {
	left   = myleft;
	right  = myright;
	bottom = mybottom;
	top    = mytop;
    }

    void setProjection(void);
    void setModelView(void);

    void update(tCarElt *car, tSituation *s) { }
    float getLODFactor(float x, float y, float z) { return 1; }
    void loadDefaults(char *attr) { }
    void setZoom(int cmd) { }
    void onSelect(tCarElt *car, tSituation *s) {}
};

class cGrBackgroundCam : public cGrPerspCamera
{
 public:
    cGrBackgroundCam(class cGrScreen *myscreen)
	: cGrPerspCamera(myscreen, 0, 0, 0, 1, 0,
			 67.5f, 67.5f, 67.5f,
			 0.1f, 2000.0f, 100000.0f, 100000.0f) {
    }
    
    void update(tCarElt *car, tSituation *s) {}

    void update(cGrCamera *curCam);
    void limitFov(void) { }
};

class cGrCarCamMirror : public cGrPerspCamera
{
 protected:
    int		vpx, vpy, vpw, vph;	/* viewport size */
    int		tw, th;			/* texture size */
    int		mx, my, mw, mh;		/* drawing area */
    float	tsu, tsv, teu, tev;	/* texture coord */
    GLuint	tex;			/* texture */
    cGrOrthoCamera *viewCam;
    
 public:
    cGrCarCamMirror(cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float myfovy, float myfovymin, float myfovymax,
		    float myfnear, float myffar = 1500.0,
		    float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 1,
			 myfovy, myfovymin, myfovymax,
			 myfnear, myffar, myfogstart, myfogend) {
	glGenTextures (1, &tex);
	limitFov();
	viewCam = NULL;
    }
    virtual ~cGrCarCamMirror ();

    void update (tCarElt *car, tSituation *s);
    void limitFov (void);

    void setViewport (int x, int y, int w, int h);
    void setPos (int x, int y, int w, int h);
    void activateViewport (void);
    void store (void);
    void display (void);
};


#define GR_ZOOM_IN 	0
#define GR_ZOOM_OUT 	1
#define GR_ZOOM_MAX 	2
#define GR_ZOOM_MIN 	3
#define GR_ZOOM_DFLT    4

void grCamCreateSceneCameraList(class cGrScreen *myscreen, tGrCamHead *cams, tdble fovFactor);

#endif /* _GRCAM_H_ */ 



