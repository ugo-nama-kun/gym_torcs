/***************************************************************************

    file                 : mycar.h
    created              : Mon Oct 10 13:51:00 CET 2001
    copyright            : (C) 2001-2006 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: mycar.h,v 1.1.2.2 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	this class holds some properties of the car
*/

#ifndef _MYCAR_H_
#define _MYCAR_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>
#include <math.h>
#include <tmath/linalg_t.h>

#include "trackdesc.h"
#include "pathfinder.h"



class Pathfinder;
class PathSeg;

class AbstractCar
{
	public:
		AbstractCar() {};
		~AbstractCar() {};
		inline tCarElt* getCarPtr() { return me; }
		inline vec2d* getCurrentPos() { return &currentpos; }
		inline vec2d* getDir() { return &dir; }
		inline double getSpeedSqr() { return speedsqr; }
		inline double getSpeed() { return speed; }
		inline int getCurrentSegId() { return currentsegid; }

	protected:
		inline void setCarPtr(tCarElt* car) { me = car; }
		inline void updateDir() { dir.x = cos(me->_yaw); dir.y = sin(me->_yaw); }
		inline void updatePos() { currentpos.x = me->_pos_X; currentpos.y = me->_pos_Y; }
		inline void updateSpeedSqr() { speedsqr = (me->_speed_x)*(me->_speed_x) + (me->_speed_y)*(me->_speed_y) + (me->_speed_z)*(me->_speed_z); }
		inline void updateSpeed() { speed = sqrt(speedsqr); }
		inline void initCGh() { cgh = GfParmGetNum(me->_carHandle, SECT_CAR, PRM_GCHEIGHT, NULL, 0.0); }

		tCarElt* me;
		vec2d currentpos;
		vec2d dir;
		double speedsqr;
		double speed;
		int currentsegid;

	private:
		double cgh;		// Height of center of gravity.
};


class MyCar : public AbstractCar
{
	public:
		// Possible behaviours.
		enum {
			INSANE = 0, PUSH = 1, NORMAL = 2, CAREFUL = 3, SLOW = 4, START = 5
		};

		static const double PATHERR;				// If derror > PATHERR we take actions to come back to the path [m].
		static const double CORRLEN;				// CORRLEN * derror is the length of the correction [m].
		static const double TURNTOL;				// Tolerance for end backing up [m].
		static const double TURNSPEED;				// If speed lower than this you can back up [m/s].
		static const double MARGIN;					// Security margin from track border [m].
		static const double STABLESPEED;			// We brake currentspeed/stablespeed if car seems unstable [m/s].
		static const double TIMETOCATCH;			// When do we start thinking about overtaking [s].
		static const double MINOVERTAKERANGE;		// Minimum length for overtaking [m].
		static const double OVERTAKERADIUS;			// Min allowed radius to start overtaking [m].
		static const double OVERTAKEDIST;			// Planned distance of CG's while overtaking [m].
		static const double OVERTAKEMINDIST;		// Minimal distance of CG's while overtaking [m].
		static const double OVERTAKEANGLE;			// [-] radians.
		static const double MAXALLOWEDPITCH;		// [-] radians.
		static const double FLYSPEED;				// Speed where antifly checking gets activated [m/s].
		static const double OVERLAPSTARTDIST;		// Distance where we start to check the need to let pass the opponent.
		static const double OVERLAPPASSDIST;		// Distance smaller than that and waiting long enough -> let him pass.
		static const double OVERLAPWAITTIME;		// Minimal waiting time before we consider let him pass.
		static const double LAPBACKTIMEPENALTY; 	// Penalty if i am able to "lap back" [s].
		static const double TCL_SLIP;				// [m/s] range [0..10].
		static const double TCL_RANGE;				// [m/s] range [0..10].
		static const double SHIFT;					// [-] (% of rpmredline).
		static const double SHIFT_MARGIN;			// [m/s].
		static const double MAX_SPEED;				// [m/s].
		static const double MAX_FUEL_PER_METER;		// [liter/m] fuel consumtion.
		static const double LOOKAHEAD_MAX_ERROR;	// [m].
		static const double LOOKAHEAD_FACTOR;		// [-].
		static const double STEER_D_CONTROLLER_GAIN;// [-] Output gain of steering d-controller.
		static const float CLUTCH_SPEED;			// [m/s]
		static const float CLUTCH_FULL_MAX_TIME;	// [s] Time to apply full clutch.
		static const int TEAM_DAMAGE_CHANGE_LEAD;	// When to change position in the team? 
		static const float FUEL_SAFETY_MARGIN;		// [kg] How much additional fuel in parts of maxfuelperlap for random errors.

