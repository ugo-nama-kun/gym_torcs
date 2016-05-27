/***************************************************************************

    file                 : grcam.cpp
    created              : Mon Aug 21 20:55:32 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grcam.cpp,v 1.50 2007/11/06 20:43:31 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <memory.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <plib/ssg.h>

#include <robottools.h>
#include <graphic.h>
#include "grcam.h"
#include "grscreen.h"
#include "grscene.h"
#include "grshadow.h"
#include "grskidmarks.h"
#include "grsmoke.h"
#include "grcar.h"
#include "grmain.h"
#include "grutil.h"
#include <tgfclient.h>

static char path[1024];


float
cGrCamera::getDist2 (tCarElt *car)
{
    float dx = car->_pos_X - eye[0];
    float dy = car->_pos_Y - eye[1];

    return dx * dx + dy * dy;
}


static void
grMakeLookAtMat4 ( sgMat4 dst, const sgVec3 eye, const sgVec3 center, const sgVec3 up )
{
  // Caveats:
  // 1) In order to compute the line of sight, the eye point must not be equal
  //    to the center point.
  // 2) The up vector must not be parallel to the line of sight from the eye
  //    to the center point.

  /* Compute the direction vectors */
  sgVec3 x,y,z;

  /* Y vector = center - eye */
  sgSubVec3 ( y, center, eye ) ;

  /* Z vector = up */
  sgCopyVec3 ( z, up ) ;

  /* X vector = Y cross Z */
  sgVectorProductVec3 ( x, y, z ) ;

  /* Recompute Z = X cross Y */
  sgVectorProductVec3 ( z, x, y ) ;

  /* Normalize everything */
  sgNormaliseVec3 ( x ) ;
  sgNormaliseVec3 ( y ) ;
  sgNormaliseVec3 ( z ) ;

  /* Build the matrix */
#define M(row,col)  dst[row][col]
  M(0,0) = x[0];    M(0,1) = x[1];    M(0,2) = x[2];    M(0,3) = 0.0;
  M(1,0) = y[0];    M(1,1) = y[1];    M(1,2) = y[2];    M(1,3) = 0.0;
  M(2,0) = z[0];    M(2,1) = z[1];    M(2,2) = z[2];    M(2,3) = 0.0;
  M(3,0) = eye[0];  M(3,1) = eye[1];  M(3,2) = eye[2];  M(3,3) = 1.0;
#undef M
}


cGrPerspCamera::cGrPerspCamera(class cGrScreen *myscreen, int id, int drawCurr, int drawDrv, int drawBG, int mirrorAllowed,
			       float myfovy, float myfovymin, float myfovymax,
			       float myfnear, float myffar, float myfogstart, float myfogend)
    : cGrCamera(myscreen, id, drawCurr, drawDrv, drawBG, mirrorAllowed)
{
    fovy     = myfovy;
    fovymin  = myfovymin;
    fovymax  = myfovymax;
    fnear    = myfnear;
    ffar     = myffar;
    fovydflt = myfovy;
    fogstart = myfogstart;
    fogend   = myfogend;
    
}

void cGrPerspCamera::setProjection(void)
{
    // PLib takes the field of view as angles in degrees. However, the
    // aspect ratio really aplies to lengths in the projection
    // plane. So we have to transform the fovy angle to a length in
    // the projection plane, apply the aspect ratio and transform the
    // result back to an angle. Care needs to be taken to because the
    // tan and atan functions operate on angles in radians. Also,
    // we're only interested in half the viewing angle.
    float fovx = atan(screen->getViewRatio() * tan(fovy * M_PI / 360.0)) * 360.0 / M_PI;
    grContext.setFOV(fovx, fovy);
    grContext.setNearFar(fnear, ffar);
}

void cGrPerspCamera::setModelView(void)
{
  sgMat4 mat;
  grMakeLookAtMat4(mat, eye, center, up);
  
  grContext.setCamera(mat);
}

void cGrPerspCamera::loadDefaults(char *attr)
{
    sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
    fovy = (float)GfParmGetNum(grHandle, path,
			       attr, (char*)NULL, fovydflt);
    limitFov();
}


/* Give the height in pixels of 1 m high object on the screen at this point */
float cGrPerspCamera::getLODFactor(float x, float y, float z) {
    tdble	dx, dy, dz, dd;
    float	ang;
    int		scrh, dummy;
    float	res;

    dx = x - eye[0];
    dy = y - eye[1];
    dz = z - eye[2];

    dd = sqrt(dx*dx+dy*dy+dz*dz);

    ang = DEG2RAD(fovy / 2.0);
    GfScrGetSize(&dummy, &scrh, &dummy, &dummy);
    
    res = (float)scrh / 2.0 / dd / tan(ang);
    if (res < 0) {
	res = 0;
    }
    return res;
}

void cGrPerspCamera::setZoom(int cmd)
{
    char	buf[256];

    switch(cmd) {
    case GR_ZOOM_IN:
	if (fovy > 2) {
	    fovy--;
	} else {
	    fovy /= 2.0;
	}
	if (fovy < fovymin) {
	    fovy = fovymin;
	}
	break;
	
    case GR_ZOOM_OUT:
	fovy++;
	if (fovy > fovymax) {
	    fovy = fovymax;
	}
	break;

    case GR_ZOOM_MAX:
	fovy = fovymax;
	break;

    case GR_ZOOM_MIN:
	fovy = fovymin;
	break;

    case GR_ZOOM_DFLT:
	fovy = fovydflt;
	break;
    }

    limitFov();

    sprintf(buf, "%s-%d-%d", GR_ATT_FOVY, screen->getCurCamHead(), getId());
    sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
    GfParmSetNum(grHandle, path, buf, (char*)NULL, (tdble)fovy);
    GfParmWriteFile(NULL, grHandle, "Graph");
}

void cGrOrthoCamera::setProjection(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(left, right, bottom, top);
}

