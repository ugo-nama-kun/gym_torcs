/***************************************************************************

    file                 : pit.cpp
    created              : Thu Mai 15 2:43:00 CET 2003
    copyright            : (C) 2003-2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: pit.cpp,v 1.11 2006/10/12 21:26:42 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "pit.h"

const float Pit::SPEED_LIMIT_MARGIN = 0.5;		// [m/s] savety margin to avoid pit speeding.


Pit::Pit(tSituation *s, Driver *driver)
{
	track = driver->getTrackPtr();
	car = driver->getCarPtr();
	mypit = driver->getCarPtr()->_pit;
	pitinfo = &track->pits;
	pitstop = inpitlane = false;
	pittimer = 0.0;

	if (mypit != NULL) {
		speedlimit = pitinfo->speedLimit - SPEED_LIMIT_MARGIN;
		speedlimitsqr = speedlimit*speedlimit;
		pitspeedlimitsqr = pitinfo->speedLimit*pitinfo->speedLimit;

		// Compute pit spline points along the track.
		p[3].x = mypit->pos.seg->lgfromstart + mypit->pos.toStart;
		p[2].x = p[3].x - pitinfo->len;
		p[4].x = p[3].x + pitinfo->len;
		p[0].x = pitinfo->pitEntry->lgfromstart;
		p[1].x = pitinfo->pitStart->lgfromstart;
		p[5].x = pitinfo->pitEnd->lgfromstart + pitinfo->pitEnd->length;
		p[6].x = pitinfo->pitExit->lgfromstart + pitinfo->pitExit->length;

		pitentry = p[0].x;
		pitexit = p[6].x;

		// Normalizing spline segments to >= 0.0.
		int i;
		for (i = 0; i < NPOINTS; i++) {
			p[i].s = 0.0;
			p[i].x = toSplineCoord(p[i].x);
		}

		// Fix broken pit exit.
		if (p[6].x < p[5].x) {
			//printf("bt: Pitexit broken on track %s.\n", track->name);
			p[6].x = p[5].x + 50.0;
		}

		// Fix point for first pit if necessary.
		if (p[1].x > p[2].x) {
			p[1].x = p[2].x;
		}

		// Fix point for last pit if necessary.
		if (p[4].x > p[5].x) {
			p[5].x = p[4].x;
		}

		float sign = (pitinfo->side == TR_LFT) ? 1.0 : -1.0;
		p[0].y = 0.0;
		p[6].y = 0.0;
		for (i = 1; i < NPOINTS - 1; i++) {
			p[i].y = fabs(pitinfo->driversPits->pos.toMiddle) - pitinfo->width;
			p[i].y *= sign;
		}

		p[3].y = fabs(pitinfo->driversPits->pos.toMiddle)*sign;
		spline = new Spline(NPOINTS, p);
	}
}


Pit::~Pit()
{
	if (mypit != NULL) {
		delete spline;
	}
}


// Transforms track coordinates to spline parameter coordinates.
float Pit::toSplineCoord(float x)
{
	x -= pitentry;
	while (x < 0.0f) {
		x += track->length;
	}
	return x;
}


// Computes offset to track middle for trajectory.
float Pit::getPitOffset(float offset, float fromstart)
{
	if (mypit != NULL) {
		if (getInPit() || (getPitstop() && isBetween(fromstart))) {
			fromstart = toSplineCoord(fromstart);
			return spline->evaluate(fromstart);
		}
	}
	return offset;
}


// Sets the pitstop flag if we are not in the pit range.
void Pit::setPitstop(bool pitstop)
{
	if (mypit == NULL) {
		return;
	}

	float fromstart = car->_distFromStartLine;

	if (!isBetween(fromstart)) {
		this->pitstop = pitstop;
	} else if (!pitstop) {
		this->pitstop = pitstop;
		pittimer = 0.0f;
	}
}


// Check if the argument fromstart is in the range of the pit.
bool Pit::isBetween(float fromstart)
{
	if (pitentry <= pitexit) {
		if (fromstart >= pitentry && fromstart <= pitexit) {
			return true;
		} else {
			return false;
		}
	} else {
		// Warning: TORCS reports sometimes negative values for "fromstart"!
		if (fromstart <= pitexit || fromstart >= pitentry) {
			return true;
		} else {
			return false;
		}
	}
}


// Checks if we stay too long without getting captured by the pit.
// Distance is the distance to the pit along the track, when the pit is
// ahead it is > 0, if we overshoot the pit it is < 0.
bool Pit::isTimeout(float distance)
{
	if (car->_speed_x > 1.0f || distance > 3.0f || !getPitstop()) {
		pittimer = 0.0f;
		return false;
	} else {
		pittimer += (float) RCM_MAX_DT_ROBOTS;
		if (pittimer > 3.0f) {
			pittimer = 0.0f;
			return true;
		} else {
			return false;
		}
	}
}


// Update pit data and strategy.
void Pit::update()
{
	if (mypit != NULL) {
		if (isBetween(car->_distFromStartLine)) {
			if (getPitstop()) {
				setInPit(true);
			}
		} else {
			setInPit(false);
		}

		if (getPitstop()) {
			car->_raceCmd = RM_CMD_PIT_ASKED;
		}
	}
}


float Pit::getSpeedLimitBrake(float speedsqr)
{
	return (speedsqr-speedlimitsqr)/(pitspeedlimitsqr-speedlimitsqr);
}

