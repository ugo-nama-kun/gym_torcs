/***************************************************************************

    file                 : robottools.h
    created              : Mon Feb 28 22:31:13 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: robottools.h,v 1.6 2003/11/23 20:21:11 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file   
    		Robots Tools
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: robottools.h,v 1.6 2003/11/23 20:21:11 torcs Exp $
*/

#ifndef _ROBOTTOOLS_H_
#define _ROBOTTOOLS_H_

#include <car.h>
#include <track.h>

#define RELAXATION2(target, prev, rate) 			\
do {								\
    tdble __tmp__;						\
    __tmp__ = target;						\
    target = (prev) + (rate) * ((target) - (prev)) * 0.01;	\
    prev = __tmp__;						\
} while (0)

#define RELAXATION(target, prev, rate) 				\
do {								\
    target = (prev) + (rate) * ((target) - (prev)) * 0.01;	\
    prev = (target);						\
} while (0)

/*
 * Track Utilities
 */

/* for variable width segments */
extern tdble RtTrackGetWidth(tTrackSeg *seg, tdble toStart);

/*
 * Convert a Local position (segment, toRight, toStart)
 * into a Global one (X, Y)
 *
 * The ToStart position refers to the current segment,
 * the function will not search for next segment if toStart
 * is greater than the segment length.
 * toStart represent an angle in radian for curves
 * and a length in meters for straights.
 *
 */
extern void RtTrackLocal2Global(tTrkLocPos *p, tdble *X, tdble *Y, int flag);

/*
 * Convert a Global (segment, X, Y) position into a Local one (segment, toRight, toStart)
 *
 * The segment in the Global position is used to start the search of a good segment
 * in term of toStart value.
 * The segments are scanned in order to find a toStart value between 0 and the length
 * of the segment for straights or the arc of the curve.
 *
 * The sides parameters is to indicate wether to use the track sides (1) or not (0) in
 * the toRight computation.
 */
extern void RtTrackGlobal2Local(tTrackSeg *segment, tdble X, tdble Y, tTrkLocPos *p, int type);


/*
 * Returns the absolute height in meters of the road
 * at the Local position p.
 * 
 * If the point lies outside the track (and sides)
 * the height is computed using the tangent to the banking
 * of the segment (or side).

                + Point given
               .^
              . |
             .  |
            .   |
           /    | heigth
          /     |
   ______/      v
   ^    ^^  ^
   |    ||  |
    track side

 */
extern tdble RtTrackHeightL(tTrkLocPos *p);


/*
 * Returns the absolute height in meters of the road
 * at the Global position (segment, X, Y)
 */
extern tdble RtTrackHeightG(tTrackSeg *seg, tdble X, tdble Y);


/*
 * Give the normal vector of the border of the track
 * including the sides.
 *
 * The side parameter is used to indicate the right (TR_RGT)
 * of the left (TR_LFT) side to consider.
 *
 * The Global position given (segment, X, Y) is used
 * to project the point on the border, it is not necessary
 * to give a point directly on the border itself.
 *
 * The vector is normalized.
 */
extern void RtTrackSideNormalG(tTrackSeg *seg, tdble X, tdble Y, int side, t3Dd *norm);


/*
 * Used to get the tangent angle for a track position
 * The angle is given in radian.
 *
 * the angle 0 is parallel to the first segment start.
 */
extern tdble RtTrackSideTgAngleL(tTrkLocPos *p);


/*
 * Used to get the normal vector of the road itself (pointing
 * upward).
 *
 * Local coordinates are used to locate the point where to
 * get the road normal vector.
 *
 * The vector is normalized.
 */
extern void RtTrackSurfaceNormalL(tTrkLocPos *p, t3Dd *norm);

extern int RtDistToPit(struct CarElt *car, tTrack *track, tdble *dL, tdble *dW);

extern tdble RtGetDistFromStart(tCarElt *car);
extern tdble RtGetDistFromStart2(tTrkLocPos *p);

/****************
 * Telemetry    *
 ****************/

/** Initialization of a telemetry session.
    @param	ymin	Minimum value for Y.
    @param	ymax	Maximum value for Y.
    @return	None
 */
extern void RtTelemInit(tdble ymin, tdble ymax);

/** Get the current segment
 */
tTrackSeg *RtTrackGetSeg(tTrkLocPos *p);



/** Create a new telemetry channel.
    @param	name	Name of the channel.
    @param	var	Address of the variable to monitor.
    @param	min	Minimum value of this variable.
    @param	max	Maximum value of this variable.
    @return	None
 */
extern void RtTelemNewChannel(const char * name, tdble * var, tdble min, tdble max);
extern void RtTelemStartMonitoring(const char * filename);
extern void RtTelemStopMonitoring(void);
extern void RtTelemUpdate(double time);
extern void RtTelemShutdown(void);

#endif /* _ROBOTTOOLS_H_ */ 