void cGrOrthoCamera::setModelView(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void cGrBackgroundCam::update(cGrCamera *curCam)
{
    static float BACKGROUND_FOVY_CUTOFF = 60.0f;
    memcpy(&eye, curCam->getPosv(), sizeof(eye));
    memcpy(&center, curCam->getCenterv(), sizeof(center));
    sgSubVec3(center, center, eye);
    sgSetVec3(eye, 0, 0, 0);
    speed[0]=0.0;
    speed[1]=0.0;
    speed[2]=0.0;
    fovy = curCam->getFovY();
    if (fovy < BACKGROUND_FOVY_CUTOFF) {
        fovy = BACKGROUND_FOVY_CUTOFF;
    }
    memcpy(&up, curCam->getUpv(), sizeof(up));
}



class cGrCarCamInside : public cGrPerspCamera
{
 public:
    cGrCarCamInside(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float myfovy, float myfovymin, float myfovymax,
		    float myfnear, float myffar = 1500.0,
		    float myfogstart = 800.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 0, drawBG, 1,
			 myfovy, myfovymin, myfovymax,
			 myfnear, myffar, myfogstart, myfogend) {
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	sgVec3 P, p;
	
	p[0] = car->_drvPos_x;
	p[1] = car->_drvPos_y;
	p[2] = car->_drvPos_z;
	sgXformPnt3(p, car->_posMat);
	
	eye[0] = p[0];
	eye[1] = p[1];
	eye[2] = p[2];

	P[0] = car->_drvPos_x + 30.0;
	P[1] = car->_drvPos_y;
	P[2] = car->_drvPos_z;
	sgXformPnt3(P, car->_posMat);

	center[0] = P[0];
	center[1] = P[1];
	center[2] = P[2];

	speed[0] =car->pub.DynGCg.vel.x;  
	speed[1] =car->pub.DynGCg.vel.y;  
	speed[2] =car->pub.DynGCg.vel.z;
    }
};


/* MIRROR */
cGrCarCamMirror::~cGrCarCamMirror ()
{
    glDeleteTextures (1, &tex);
    delete viewCam;
}


void cGrCarCamMirror::limitFov(void) {
    fovy = 90.0 / screen->getViewRatio();
}


void cGrCarCamMirror::update(tCarElt *car, tSituation * /* s */)
{
    sgVec3 P, p;

    P[0] = car->_bonnetPos_x;
    P[1] = car->_bonnetPos_y;
    P[2] = car->_bonnetPos_z;
    sgXformPnt3(P, car->_posMat);
	
    eye[0] = P[0];
    eye[1] = P[1];
    eye[2] = P[2];
	
    p[0] = car->_bonnetPos_x - 30.0;
    p[1] = car->_bonnetPos_y;
    p[2] = car->_bonnetPos_z;
    sgXformPnt3(p, car->_posMat);

    center[0] = p[0];
    center[1] = p[1];
    center[2] = p[2];

    up[0] = car->_posMat[2][0];
    up[1] = car->_posMat[2][1];
    up[2] = car->_posMat[2][2];
}


void cGrCarCamMirror::setViewport(int x, int y, int w, int h)
{
	vpx = x;
	vpy = y;
	vpw = w;
	vph = h;

	if (viewCam) {
		delete viewCam;
	}
	viewCam = new cGrOrthoCamera(screen, x,  x + w, y, y + h);
	limitFov();
}


void cGrCarCamMirror::setPos (int x, int y, int w, int h)
{
    mx = x;
    my = y;
    mw = w;
    mh = h;

    // round up texture size to next power of two
    tw = GfNearestPow2(w);
    th = GfNearestPow2(h);
    if (tw < w) {
		tw *= 2;
	}
    if (th < h) {
		th *= 2;
	}

    // Create texture object.
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glReadBuffer(GL_BACK);

    glCopyTexImage2D(GL_TEXTURE_2D,
		      0, // map level,
		      GL_RGB, // internal format,
		      0, 0, tw, th,
		      0 ); // border

    tsu = (float) mw / tw;
    teu = 0.0;
    tsv = 0.0;
    tev = (float) mh / th;
}


void cGrCarCamMirror::activateViewport (void)
{
    glViewport(vpx, vpy, vpw, vph);

    // Enable scissor test to conserve graphics memory bandwidth.
    glEnable(GL_SCISSOR_TEST);
    glScissor(vpx + (vpw - mw)/2, vpy + (vph - mh)/2, mw, mh);
}

void cGrCarCamMirror::store (void)
{
	glDisable(GL_SCISSOR_TEST);

	glBindTexture(GL_TEXTURE_2D, tex);
	glReadBuffer(GL_BACK);

	// NVidia recommends to NOT use glCopyTexImage2D for performance reasons.
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
			    vpx + (vpw - mw)/2,
			    vpy + (vph - mh)/2, mw, mh);
}


void cGrCarCamMirror::display (void)
{
    viewCam->action ();

    glBindTexture (GL_TEXTURE_2D, tex);
    glBegin(GL_TRIANGLE_STRIP);
    {
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glTexCoord2f(tsu, tsv); glVertex2f(mx, my);
	glTexCoord2f(tsu, tev); glVertex2f(mx, my + mh);
	glTexCoord2f(teu, tsv); glVertex2f(mx + mw, my);
	glTexCoord2f(teu, tev); glVertex2f(mx + mw, my + mh);
    }
    glEnd();
}




class cGrCarCamInsideFixedCar : public cGrPerspCamera
{
 public:
    cGrCarCamInsideFixedCar(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
			    float myfovy, float myfovymin, float myfovymax,
			    float myfnear, float myffar = 1500.0,
			    float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 1,
			 myfovy, myfovymin, myfovymax,
			 myfnear, myffar, myfogstart, myfogend) {
    }

    void update(tCarElt *car, tSituation *s) {
	sgVec3 P, p;
	
	p[0] = car->_bonnetPos_x;
	p[1] = car->_bonnetPos_y;
	p[2] = car->_bonnetPos_z;
	sgXformPnt3(p, car->_posMat);
	
	eye[0] = p[0];
	eye[1] = p[1];
	eye[2] = p[2];

	P[0] = car->_bonnetPos_x + 30.0;
	P[1] = car->_bonnetPos_y;
	P[2] = car->_bonnetPos_z;
	sgXformPnt3(P, car->_posMat);

	center[0] = P[0];
	center[1] = P[1];
	center[2] = P[2];

	up[0] = car->_posMat[2][0];
	up[1] = car->_posMat[2][1];
	up[2] = car->_posMat[2][2];

	speed[0] =car->pub.DynGCg.vel.x;
	speed[1] =car->pub.DynGCg.vel.y;
	speed[2] =car->pub.DynGCg.vel.z;
    }
};


