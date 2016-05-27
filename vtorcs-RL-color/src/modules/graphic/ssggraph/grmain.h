/***************************************************************************

    file                 : grmain.h
    created              : Fri Aug 18 00:00:41 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grmain.h,v 1.18 2005/08/05 09:48:30 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

 
#ifndef _GRMAIN_H_
#define _GRMAIN_H_

#include <plib/ssg.h>
#include <raceman.h>

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>

////// Multitexturing Info
extern PFNGLMULTITEXCOORD2FARBPROC glMultiTexCoord2fARB ;
extern PFNGLMULTITEXCOORD2FVARBPROC glMultiTexCoord2fvARB;
extern PFNGLACTIVETEXTUREARBPROC   glActiveTextureARB ;
extern PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTextureARB ;

extern bool InStr(char *searchStr, char *str);
#endif

extern bool InitMultiTex();
extern int grWinx, grWiny, grWinw, grWinh;

extern int grVectFlag;
extern int grVectDispFlag[];
extern int segIndice;

extern double grCurTime;

extern void *grHandle;
extern void *grTrackHandle;

extern ssgContext grContext;
extern int grNbCars;

extern int  initView(int x, int y, int width, int height, int flag, void *screen);
extern int  initCars(tSituation *s);
extern int  refresh(tSituation *s);
extern void shutdownCars(void);
extern int  initTrack(tTrack *track);
extern void shutdownTrack(void);
//extern void bendCar (int index, sgVec3 poc, sgVec3 force, int cnt);
extern int maxTextureUnits;
extern tdble grMaxDammage;

extern class cGrScreen *grScreens[];

#define GR_SPLIT_ADD	0
#define GR_SPLIT_REM	1

#define GR_NB_MAX_SCREEN 4

extern tdble grLodFactorValue;
extern double grDeltaTime;

inline float urandom() {
	return (((float)rand()/(1.0+(float)RAND_MAX)));
}


#endif /* _GRMAIN_H_ */ 



