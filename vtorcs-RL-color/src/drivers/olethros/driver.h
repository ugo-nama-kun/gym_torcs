// -*- Mode: C++ -*-
/***************************************************************************

    file                 : driver.h
    created              : Thu Dec 20 01:20:19 CET 2002
    copyright            : (C) 2002-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: driver.h,v 1.11.2.1 2008/11/09 17:50:20 berniw Exp $

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
#include "learn.h"
#include "strategy.h"
#include "cardata.h"
#include "geometry.h"

#define OLETHROS_SECT_PRIV "olethros private"
#define OLETHROS_ATT_FUELPERLAP "fuel per lap"
#define OLETHROS_ATT_FUELCONSUMPTION "fuel consumption"
#define OLETHROS_ATT_MUFACTOR "mufactor"

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif

class Opponents;
class Opponent;
class Pit;
class AbstractStrategy;

/**
   \brief the Driver class
*/
class Driver {
public:
	Driver(int index);
	~Driver();

	/// Callback functions called from TORCS.
	void initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s);
	void newRace(tCarElt* car, tSituation *s);
	void drive(tSituation *s);
	int pitCommand(tSituation *s);
	void endRace(tSituation *s);

	tCarElt *getCarPtr() { return car; }
	tTrack *getTrackPtr() { return track; }
	float* max_speed_list;
	float getSpeed() { return mycardata->getSpeedInTrackDirection(); /*speed;*/ }

protected:
	// Utility functions.
	bool isStuck();
	void update(tSituation *s);
	void prepareTrack();
	float getAllowedSpeed(tTrackSeg *segment);
	float getAccel();
	float getDistToSegEnd();
	float getBrake();
	int getGear();
	float EstimateTorque (float rpm);
	float getSteer();
	float getClutch();
	v2d getTargetPoint(); 
	float getOffset();
	float brakedist(float allowedspeed, float mu);

	float filterOverlap(float accel);
	float filterBColl(float brake);
	float filterABS(float brake);
	float filterBPit(float brake);
	float filterBrakeSpeed(float brake);
	float filterTurnSpeed(float brake);

	float filterTCL(float accel);
	float filterTrk(tSituation* s, float accel);
	float filterAPit(float accel);

	float filterSColl(float steer);

	float filterTCL_RWD();
	float filterTCL_FWD();
	float filterTCL_4WD();
	void initTCLfilter();

	void initCa();
	void initCw();
	void initTireMu();

	void computeRadius(float *radius);
	int isAlone();

	float EstimateRadius2 (tTrackSeg* seg);
	float EstimateRadius (tTrackSeg* seg, tTrackSeg* prev_seg, tTrackSeg* next_seg);
	float FindCurveTarget(tTrackSeg* seg, Vector* C, float rmax);
	float FindStraightTarget(tTrackSeg* curve, tTrackSeg* seg, Vector* C, float rmax, bool& flag);
	// Misc functions
	void AdjustRadi(tTrackSeg* cs, tTrackSeg* ce, float* radi);
	void ShowPaths();

	float current_allowed_speed;
	// Per robot global data.
	int race_type;
	int stuck;
	float speedangle;		///< the angle of the speed vector relative to trackangle, > 0.0 points to right.
	float mass;				///< Mass of car + fuel.
	float myoffset;			///< Offset to the track middle.
	tCarElt *car;			///< Pointer to tCarElt struct.

	Opponents *opponents;	///< The container for opponents.
	Opponent *opponent;		///< The array of opponents.

	Pit *pit;						///< Pointer to the pit instance.
	float pit_exit_timer;
	AbstractStrategy *strategy;		///< Pit stop strategy.

	static Cardata *cardata;		///< Data about all cars shared by all instances.
	SingleCardata *mycardata;		///< Pointer to "global" data about my car.
	static double currentsimtime;	///< Store time to avoid useless updates.

	float currentspeedsqr;	///< Square of the current speed_x.
	float clutchtime;		///< Clutch timer.
	float oldlookahead;		///< Lookahead for steering in the previous step.

	float speed_factor; ///< speed factor to use.

	float *seg_alpha; ///< targets for segments
	float *seg_alpha_new; ///< new targets for segments
	float *radius; ///< segment radi
	float *ideal_radius; ///< ideal radius (according to circle fit)
	SegLearn *learn; ///< handle to learning module
	int alone; ///< whether we are alone
	bool overtaking; ///< are we overtaking?
	float prev_steer; ///< for steering filter
	float prev_toleft; ///< previous left margin
	float prev_toright; ///< previous right margin
	float u_toleft; ///< speed to left side
	float u_toright; ///< speed to right side
	float dt; ///< delta time
	float my_pitch;
	float TCL_status; ///< traction control
	float alone_count; 

	// Data that should stay constant after first initialization.
	int MAX_UNSTUCK_COUNT;
	int INDEX;
	float CARMASS;		///< Mass of the car only [kg].
	float CA;			///< Aerodynamic downforce coefficient.
	float CW;			///< Aerodynamic drag coefficient.
	float TIREMU;		///< Friction coefficient of tires.
	float (Driver::*GET_DRIVEN_WHEEL_SPEED)();
	float OVERTAKE_OFFSET_INC;		///< [m/timestep]
	float MU_FACTOR;				///< [-]

	// Class constants.
	static const float MAX_UNSTUCK_ANGLE;
	static const float UNSTUCK_TIME_LIMIT;
	static const float MAX_UNSTUCK_SPEED;
	static const float MIN_UNSTUCK_DIST;
	static const float G;
	static const float FULL_ACCEL_MARGIN;
	static const float SHIFT;
	static const float SHIFT_MARGIN;
	static const float ABS_SLIP;
	static const float ABS_RANGE ;
	static const float ABS_MINSPEED;
	static const float TCL_SLIP;
	static const float LOOKAHEAD_CONST;
	static const float LOOKAHEAD_FACTOR;
	static const float WIDTHDIV;
	static const float SIDECOLL_MARGIN;
	static const float BORDER_OVERTAKE_MARGIN;
	static const float OVERTAKE_OFFSET_SPEED;
	static const float OVERTAKE_TIME;
	static const float PIT_LOOKAHEAD;
	static const float PIT_BRAKE_AHEAD;
	static const float PIT_MU;
	static const float MAX_SPEED;
	static const float TCL_RANGE;
	static const float MAX_FUEL_PER_METER;
	static const float CLUTCH_SPEED;
	static const float CENTERDIV;
	static const float DISTCUTOFF;
	static const float MAX_INC_FACTOR;
	static const float CATCH_FACTOR;
	static const float CLUTCH_FULL_MAX_TIME;
	static const float USE_LEARNED_OFFSET_RANGE;
	static const float ACCELERATOR_LETGO_TIME;
	static const float MIN_BRAKE_FOLLOW_DISTANCE;
	static const float MAX_BRAKE_FOLLOW_DISTANCE;
	static const float STEER_DIRECTION_GAIN;
	static const float STEER_PREDICT_GAIN;
	static const float STEER_DRIFT_GAIN;
	static const float STEER_AVOIDANCE_GAIN;
	static const float STEER_EMERGENCY_GAIN;
	static const float FILTER_STEER_FEEDBACK;
	static const float FILTER_PREDICT_FEEDBACK;
	static const float FILTER_TARGET_FEEDBACK;
	static const bool USE_NEW_ALPHA;
	// Track variables.
	tTrack* track;
};

#ifdef USE_OLETHROS_NAMESPACE
}
#endif

#endif // _DRIVER_H_

