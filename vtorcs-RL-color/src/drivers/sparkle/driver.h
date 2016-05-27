/***************************************************************************

    file                 : driver.h
    created              : Thu Dec 20 01:20:19 CET 2002
    copyright            : (C) 2002 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: driver.h,v 1.2.2.1 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DRIVER_H_
#define _DRIVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "linalg.h"
#include "opponent.h"
#include "pit.h"


class Opponents;
class Opponent;
class Pit;

class Driver {
	public:
		Driver(int index);
		~Driver();

		/* callback functions called from TORCS */
		void initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s);
		void newRace(tCarElt* car, tSituation *s);
		void drive(tSituation *s);
		int pitCommand(tSituation *s);
		void endRace(tSituation *s);

		tCarElt *getCarPtr() { return car; }
		tTrack *getTrackPtr() { return track; }
		float getSpeed() { return speed; }

	private:
		/* utility functions */
		bool isStuck();
		void update(tSituation *s);
		float getAllowedSpeed(tTrackSeg *segment);
		float getAccel();
		float getDistToSegEnd();
		float getBrake();
		int getGear();
		float getSteer();
		v2d getTargetPoint();
		float getOvertakeOffset();
		float brakedist(float allowedspeed, float mu);

		float filterBColl(float brake);
		float filterABS(float brake);
		float filterBPit(float brake);
		float filterBrakeSpeed(float brake);

		float filterTCL(float accel);
		float filterTrk(float accel);

		float filterSColl(float steer);

		float filterTCL_RWD();
		float filterTCL_FWD();
		float filterTCL_4WD();
		void initTCLfilter();

		void initCa();
		void initCw();
		void initTireMu();

		/* per robot global data */
		int stuck;
		float trackangle;		/* the angle of the current track segment (global coordinates) */
		float angle;			/* the angle of the car relative to the current segment */
		float speed;			/* speed in track direction */
		float mass;				/* mass of car + fuel */
		float myoffset;			/* offset to the track middle */
		tCarElt *car;			/* pointer to tCarElt struct */
		Opponents *opponents;	/* the container for opponents */
		Opponent *opponent;		/* the array of opponents */
		Pit *pit;				/* pointer to the pit instance */
		float lastturnarc;		/* the arc of the current turn segments of the same type */
		int lastsegtype;		/* the segment type for which lastturnarc is valid */
		float currentspeedsqr;	/* square of the current speed_x */


		/* data that should stay constant after first initialization */
		int MAX_UNSTUCK_COUNT;
		int INDEX;
		float CARMASS;		/* mass of the car only */
		float CA;			/* aerodynamic downforce coefficient */
		float CW;			/* aerodynamic drag coefficient */
		float TIREMU;		/* friction coefficient of tires */
		float (Driver::*GET_DRIVEN_WHEEL_SPEED)();
		float OVERTAKE_OFFSET_INC;		/* [m/timestep] */
		float MU_FACTOR;				/* [-] */

		/* class constants */
		static const float MAX_UNSTUCK_ANGLE;
		static const float UNSTUCK_TIME_LIMIT;
		static const float MAX_UNSTUCK_SPEED;
		static const float MIN_UNSTUCK_DIST;
		static const float G;
		static const float FULL_ACCEL_MARGIN;
		static const float SHIFT;
		static const float SHIFT_MARGIN;
		static const float ABS_SLIP;
		static const float ABS_MINSPEED;
		static const float TCL_SLIP;
		static const float TCL_MINSPEED;
		static const float LOOKAHEAD_CONST;
		static const float LOOKAHEAD_FACTOR;
		static const float WIDTHDIV;
		static const float SIDECOLL_MARGIN;
		static const float BORDER_OVERTAKE_MARGIN;
		static const float OVERTAKE_OFFSET_SPEED;
		static const float PIT_LOOKAHEAD;
		static const float PIT_BRAKE_AHEAD;
		static const float PIT_MU;
		static const float MAX_SPEED;
		static const float TCL_RANGE;
		/* track variables */
		tTrack* track;
};

#endif // _DRIVER_H_