class cGrCarCamBehind : public cGrPerspCamera
{
    tdble PreA;

 protected:
    float dist;
    float height;
    
 public:
    cGrCarCamBehind(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float fovy, float fovymin, float fovymax,
		    float mydist, float myHeight, float fnear, float ffar = 1500.0,
		    float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	dist = mydist;
	height = myHeight;
	PreA = 0.0;
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	tdble A;
	tdble CosA;
	tdble SinA;
	tdble x;
	tdble y;

	A = car->_yaw;
	if (fabs(PreA - A) > fabs(PreA - A + 2*PI)) {
	    PreA += 2*PI;
	} else if (fabs(PreA - A) > fabs(PreA - A - 2*PI)) {
	    PreA -= 2*PI;
	}
	RELAXATION(A, PreA, 10.0);
	CosA = cos(A);
	SinA = sin(A);
	x = car->_pos_X - dist * CosA;
	y = car->_pos_Y - dist * SinA;
    
	eye[0] = x;
	eye[1] = y;
	eye[2] = RtTrackHeightG(car->_trkPos.seg, x, y) + height;
	center[0] = car->_pos_X + (10 - dist) * CosA;
	center[1] = car->_pos_Y + (10 - dist) * SinA;
	center[2] = car->_pos_Z;

	speed[0] = car->pub.DynGCg.vel.x;
	speed[1] = car->pub.DynGCg.vel.y;
	speed[2] = car->pub.DynGCg.vel.z;

    }
};


class cGrCarCamBehind2 : public cGrPerspCamera
{
    tdble PreA;

 protected:
    float dist;
    
 public:
    cGrCarCamBehind2(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float fovy, float fovymin, float fovymax,
		    float mydist, float fnear, float ffar = 1500.0,
		    float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	dist = mydist;
	PreA = 0.0;
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	tdble A;
	tdble CosA;
	tdble SinA;
	tdble x;
	tdble y;

	A = RtTrackSideTgAngleL(&(car->_trkPos));
	if (fabs(PreA - A) > fabs(PreA - A + 2*PI)) {
	    PreA += 2*PI;
	} else if (fabs(PreA - A) > fabs(PreA - A - 2*PI)) {
	    PreA -= 2*PI;
	}
	RELAXATION(A, PreA, 5.0);
	CosA = cos(A);
	SinA = sin(A);
	x = car->_pos_X - dist * CosA;
	y = car->_pos_Y - dist * SinA;
    
	eye[0] = x;
	eye[1] = y;
	eye[2] = RtTrackHeightG(car->_trkPos.seg, x, y) + 5.0;
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	center[2] = car->_pos_Z;


	speed[0] = car->pub.DynGCg.vel.x;
	speed[1] = car->pub.DynGCg.vel.y;
	speed[2] = car->pub.DynGCg.vel.z;

    }
};


class cGrCarCamFront : public cGrPerspCamera
{
 protected:
    float dist;
    
 public:
    cGrCarCamFront(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		   float fovy, float fovymin, float fovymax,
		   float mydist, float fnear, float ffar = 1500.0,
		   float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	dist = mydist;
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	tdble CosA = cos(car->_yaw);
	tdble SinA = sin(car->_yaw);
	tdble x = car->_pos_X + dist * CosA;
	tdble y = car->_pos_Y + dist * SinA;

	eye[0] = x;
	eye[1] = y;
	eye[2] = RtTrackHeightG(car->_trkPos.seg, x, y) + 0.5;
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	center[2] = car->_pos_Z;

	speed[0] = car->pub.DynGCg.vel.x;
	speed[1] = car->pub.DynGCg.vel.y;
	speed[2] = car->pub.DynGCg.vel.z;

    }
};


class cGrCarCamSide : public cGrPerspCamera
{
protected:
    float distx;
    float disty;
    float distz;
    
 public:
    cGrCarCamSide(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		  float fovy, float fovymin, float fovymax,
		  float mydistx, float mydisty, float mydistz,
		  float fnear, float ffar = 1500.0,
		  float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	distx = mydistx;
	disty = mydisty;
	distz = mydistz;

	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	tdble x = car->_pos_X + distx;
	tdble y = car->_pos_Y + disty;
	tdble z = car->_pos_Z + distz;
    
	eye[0] = x;
	eye[1] = y;
	eye[2] = z;
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	center[2] = car->_pos_Z;

	speed[0] = car->pub.DynGCg.vel.x;
	speed[1] = car->pub.DynGCg.vel.y;
	speed[2] = car->pub.DynGCg.vel.z;

    }
};

class cGrCarCamUp : public cGrPerspCamera
{
 protected:
    float distz;
    
 public:
    cGrCarCamUp(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		float fovy, float fovymin, float fovymax,
		float mydistz, int axis,
		float fnear, float ffar = 1500.0,
		float myfogstart = 1600.0, float myfogend = 1700.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	distz = mydistz;
	up[2] = 0;
	switch (axis) {
	case 0:
	    up[0] = 0;
	    up[1] = 1;
	    break;
	case 1:
	    up[0] = 0;
	    up[1] = -1;
	    break;
	case 2:
	    up[0] = 1;
	    up[1] = 0;
	    break;
	case 3:
	    up[0] = -1;
	    up[1] = 0;
	    break;
	default:
	    up[0] = 0;
	    up[1] = 1;
	    break;
	}
    }

    void update(tCarElt *car, tSituation *s) {
	tdble x = car->_pos_X;
	tdble y = car->_pos_Y;
	tdble z = car->_pos_Z + distz;
    
	eye[0] = x;
	eye[1] = y;
	eye[2] = z;
	center[0] = x;
	center[1] = y;
	center[2] = car->_pos_Z;


	speed[0] = car->pub.DynGCg.vel.x;
	speed[1] = car->pub.DynGCg.vel.y;
	speed[2] = car->pub.DynGCg.vel.z;

    }
};

class cGrCarCamCenter : public cGrPerspCamera
{
 protected:
    float distz;
    float locfar;
    float locfovy;
    
