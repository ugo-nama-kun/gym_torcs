/***************************************************************************

    file                 : pit.cpp
    created              : Thu Mai 15 2:43:00 CET 2003
    copyright            : (C) 2003 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: pit.cpp,v 1.2 2003/08/13 00:03:15 berniw Exp $

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

const float Pit::SPEED_LIMIT_MARGIN = 0.5;      /* [m/s] savety margin */
const int Pit::PIT_DAMMAGE = 5000;              /* [-] */


Pit::Pit(tSituation *s, Driver *driver)
{
	track = driver->getTrackPtr();
	car = driver->getCarPtr();
	mypit = driver->getCarPtr()->_pit;
	pitinfo = &track->pits;
	pitstop = inpitlane = false;
	fuelchecked = false;
	fuelperlap = 0.0;
	lastpitfuel = 0.0;
	lastfuel = car->priv.fuel;

	if (mypit != NULL) {
		speedlimit = pitinfo->speedLimit - SPEED_LIMIT_MARGIN;
		speedlimitsqr = speedlimit*speedlimit;
		pitspeedlimitsqr = pitinfo->speedLimit*pitinfo->speedLimit;

		/* compute pit spline points along the track */
		p[3].x = mypit->pos.seg->lgfromstart + mypit->pos.toStart;
		p[2].x = p[3].x - pitinfo->len;
		p[4].x = p[3].x + pitinfo->len;
		p[0].x = pitinfo->pitEntry->lgfromstart;
		p[1].x = pitinfo->pitStart->lgfromstart;
		p[5].x = p[3].x + (pitinfo->nMaxPits - car->index)*pitinfo->len;
		p[6].x = pitinfo->pitExit->lgfromstart;
		pitentry = p[0].x;
		pitexit = p[6].x;

		/* normalizing spline segments to >= 0.0 */
		int i;
		for (i = 0; i < NPOINTS; i++) {
			p[i].s = 0.0;
			p[i].x = toSplineCoord(p[i].x);
		}

		if (p[1].x > p[2].x) p[1].x = p[2].x;
		if (p[4].x > p[5].x) p[5].x = p[4].x;

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
	if (mypit != NULL) delete spline;
}


/* Transforms track coordinates to spline parameter coordinates */
float Pit::toSplineCoord(float x)
{
	x -= pitentry;
	while (x < 0.0) {
		x += track->length;
	}
	return x;
}


/* computes offset to track middle for trajectory */
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


/* Sets the pitstop flag if we are not in the pit range */
void Pit::setPitstop(bool pitstop)
{
	if (mypit == NULL) return;
	float fromstart = car->_distFromStartLine;

	if (!isBetween(fromstart)) {
		this->pitstop = pitstop;
	} else if (!pitstop) {
		this->pitstop = pitstop;
	}
}


/* Check if the argument fromstart is in the range of the pit */
bool Pit::isBetween(float fromstart)
{
	if (pitentry <= pitexit) {
		if (fromstart >= pitentry && fromstart <= pitexit) {
			return true;
		} else {
			return false;
		}
	} else {
		if ((fromstart >= 0.0 && fromstart <= pitexit) ||
			(fromstart >= pitentry && fromstart <= track->length))
		{
			return true;
		} else {
			return false;
		}
	}
}


/* update pit data and strategy */
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

		/* check for damage */
		if (car->_dammage > PIT_DAMMAGE) {
			setPitstop(true);
		}

		/* fuel update */
		int id = car->_trkPos.seg->id;
		if (id >= 0 && id < 5 && !fuelchecked) {
			if (car->race.laps > 0) {
				fuelperlap = MAX(fuelperlap, (lastfuel+lastpitfuel-car->priv.fuel));
			}
			lastfuel = car->priv.fuel;
			lastpitfuel = 0.0;
			fuelchecked = true;
		} else if (id > 5) {
			fuelchecked = false;
		}

		int laps = car->_remainingLaps-car->_lapsBehindLeader;
		if (!getPitstop() && laps > 0) {
			if (car->_fuel < 1.5*fuelperlap &&
				car->_fuel < laps*fuelperlap) {
				setPitstop(true);
			}
		}

		if (getPitstop()) car->_raceCmd = RM_CMD_PIT_ASKED;
	}
}


float Pit::getSpeedLimitBrake(float speedsqr)
{
	return (speedsqr-speedlimitsqr)/(pitspeedlimitsqr-speedlimitsqr);
}

/* Computes the amount of fuel */
float Pit::getFuel()
{
	float fuel;
	fuel = MAX(MIN((car->_remainingLaps+1.0)*fuelperlap - car->_fuel,
					car->_tank - car->_fuel),
			   0.0);
	lastpitfuel = fuel;
	return fuel;
}


/* Computes how much damage to repair */
int Pit::getRepair()
{
	return car->_dammage;
}


