/***************************************************************************

    file                 : mycar.h
    created              : Mon Oct 10 13:51:00 CET 2001
    copyright            : (C) 2001-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: mycar.h,v 1.30 2003/11/12 01:45:30 berniw Exp $

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
#include <string.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>
#include <math.h>
#include "trackdesc.h"
#include "pathfinder.h"
#include "linalg.h"


class Pathfinder;
class PathSeg;

class AbstractCar
{
	public:
		AbstractCar() {};
		~AbstractCar() {};
		inline tCarElt* getCarPtr() { return me; }
		inline v3d* getCurrentPos() { return &currentpos; }
		inline v3d* getDir() { return &dir; }
		inline double getSpeedSqr() { return speedsqr; }
		inline double getSpeed() { return speed; }
		inline int getCurrentSegId() { return currentsegid; }

	protected:
		inline void setCarPtr(tCarElt* car) { me = car; }
		inline void updateDir() { dir.x = cos(me->_yaw); dir.y = sin(me->_yaw); dir.z = 0.0; }
		inline void updatePos() { currentpos.x = me->_pos_X; currentpos.y = me->_pos_Y; currentpos.z = me->_pos_Z - cgh; }
		inline void updateSpeedSqr() { speedsqr = (me->_speed_x)*(me->_speed_x) + (me->_speed_y)*(me->_speed_y) + (me->_speed_z)*(me->_speed_z); }
		inline void updateSpeed() { speed = sqrt(speedsqr); }
		inline void initCGh() { cgh = GfParmGetNum(me->_carHandle, SECT_CAR, PRM_GCHEIGHT, NULL, 0.0); }

		tCarElt* me;
		v3d currentpos;
		v3d dir;
		double speedsqr;
		double speed;
		int currentsegid;

	private:
		double cgh;			/* height of center of gravity */
};


class MyCar : public AbstractCar
{
	public:
		/* possible behaviours */
		enum {
			INSANE = 0, PUSH = 1, NORMAL = 2, CAREFUL = 3, SLOW = 4, START = 5
		};

		static const double PATHERR;			/* if derror > PATHERR we take actions to come back to the path [m] */
		static const double CORRLEN;			/* CORRLEN * derror is the length of the correction [m] */
		static const double TURNTOL;			/* tolerance for end backing up [m] */
		static const double TURNSPEED;			/* if speed lower than this you can back up [m/s] */
		static const double MARGIN;				/* security margin from track border [m] */
		static const double STABLESPEED;		/* we brake currentspeed/stablespeed if car seems unstable [m/s] */
		static const double TIMETOCATCH;		/* when do we start thinking about overtaking [s]*/
		static const double MINOVERTAKERANGE;	/* minimum length for overtaking [m] */
		static const double OVERTAKERADIUS;		/* min allowed radius to start overtaking [m] */
		static const double OVERTAKEDIST;		/* planned distance of CG's while overtaking [m] */
		static const double OVERTAKEMINDIST;	/* minimal distance of CG's while overtaking [m] */
		static const double OVERTAKEANGLE;		/* [-] radians */
		static const double OVERTAKEMARGIN;		/* [m] */
		static const double MAXALLOWEDPITCH;	/* [-] radians */
		static const double FLYSPEED;			/* speed where antifly checking gets activated [m/s] */
		static const double OVERLAPSTARTDIST;	/* distance where we start to check the need to let pass the opponent */
		static const double OVERLAPPASSDIST;	/* distance smaller than that and waiting long enough -> let him pass */
		static const double OVERLAPWAITTIME;	/* minimal waiting time before we consider let him pass */
		static const double LAPBACKTIMEPENALTY; /* penalty if i am able to "lap back" [s] */
		static const double TCL_SLIP;			/* [m/s] range [0..10] */
		static const double TCL_RANGE;			/* [m/s] range [0..10] */
		static const double SHIFT;				/* [-] (% of rpmredline) */
		static const double SHIFT_MARGIN;		/* [m/s] */
		static const double MAX_SPEED;			/* [m/s] */
		static const double MAX_FUEL_PER_METER;	/* [liter/m] fuel consumtion */
		static const double LOOKAHEAD_MAX_ERROR;/* [m] */
		static const double LOOKAHEAD_FACTOR;	/* [-] */

		/* data for behavior */
		int bmode;
		double behaviour[6][12];
		int MAXDAMMAGE;									/* if dammage > MAXDAMMAGE then we plan a pit stop [-] */
		double DIST;									/* minimal distance to other cars [m] */
		double MAXRELAX;								/* to avoid skidding (0..0.99) [-] */
		double MAXANGLE;								/* biggest allowed angle to the path [deg] */
		double ACCELINC;								/* increment/decrement for acceleration [-] */
		double SPEEDSQRFACTOR;							/* multiplier for speedsqr */
		double GCTIME;									/* minimal time between gear changes */
		double ACCELLIMIT;								/* maximal allowed acceleration */
		double PATHERRFACTOR;							/* if derror > PATHERR*PATHERRFACTOR we compute a corrected path [-] */
		double CARWIDTH;								/* width of the car [m] */
		double CARLEN;									/* length of the car [m] */
		double AEROMAGIC;								/* aerodynamic lift factor [-] */
		double CFRICTION;								/* friction "magic" coefficient [-] */

		/* static data */
		double cgcorr_b;
		double ca;
		double cw;

		/* dynamic data */
		double mass;
		int destsegid;
		double trtime;

		TrackSegment* currentseg;
		TrackSegment* destseg;
		PathSeg* currentpathseg;
		PathSeg* destpathseg;

		int undamaged;
		double lastfuel;
		double fuelperlap;
		double lastpitfuel;

		double turnaround;
		int tr_mode;
		double accel;
		bool fuelchecked;
		bool startmode;

		double derror;			/* distance to desired trajectory */

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

		int drivetrain;			/* RWD, FWD or 4WD */
		double carmass;			/* mass of car without fuel */
		double deltapitch;		/* angle between road and car */
		double wheelbase;
		double wheeltrack;
		double derrorsgn;		/* on which side of the trajectory am i left -1 or 1 right */

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