 public:
    cGrCarCamCenter(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float fovy, float fovymin, float fovymax,
		    float mydistz,
		    float fnear, float ffar = 1500.0,
		    float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	distz = mydistz;
	locfar = ffar;
	locfovy = fovy;

	eye[0] = grWrldX * 0.5;
	eye[1] = grWrldY * 0.6;
	eye[2] = distz;

	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void loadDefaults(char *attr) {
	sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
	locfovy = (float)GfParmGetNum(grHandle, path,
				   attr, (char*)NULL, fovydflt);
    }

    void setZoom(int cmd) {
	fovy = locfovy;
	cGrPerspCamera::setZoom(cmd);
	locfovy = fovy;
    }
    
    void update(tCarElt *car, tSituation *s) {
	tdble	dx, dy, dz, dd;
	
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	center[2] = car->_pos_Z;

	dx = center[0] - eye[0];
	dy = center[1] - eye[1];
	dz = center[2] - eye[2];
    
	dd = sqrt(dx*dx+dy*dy+dz*dz);
    
	fnear = dz - 5;
	if (fnear < 1) {
	    fnear = 1;
	}
	ffar  = dd + locfar;

	fovy = RAD2DEG(atan2(locfovy, dd));

	speed[0] = 0;
	speed[1] = 0;
	speed[2] = 0;

    }
};

class cGrCarCamLookAt : public cGrPerspCamera
{
 protected:
    
 public:
    cGrCarCamLookAt(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		    float fovy, float fovymin, float fovymax,
		    int axis,
		    float eyex, float eyey, float eyez,
		    float centerx, float centery, float centerz,
		    float fnear, float ffar = 1500.0,
		    float myfogstart = 1600.0, float myfogend = 1700.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {

	eye[0] = eyex;
	eye[1] = eyey;
	eye[2] = eyez;

	center[0] = centerx;
	center[1] = centery;
	center[2] = centerz;
	
	switch (axis) {
	case 0:
	    up[0] = 0;
	    up[1] = 1;
	    up[2] = 0;
	    break;
	case 1:
	    up[0] = 0;
	    up[1] = -1;
	    up[2] = 0;
	    break;
	case 2:
	    up[0] = 1;
	    up[1] = 0;
	    up[2] = 0;
	    break;
	case 3:
	    up[0] = -1;
	    up[1] = 0;
	    up[2] = 0;
	    break;
	case 4:
	    up[0] = 0;
	    up[1] = 0;
	    up[2] = 1;
	    break;
	case 5:
	    up[0] = 0;
	    up[1] = 0;
	    up[2] = -1;
	    break;
	default:
	    up[0] = 0;
	    up[1] = 0;
	    up[2] = 1;
	    break;
	}
    }

    void update(tCarElt *car, tSituation *s) {
    }
};


class cGrCarCamRoadNoZoom : public cGrPerspCamera
{
 protected:
    
 public:
    cGrCarCamRoadNoZoom(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
			float fovy, float fovymin, float fovymax,
			float fnear, float ffar = 1500.0,
			float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void update(tCarElt *car, tSituation *s) {
	tRoadCam *curCam;


	curCam = car->_trkPos.seg->cam;
    
	if (curCam == NULL) {
	    eye[0] = grWrldX * 0.5;
	    eye[1] = grWrldY * 0.6;
	    eye[2] = 120;
	    center[2] = car->_pos_Z;
	} else {
	    eye[0] = curCam->pos.x;
	    eye[1] = curCam->pos.y;
	    eye[2] = curCam->pos.z;
	    center[2] = curCam->pos.z;
	}
	
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	/* center[2] = car->_pos_Z; */

	speed[0] = 0.0;
	speed[1] = 0.0;
	speed[2] = 0.0;
    }
};

class cGrCarCamRoadFly : public cGrPerspCamera
{
 protected:
    int current;
    int timer;
    float zOffset;
    float gain;
    float damp;
    float offset[3];
    double currenttime;
 public:
    cGrCarCamRoadFly(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		     float fovy, float fovymin, float fovymax,
		     float fnear, float ffar = 1500.0,
		     float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
	timer = 0;
	offset[0]=0.0;
	offset[1]=0.0;
	offset[2]=60.0;
	current = -1;
	currenttime = 0.0;
	speed[0] = 0.0;
	speed[1] = 0.0;
	speed[2] = 0.0;
    }
    
    void update(tCarElt *car, tSituation *s) {
	tRoadCam *curCam;
	float height;
	float dt;

	curCam = car->_trkPos.seg->cam;

	if (currenttime == 0.0) {
	    currenttime = s->currentTime;
	}
	
	if (currenttime == s->currentTime) {
            return;
        }

        bool reset_camera = false;
        dt = s->currentTime - currenttime;
        currenttime = s->currentTime;
        if (fabs(dt) > 1.0f) { 
            dt = 0.1f; // avoid overflow
            reset_camera = true;
        }

        timer--;
        if (timer<0) {
            reset_camera = true;
        }

        if (current != car->index) {
            /* the target car changed */
            zOffset = 50.0;
            current = car->index;
            reset_camera = true;
        } else {
            zOffset = 0.0;
        }

        if ((timer <= 0) || (zOffset > 0.0)) {
            timer = 500 + (int)(500.0*rand()/(RAND_MAX+1.0));
            offset[0] = -0.5 + (rand()/(RAND_MAX+1.0));
            offset[1] = -0.5 + (rand()/(RAND_MAX+1.0));
            offset[2] = 10.0f + (50.0*rand()/(RAND_MAX+1.0)) + zOffset;
            offset[0] = offset[0]*(offset[2]+1.0);
            offset[1] = offset[1]*(offset[2]+1.0);
            // follow the car more closely when low
            gain = 300.0/(10.0f+offset[2]); 
            damp = 5.0f;
        }


        if (reset_camera) {
            eye[0] = car->_pos_X + 50.0 + (50.0*rand()/(RAND_MAX+1.0));
            eye[1] = car->_pos_Y + 50.0 + (50.0*rand()/(RAND_MAX+1.0));
            eye[2] = car->_pos_Z + 50.0 + (50.0*rand()/(RAND_MAX+1.0));
            speed[0] = speed[1] = speed[2] = 0.0f;
        }
	
        speed[0] += (gain*(offset[0]+car->_pos_X - eye[0]) - speed[0]*damp)*dt;
        speed[1] += (gain*(offset[1]+car->_pos_Y - eye[1]) - speed[1]*damp)*dt;
        speed[2] += (gain*(offset[2]+car->_pos_Z - eye[2]) - speed[2]*damp)*dt;

        eye[0] = eye[0] + speed[0]*dt;
        eye[1] = eye[1] + speed[1]*dt;
        eye[2] = eye[2] + speed[2]*dt;

        center[0] = (car->_pos_X);
        center[1] = (car->_pos_Y);
        center[2] = (car->_pos_Z);

        // avoid going under the scene
        height = grGetHOT(eye[0], eye[1]) + 1.0;
        if (eye[2] < height) {
            timer = 500 + (int)(500.0*rand()/(RAND_MAX+1.0));
            offset[2] = height - car->_pos_Z + 1.0;
            eye[2] = height;
        }
	
    }



