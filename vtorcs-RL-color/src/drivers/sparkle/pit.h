/***************************************************************************

    file                 : pit.h
    created              : Thu Mai 15 2:41:00 CET 2003
    copyright            : (C) 2003 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: pit.h,v 1.2 2003/08/13 00:03:15 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _PIT_H_
#define _PIT_H_

#include "driver.h"
#include "spline.h"

#define NPOINTS 7


class Driver;


class Pit {
	public:
		Pit(tSituation *s, Driver *driver);
		~Pit();

		void setPitstop(bool pitstop);
		bool getPitstop() { return pitstop; }

		void setInPit(bool inpitlane) { this->inpitlane = inpitlane; }
		bool getInPit() { return inpitlane; }

		float getPitOffset(float offset, float fromstart);

		bool isBetween(float fromstart);

		float getNPitStart() { return p[1].x; }
		float getNPitLoc() { return p[3].x; }
		float getNPitEnd() { return p[5].x; }

		float toSplineCoord(float x);

		float getSpeedlimitSqr() { return speedlimitsqr; }
		float getSpeedlimit() { return speedlimit; }
		float getSpeedLimitBrake(float speedsqr);

		void update();
		int getRepair();
		float getFuel();

	private:
		tTrack *track;
		tCarElt *car;
		tTrackOwnPit *mypit;    /* pointer to my pit */
		tTrackPitInfo *pitinfo; /* general pit info */

		SplinePoint p[NPOINTS]; /* spline points */
		Spline *spline;         /* spline */

		bool pitstop;           /* pitstop planned */
		bool inpitlane;         /* we are still in the pit lane */
		float pitentry;         /* distance to start line of the pit entry */
		float pitexit;          /* distance to the start line of the pit exit */
		float speedlimitsqr;    /* pit speed limit squared */
		float speedlimit;       /* pit speed limit */
		float pitspeedlimitsqr;	/* the original speedlimit squared */

		bool fuelchecked;       /* fuel statistics updated */
		float lastfuel;         /* the fuel available when we cross the start lane */
		float lastpitfuel;      /* amount refueled, special case when we refuel */
		float fuelperlap;       /* the maximum amount of fuel we needed for a lap */

		static const float SPEED_LIMIT_MARGIN;
		static const int PIT_DAMMAGE;
};

#endif // _PIT_H_


