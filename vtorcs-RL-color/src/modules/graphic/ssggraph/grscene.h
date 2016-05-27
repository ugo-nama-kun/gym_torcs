/***************************************************************************

    file                 : grscene.h
    created              : Mon Aug 21 20:09:40 CEST 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grscene.h,v 1.12 2004/11/26 15:37:47 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _GRSCENE_H_
#define _GRSCENE_H_
#ifdef GUIONS
#include <glib.h> 
#endif /* GUIONS */
#include <track.h>


extern int grWrldX;
extern int grWrldY;
extern int grWrldZ;
extern int grWrldMaxSize;
extern tTrack *grTrack;

extern ssgRoot *TheScene;
extern ssgBranch *SunAnchor;
extern ssgBranch *LandAnchor;
extern ssgBranch *CarsAnchor;
extern ssgBranch *ShadowAnchor;
extern ssgBranch *PitsAnchor;
extern ssgBranch *SmokeAnchor;
extern ssgBranch *SkidAnchor;
extern ssgBranch *CarlightAnchor;

extern ssgBranch *ThePits;

extern int grInitScene(void);
extern int grLoadScene(tTrack *track);
extern void grDrawScene(void);
extern void grShutdownScene(void);
extern void grCustomizePits(void);

struct Camera;
extern void grDrawBackground(class cGrCamera *, class cGrBackgroundCam *bgCam);

extern ssgStateSelector	*grEnvSelector;
#include "grmultitexstate.h"
extern grMultiTexState	*grEnvState;
extern grMultiTexState	*grEnvShadowState;
extern grMultiTexState	*grEnvShadowStateOnCars;

#ifdef GUIONS
typedef struct DoV 
{
  tdble FrontLevelGroupGlobal; /* the distance for the end of the front scene */
  tdble FrontLevelGroup1;      /* the distance for the end of the front scene for group type 1*/
  tdble FrontLevelGroup2;      /* the distance for the end of the front scene for group type 2*/
  tdble FrontLevelGroup3;      /* the distance for the end of the front scene for group type 3*/

  tdble RearLevelGroupGlobal; /* the distance for the end of the front scene */
  tdble RearLevelGroup1;
  tdble RearLevelGroup2;
  tdble RearLevelGroup3;
  
  tdble FrontLevelMap1;      /* the distance for the end of the front scene with only one mapping*/
  tdble FrontLevelMap2;      /* the distance for the end of the front scene with two mapping*/
  tdble FrontLevelMap3;      /* the distance for the end of the front scene with three mapping*/
  tdble RearLevelMap1;
  tdble RearLevelMap2;
  tdble RearLevelMap3;
} DoV_t;

typedef struct hashMapElement 
{
  char * name;
  int    numberOfMapToApply;
} hashMapElement_t;

typedef  struct DistanceOfViewHashing
{
  char * name;                 /* segment name */
  GHashTable * ViewGroup;      /* all object to display group1+group2+group3 for this segment */
  int ViewGroup_num;           /* number of object */
  int ViewGroupMap1_num;
  int ViewGroupMap2_num;
  int ViewGroupMap3_num;
} DistanceOfViewHashing_t;

extern DistanceOfViewHashing_t * SceneHashing;
extern DoV_t * currentDistanceOfView;
extern DoV_t  PlayableDistanceOfView;
extern DoV_t  UnPlayableDistanceOfView;
#endif /* GUIONS */



#endif /* _GRSCENE_H_ */ 