    void onSelect(tCarElt *car, tSituation *s)
    {
        printf ("%f select\n", s->currentTime);
	timer = 0;
	current = -1;
    }

};

class cGrCarCamRoadZoom : public cGrPerspCamera
{
 protected:
    float locfar;
    float locfovy;
    
 public:
    cGrCarCamRoadZoom(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
		      float fovy, float fovymin, float fovymax,
		      float fnear, float ffar = 1500.0,
		      float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrPerspCamera(myscreen, id, drawCurr, 1, drawBG, 0, fovy, fovymin,
			 fovymax, fnear, ffar, myfogstart, myfogend) {
	locfar = ffar;
	locfovy = fovy;

	up[0] = 0;
	up[1] = 0;
	up[2] = 1;
    }

    void loadDefaults(char *attr) {
	sprintf(path, "%s/%d", GR_SCT_DISPMODE, screen->getId());
	locfovy = (float)GfParmGetNum(grHandle, path,
				   attr, (char*)NULL, fovydflt);
    }

    void setZoom(int cmd) {
	fovy = locfovy;
	cGrPerspCamera::setZoom(cmd);
	locfovy = fovy;
    }
    
    void update(tCarElt *car, tSituation *s) {
	tdble	dx, dy, dz, dd;
	tRoadCam *curCam;

	curCam = car->_trkPos.seg->cam;
    
	if (curCam == NULL) {
	    eye[0] = grWrldX * 0.5;
	    eye[1] = grWrldY * 0.6;
	    eye[2] = 120;
	} else {
	    eye[0] = curCam->pos.x;
	    eye[1] = curCam->pos.y;
	    eye[2] = curCam->pos.z;
	}
	
	center[0] = car->_pos_X;
	center[1] = car->_pos_Y;
	center[2] = car->_pos_Z;

	dx = center[0] - eye[0];
	dy = center[1] - eye[1];
	dz = center[2] - eye[2];
    
	dd = sqrt(dx*dx+dy*dy+dz*dz);

	fnear = dz - 5;
	if (fnear < 1) {
	    fnear = 1;
	}
	ffar  = dd + locfar;
	fovy = RAD2DEG(atan2(locfovy, dd));
	limitFov();

	speed[0] = 0.0;
	speed[1] = 0.0;
	speed[2] = 0.0;
    }
};

static tdble
GetDistToStart(tCarElt *car)
{
    tTrackSeg	*seg;
    tdble	lg;
    
    seg = car->_trkPos.seg;
    lg = seg->lgfromstart;
    
    switch (seg->type) {
    case TR_STR:
	lg += car->_trkPos.toStart;
	break;
    default:
	lg += car->_trkPos.toStart * seg->radius;
	break;
    }
    return lg;
}

typedef struct 
{
    double	prio;
    int		viewable;
    int		event;
} tSchedView;

class cGrCarCamRoadZoomTVD : public cGrCarCamRoadZoom
{
    tSchedView *schedView;
    double camChangeInterval;
    double camEventInterval;
    double lastEventTime;
    double lastViewTime;
    tdble  proximityThld;
    int		current;

 public:
    cGrCarCamRoadZoomTVD(class cGrScreen *myscreen, int id, int drawCurr, int drawBG,
			 float fovy, float fovymin, float fovymax,
			 float fnear, float ffar = 1500.0,
			 float myfogstart = 1400.0, float myfogend = 1500.0)
	: cGrCarCamRoadZoom(myscreen, id, drawCurr, drawBG, fovy, fovymin,
			    fovymax, fnear, ffar, myfogstart, myfogend) {
	schedView = (tSchedView *)calloc(grNbCars, sizeof(tSchedView));
	if (!schedView) {
	    GfTrace("malloc error");
	    GfScrShutdown();
	    exit (1);
	}
    
	lastEventTime = 0;
	lastViewTime = 0;

	current = -1;

	camChangeInterval = GfParmGetNum(grHandle, GR_SCT_TVDIR, GR_ATT_CHGCAMINT, (char*)NULL, 10.0);
	camEventInterval  = GfParmGetNum(grHandle, GR_SCT_TVDIR, GR_ATT_EVTINT, (char*)NULL, 1.0);
	proximityThld     = GfParmGetNum(grHandle, GR_SCT_TVDIR, GR_ATT_PROXTHLD, (char*)NULL, 10.0);
    }

    ~cGrCarCamRoadZoomTVD() { free(schedView); }