		// Data for behavior.
		int bmode;
		double behaviour[6][8];
		int MAXDAMMAGE;									// If dammage > MAXDAMMAGE then we plan a pit stop [-].
		double DIST;									// Minimal distance to other cars [m].
		double MAXRELAX;								// To avoid skidding (0..0.99) [-].
		double MAXANGLE;								// Biggest allowed angle to the path [deg].
		double ACCELINC;								// Increment/decrement for acceleration [-].
		double SPEEDSQRFACTOR;							// Multiplier for speedsqr.
		double GCTIME;									// Minimal time between gear changes.
		double ACCELLIMIT;								// Maximal allowed acceleration.
		double PATHERRFACTOR;							// If derror > PATHERR*PATHERRFACTOR we compute a corrected path [-].

		double CARWIDTH;								// Width of the car [m].
		double CARLEN;									// Length of the car [m].
		double AEROMAGIC;								// Aerodynamic lift factor [-].
		double CFRICTION;								// Friction "magic" coefficient [-].

		double STEER_P_CONTROLLER_MAX;					// [-] Maximum output of p-controller.
		double STEER_P_CONTROLLER_GAIN;					// [-] Output gain of p-controller.


		// Static data.
		double cgcorr_b;
		double ca;
		double cw;

		// Dynamic data.
		double mass;
		int destsegid;
		double trtime;

		TrackSegment2D* currentseg;
		TrackSegment2D* destseg;
		int currentpathsegid;
		int destpathsegid;
		PathSeg* dynpath;

		int undamaged;
		double lastfuel;
		double fuelperlap;
		double lastpitfuel;

		double turnaround;
		int tr_mode;
		double accel;
		bool fuelchecked;
		bool startmode;

		double derror;			// Distance to desired trajectory.
		float clutchtime;		// Clutch timer.

		MyCar(TrackDesc* track, tCarElt* car, tSituation *situation);
		~MyCar();

		void info(void);
		void update(TrackDesc* track, tCarElt* car, tSituation *situation);
		void loadBehaviour(int id);
		double querySlipSpeed(tCarElt* car);
		double queryAcceleration(tCarElt * car, double speed);
		inline double getDeltaPitch() { return deltapitch; }
		inline double getWheelBase() { return wheelbase; }
		inline double getWheelTrack() { return wheeltrack; }
		inline double getErrorSgn() { return derrorsgn; }
		inline Pathfinder* getPathfinderPtr() { return pf; }

	private:
		enum { DRWD = 0, DFWD = 1, D4WD = 2 };

		int drivetrain;			// RWD, FWD or 4WD.
		double carmass;			// Mass of car without fuel.
		double deltapitch;		// Angle of attack between road and car.
		double wheelbase;
		double wheeltrack;
		double derrorsgn;		// On which side of the trajectory am i left -1 or 1 right.

		Pathfinder* pf;

		void updateCa();
		void updateDError();
		void initCarGeometry();
};


class OtherCar: public AbstractCar
{
	public:
		void init(TrackDesc* itrack, tCarElt* car, tSituation *situation);
		void update(void);

	private:
		TrackDesc* track;
		double dt;
};

#endif // _MYCAR_H_

