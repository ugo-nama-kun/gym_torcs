/***************************************************************************

    file                 : common.h
    created              : Sun Oct 24 14:19:43 CEST 1999
    copyright            : (C) 1999 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: common.h,v 1.3 2003/10/19 18:59:38 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 

#ifndef _COMMON_H_
#define _COMMON_H_

extern tdble Tright[];
extern tdble MaxSpeed[];
extern tdble hold[];
extern int   PitState[];
extern tdble DynOffset[];
extern tdble shiftThld[10][MAX_GEARS+1];
extern tTrack *DmTrack;
extern tdble O1[];
extern tdble O2[];
extern tdble O3[];
extern tdble O4[];
extern tdble O5[];
extern tdble OP[];
extern tdble OA[];
extern tdble OW[];
extern tdble VM;
extern tdble VM1[];
extern tdble VM2[];
extern tdble VM3[];

extern tdble ConsFactor[];
extern tdble OffsetApproach[];
extern tdble OffsetFinal[];
extern tdble OffsetExit[];
extern tdble LgfsFinal[];


#define PIT_STATE_NO            -1
#define PIT_STATE_NONE           0
#define PIT_STATE_ASKED          1
#define PIT_STATE_ENTERED        2
#define PIT_STATE_PITLANE_BEFORE 3
#define PIT_STATE_PIT_ENTRY      4
#define PIT_STATE_PIT_ALIGN      5
#define PIT_STATE_PIT_EXIT       6
#define PIT_STATE_PITLANE_AFTER  7
#define PIT_STATE_EXIT           8
#define PIT_STATE_DECEL          9

extern void InitGears(tCarElt* car, int idx);
extern void SpeedStrategy(tCarElt* car, int idx, tdble Vtarget, tSituation *s, tdble aspect);
extern tdble GetDistToStart(tCarElt *car);
extern void CollDet(tCarElt* car, int idx, tSituation *s, tdble Curtime, tdble dny);


#endif /* _COMMON_H_ */ 