    void update(tCarElt *car, tSituation *s) {
	int	i, j;
	int	curCar;
	double	curPrio;
	double	deltaEventTime = s->currentTime - lastEventTime;
	double	deltaViewTime = s->currentTime - lastViewTime;
	int	event = 0;

	if (current == -1) {
	    current = 0;
	    for (i = 0; i < grNbCars; i++) {
		if (car == s->cars[i]) {
		    current = i;
		    break;
		}
	    }
	}
	
    
	/* Track events */
	if (deltaEventTime > camEventInterval) {

	    memset(schedView, 0, grNbCars * sizeof(tSchedView));
	    for (i = 0; i < grNbCars; i++) {
		schedView[i].viewable = 1;
	    }
	    
	    for (i = 0; i < GR_NB_MAX_SCREEN; i++) {
		if ((screen != grScreens[i]) && grScreens[i]->isActive()) {
		    car = grScreens[i]->getCurrentCar();
		    schedView[car->index].viewable = 0;
		    schedView[car->index].prio -= 10000;
		}
	    }
	    
	    for (i = 0; i < grNbCars; i++) {
		tdble dist, fs;

		car = s->cars[i];
		schedView[car->index].prio += grNbCars - i;
		fs = GetDistToStart(car);
		if ((car->_state & RM_CAR_STATE_NO_SIMU) != 0) {
		    schedView[car->index].viewable = 0;
		} else {
		    if ((fs > (grTrack->length - 200.0)) && (car->_remainingLaps == 0)) {
			schedView[car->index].prio += 5 * grNbCars;
			event = 1;
		    }
		}
	
		if ((car->_state & RM_CAR_STATE_NO_SIMU) == 0) {
		    dist = fabs(car->_trkPos.toMiddle) - grTrack->width / 2.0;
		    /* out of track */
		    if (dist > 0) {
			schedView[car->index].prio += grNbCars;
			if (car->ctrl.raceCmd & RM_CMD_PIT_ASKED) {
			    schedView[car->index].prio += grNbCars;
			    event = 1;
			}
		    }

		    for (j = i+1; j < grNbCars; j++) {
			tCarElt *car2 = s->cars[j];
			tdble fs2 = GetDistToStart(car2);
			tdble d = fabs(fs2 - fs);

			if ((car2->_state & RM_CAR_STATE_NO_SIMU) == 0) {
			    if (d < proximityThld) {
				d = proximityThld - d;
				schedView[car->index].prio  += d * grNbCars / proximityThld;
				schedView[car2->index].prio += d * (grNbCars - 1) / proximityThld;
				if (i == 0) {
				    event = 1;
				}
			    }
			}
		    }

		    if (car->priv.collision) {
			schedView[car->index].prio += grNbCars;
			event = 1;
		    }
		} else {
		    if (i == current) {
			event = 1;	/* update view */
		    }
		}
	    }


	    /* change current car */
	    if ((event && (deltaEventTime > camEventInterval)) || (deltaViewTime > camChangeInterval)) {
		int	last_current = current;

		curCar = 0;
		curPrio = -1000000.0;
		for (i = 0; i < grNbCars; i++) {
	    
		    if ((schedView[i].prio > curPrio) && (schedView[i].viewable)) {
			curPrio = schedView[i].prio;
			curCar = i;
		    }
		}
		for (i = 0; i < grNbCars; i++) {
		    if (s->cars[i]->index == curCar) {
			current = i;
			break;
		    }
		}
		if (last_current != current) {
		    lastEventTime = s->currentTime;
		    lastViewTime = s->currentTime;

		    for (i = 0; i < grNbCars; i++) {
			s->cars[i]->priv.collision = 0;
		    }
		}
	    }
	}

	screen->setCurrentCar(s->cars[current]);
	
	cGrCarCamRoadZoom::update(s->cars[current], s);
    }
};


void
grCamCreateSceneCameraList(class cGrScreen *myscreen, tGrCamHead *cams, tdble fovFactor)
{
    int			id;
    int			c;
    class cGrCamera	*cam;
    
    /* Scene Cameras */
    c = 0;

    /* F2 */
    GF_TAILQ_INIT(&cams[c]);
    id = 0;
    
    /* cam F2 = behind very near */
    cam = new cGrCarCamBehind(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      40.0,	/* fovy */
			      5.0,	/* fovymin */
			      95.0,	/* fovymax */
			      6.0,	/* dist */
			      2.0,	/* height */
			      1.0,	/* near */
			      600.0 * fovFactor,	/* far */
			      300.0 * fovFactor,	/* fog */
			      600.0 * fovFactor	/* fog */
			      );
    cam->add(&cams[c]);
    id++;
    
    /* cam F2 = behind near */
    cam = new cGrCarCamBehind(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      40.0,	/* fovy */
			      5.0,	/* fovymin */
			      95.0,	/* fovymax */
			      10.0,	/* dist */
			      2.0,	/* height */
			      1.0,	/* near */
			      600.0 * fovFactor,	/* far */
			      300.0 * fovFactor,	/* fog */
			      600.0 * fovFactor	/* fog */
			      );
    cam->add(&cams[c]);
    id++;
    
    /* cam F2 = car inside with car (bonnet view) fixed to the car */
    cam = new cGrCarCamInsideFixedCar(myscreen,
				      id,
				      1,	/* drawCurr */
				      1,	/* drawBG  */
				      67.5,	/* fovy */
				      50.0,	/* fovymin */
				      95.0,	/* fovymax */
				      0.3,	/* near */
				      600.0 * fovFactor,	/* far */
				      300.0 * fovFactor,	/* fog */
				      600.0 * fovFactor	/* fog */
				      );
    cam->add(&cams[c]);
    id++;
    
    /* cam F2 = car inside with car (bonnet view) */
    cam = new cGrCarCamInside(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      67.5,	/* fovy */
			      50.0,	/* fovymin */
			      95.0,	/* fovymax */
			      0.1,	/* near */
			      600.0 * fovFactor,	/* far */
			      300.0 * fovFactor,	/* fog */
			      600.0 * fovFactor	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F2 = car inside car (no car - road view) */
    cam = new cGrCarCamInsideFixedCar(myscreen,
				      id,
				      0,	/* drawCurr */
				      1,	/* drawBG  */
				      67.5,	/* fovy */
				      50.0,	/* fovymin */
				      95.0,	/* fovymax */
				      0.3,	/* near */
				      600.0 * fovFactor,	/* far */
				      300.0 * fovFactor,	/* fog */
				      600.0 * fovFactor	/* fog */
				      );
    cam->add(&cams[c]);

    /* F3 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;
    
    /* cam F3 = behind far */
    cam = new cGrCarCamBehind(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      40.0,	/* fovy */
			      5.0,	/* fovymin */
			      95.0,	/* fovymax */
			      20.0,	/* dist */
			      2.0,	/* height */
			      1.0,	/* near */
			      600.0 * fovFactor,	/* far */
			      300.0 * fovFactor,	/* fog */
			      600.0 * fovFactor	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F3 = car behind*/
    cam = new cGrCarCamBehind2(myscreen,
			       id,
			       1,	/* drawCurr */
			       1,	/* drawBG  */
			       40.0,	/* fovy */
			       5.0,	/* fovymin */
			       95.0,	/* fovymax */
			       30.0,	/* dist */
			       1.0,	/* near */
			       1000.0 * fovFactor,	/* far */
			       500.0 * fovFactor,	/* fog */
			       1000.0 * fovFactor	/* fog */
			       );
    cam->add(&cams[c]);
    id++;

    /* cam F3 = car behind*/
    cam = new cGrCarCamBehind(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      40.0,	/* fovy */
			      5.0,	/* fovymin */
			      95.0,	/* fovymax */
			      8.0,	/* dist */
			      .50,	/* height */
			      .50,	/* near */
			      600.0 * fovFactor,	/* far */
			      300.0 * fovFactor,	/* fog */
			      600.0 * fovFactor	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F3 = car reverse*/
    cam = new cGrCarCamFront(myscreen,
			     id,
			     1,	/* drawCurr */
			     1,	/* drawBG  */
			     40.0,	/* fovy */
			     5.0,	/* fovymin */
			     95.0,	/* fovymax */
			     8.0,	/* dist */
			     0.5,	/* near */
			     1000.0 * fovFactor,	/* far */
			     500.0 * fovFactor,	/* fog */
			     1000.0 * fovFactor	/* fog */
			     );
    cam->add(&cams[c]);

    /* F4 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F4 = car side 1*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    0.0,	/* distx */
			    -20.0,	/* disty */
			    3.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 2*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    0.0,	/* distx */
			    20.0,	/* disty */
			    3.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 3*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    -20.0,	/* distx */
			    0.0,	/* disty */
			    3.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 4*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    20.0,	/* distx */
			    0.0,	/* disty */
			    3.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 5*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    0.0,	/* distx */
			    -40.0,	/* disty */
			    6.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 6*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    0.0,	/* distx */
			    40.0,	/* disty */
			    6.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 7*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    -40.0,	/* distx */
			    0.0,	/* disty */
			    6.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);
    id++;

    /* cam F4 = car side 8*/
    cam = new cGrCarCamSide(myscreen,
			    id,
			    1,	/* drawCurr */
			    1,	/* drawBG  */
			    30.0,	/* fovy */
			    5.0,	/* fovymin */
			    60.0,	/* fovymax */
			    40.0,	/* distx */
			    0.0,	/* disty */
			    6.0,	/* distz */
			    1.0,	/* near */
			    1000.0 * fovFactor,	/* far */
			    500.0 * fovFactor,	/* fog */
			    1000.0 * fovFactor	/* fog */
			    );
    cam->add(&cams[c]);

    /* F5 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F5 = car up 1*/
    cam = new cGrCarCamUp(myscreen,
			  id,
			  1,	/* drawCurr */
			  1,	/* drawBG  */
			  //12.0,	/* fovy */
			  67.5,	/* fovy */
			  1.0,	/* fovymin */
			  90.0,	/* fovymax */
			  //300.0,	/* distz */
			  200.0,	/* distz */
			  0,		/* axis */
			  //200.0,	/* near */
			  100.0,	/* near */
			  1000.0 * fovFactor,/* far */
			  500.0 * fovFactor,	/* fog */
			  1000.0 * fovFactor	/* fog */
			  );
    cam->add(&cams[c]);
    id++;

    /* cam F5 = car up 2*/
    cam = new cGrCarCamUp(myscreen,
			  id,
			  1,	/* drawCurr */
			  1,	/* drawBG  */
			  //12.0,	/* fovy */
			  67.5,	/* fovy */
			  1.0,	/* fovymin */
			  90.0,	/* fovymax */
			  //300.0,	/* distz */
			  250.0,	/* distz */
			  1,		/* axis */
			  200.0,	/* near */
			  1000.0 * fovFactor,/* far */
			  500.0 * fovFactor,	/* fog */
			  1000.0 * fovFactor	/* fog */
			  );
    cam->add(&cams[c]);
    id++;

    /* cam F5 = car up 3*/
    cam = new cGrCarCamUp(myscreen,
			  id,
			  1,	/* drawCurr */
			  1,	/* drawBG  */
			  //12.0,	/* fovy */
			  67.5,	/* fovy */
			  1.0,	/* fovymin */
			  90.0,	/* fovymax */
			  //300.0,	/* distz */
			  350.0,	/* distz */
			  2,		/* axis */
			  200.0,	/* near */
			  1000.0 * fovFactor,/* far */
			  500.0 * fovFactor,	/* fog */
			  1000.0 * fovFactor	/* fog */
			  );
    cam->add(&cams[c]);
    id++;

    /* cam F5 = car up 4*/
    cam = new cGrCarCamUp(myscreen,
			  id,
			  1,	/* drawCurr */
			  1,	/* drawBG  */
			  //12.0,	/* fovy */
			  67.5,	/* fovy */
			  1.0,	/* fovymin */
			  90.0,	/* fovymax */
			  //300.0,	/* distz */
			  400.0,	/* distz */
			  3,		/* axis */
			  200.0,	/* near */
			  1000.0 * fovFactor,/* far */
			  500.0 * fovFactor,	/* fog */
			  1000.0 * fovFactor	/* fog */
			  );
    cam->add(&cams[c]);

    /* F6 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F6 = car from circuit centre */
    cam = new cGrCarCamCenter(myscreen,
			      id,
			      1,	/* drawCurr */
			      1,	/* drawBG  */
			      21.0,	/* fovy */
			      2.0,	/* fovymin */
			      60.0,	/* fovymax */
			      120.0,	/* distz */
			      100.0,	/* near */
			      1500.0,/* far */
			      10500.0,/* fog */
			      20500.0	/* fog */
			      );
    cam->add(&cams[c]);

    /* F7 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F7 = panoramic */
    cam = new cGrCarCamLookAt(myscreen,
			      id,
			      1,		/* drawCurr */
			      0,		/* drawBG  */
			      74.0,		/* fovy */
			      1.0,		/* fovymin */
			      110.0,		/* fovymax */
			      0,		/* up axis */
			      grWrldX/2,	/* eyex */
			      grWrldY/2,	/* eyey */
			      MAX(grWrldX/2, grWrldY*4/3/2) + grWrldZ, /* eyez */
			      grWrldX/2,	/* centerx */
			      grWrldY/2,	/* centery */
			      0,		/* centerz */
			      10.0,		/* near */
			      grWrldMaxSize * 2.0,	/* far */
			      grWrldMaxSize * 10.0,	/* fog */
			      grWrldMaxSize * 20.0	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F7 = panoramic */
    cam = new cGrCarCamLookAt(myscreen,
			      id,
			      1,		/* drawCurr */
			      0,		/* drawBG  */
			      74.0f,		/* fovy */
			      1.0f,		/* fovymin */
			      110.0f,		/* fovymax */
			      4,		/* up axis */
			      -grWrldX/2.0f,	/* eyex */
			      -grWrldY/2.0f,	/* eyey */
			      0.25f * sqrt((double)(grWrldX*grWrldX+grWrldY*grWrldY)), /* eyez */
			      grWrldX/2.0f,	/* centerx */
			      grWrldY/2.0f,	/* centery */
			      0.0f,		/* centerz */
			      10.0f,		/* near */
			      2.0f * grWrldMaxSize,	/* far */
			      10.0f * grWrldMaxSize,	/* fog */
			      20.0f * grWrldMaxSize	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F7 = panoramic */
    cam = new cGrCarCamLookAt(myscreen,
			      id,
			      1,		/* drawCurr */
			      0,		/* drawBG  */
			      74.0f,		/* fovy */
			      1.0f,		/* fovymin */
			      110.0f,		/* fovymax */
			      4,		/* up axis */
			      -grWrldX/2.0f,	/* eyex */
			      grWrldY * 3.0f/2.0f,	/* eyey */
			      0.25f * sqrt((double)(grWrldX*grWrldX+grWrldY*grWrldY)), /* eyez */
			      grWrldX/2.0f,	/* centerx */
			      grWrldY/2.0f,	/* centery */
			      0.0f,		/* centerz */
			      10.0f,		/* near */
			      2.0f * grWrldMaxSize,	/* far */
			      10.0f * grWrldMaxSize,	/* fog */
			      20.0f * grWrldMaxSize	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F7 = panoramic */
    cam = new cGrCarCamLookAt(myscreen,
			      id,
			      1,		/* drawCurr */
			      0,		/* drawBG  */
			      74.0f,		/* fovy */
			      1.0f,		/* fovymin */
			      110.0f,		/* fovymax */
			      4,		/* up axis */
			      grWrldX * 3.0f/2.0f,	/* eyex */
			      grWrldY * 3.0f/2.0f,	/* eyey */
			      0.25f * sqrt((double)(grWrldX*grWrldX+grWrldY*grWrldY)), /* eyez */
			      grWrldX/2.0f,	/* centerx */
			      grWrldY/2.0f,	/* centery */
			      0.0f,		/* centerz */
			      10.0f,		/* near */
			      2.0f * grWrldMaxSize,	/* far */
			      10.0f * grWrldMaxSize,	/* fog */
			      20.0f * grWrldMaxSize	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* cam F7 = panoramic */
    cam = new cGrCarCamLookAt(myscreen,
			      id,
			      1,		/* drawCurr */
			      0,		/* drawBG  */
			      74.0f,		/* fovy */
			      1.0f,		/* fovymin */
			      110.0f,		/* fovymax */
			      4,		/* up axis */
			      grWrldX * 3.0f/2.0f,	/* eyex */
			      -grWrldY/2.0f,	/* eyey */
			      0.25f * sqrt((double)(grWrldX*grWrldX+grWrldY*grWrldY)), /* eyez */
			      grWrldX/2.0f,	/* centerx */
			      grWrldY/2.0f,	/* centery */
			      0.0f,		/* centerz */
			      10.0f,		/* near */
			      2.0f * grWrldMaxSize,	/* far */
			      10.0f * grWrldMaxSize,	/* fog */
			      20.0f * grWrldMaxSize	/* fog */
			      );
    cam->add(&cams[c]);
    id++;

    /* F8 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F8 = road cam fixed fov */

    cam = new cGrCarCamRoadNoZoom(myscreen,
				  id,
				  1,		/* drawCurr */
				  1,		/* drawBG  */
				  30.0,	/* fovy */
				  5.0,	/* fovymin */
				  60.0,	/* fovymax */
				  1.0,	/* near */
				  1000.0 * fovFactor,/* far */
				  500.0 * fovFactor,	/* fog */
				  1000.0 * fovFactor	/* fog */
				  );
    cam->add(&cams[c]);

    /* F9 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    /* cam F9 = road cam zoomed */
    cam = new cGrCarCamRoadZoom(myscreen,
				id,
				1,		/* drawCurr */
				1,		/* drawBG  */
				9.0,	/* fovy */
				1.0,		/* fovymin */
				90.0,	/* fovymax */
				1.0,		/* near */
				1000.0 * fovFactor,	/* far */
				500.0 * fovFactor,	/* fog */
				1000.0 * fovFactor	/* fog */
				);
    cam->add(&cams[c]);

    /* F10 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;

    cam = new cGrCarCamRoadFly(myscreen,
			       id,
			       1,		/* drawCurr */
			       1,		/* drawBG  */
			       //17.0,	/* fovy */
			       67.5,	/* fovy */
			       1.0,		/* fovymin */
			       90.0,	/* fovymax */
			       1.0,		/* near */
			       1000.0 * fovFactor,	/* far */
			       500.0 * fovFactor,	/* fog */
			       1000.0 * fovFactor	/* fog */
			       );
    cam->add(&cams[c]);

    /* F11 */
    c++;
    GF_TAILQ_INIT(&cams[c]);
    id = 0;
    cam = new cGrCarCamRoadZoomTVD(myscreen,
				   id,
				   1,	/* drawCurr */
				   1,	/* drawBG  */
				   9.0,	/* fovy */
				   1.0,	/* fovymin */
				   90.0,	/* fovymax */
				   1.0,	/* near */
				   1000.0 * fovFactor,	/* far */
				   500.0 * fovFactor,	/* fog */
				   1000.0 * fovFactor	/* fog */
				   );
    cam->add(&cams[c]);

}
