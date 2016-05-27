/***************************************************************************

    file                 : opponent.cpp
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: opponent.cpp,v 1.9 2006/03/06 22:43:50 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "opponent.h"


// class variables and constants.
tTrack* Opponent::track;
const float Opponent::FRONTCOLLDIST = 200.0f;			// [m] distance on the track to check other cars.
const float Opponent::BACKCOLLDIST = 70.0f;				// [m] distance on the track to check other cars.
const float Opponent::LENGTH_MARGIN = 3.0f;				// [m] savety margin.
const float Opponent::SIDE_MARGIN = 1.0f;				// [m] savety margin.
const float Opponent::EXACT_DIST = 12.0f;				// [m] if the estimated distance is smaller, compute it more accurate
const float Opponent::LAP_BACK_TIME_PENALTY = -30.0f;	// [s]
const float Opponent::OVERLAP_WAIT_TIME = 5.0f;			// [s] overlaptimer must reach this time before we let the opponent pass.
const float Opponent::SPEED_PASS_MARGIN = 5.0f;			// [m/s] avoid overlapping opponents to stuck behind me.


Opponent::Opponent() : teammate(false)
{
}


void Opponent::update(tSituation *s, Driver *driver)
{
	tCarElt *mycar = driver->getCarPtr();

	// Init state of opponent to ignore.
	state = OPP_IGNORE;

	// If the car is out of the simulation ignore it.
	if (car->_state & (RM_CAR_STATE_NO_SIMU & ~RM_CAR_STATE_PIT)) {
		return;
	}

	// Updating distance along the middle.
	float oppToStart = car->_trkPos.seg->lgfromstart + getDistToSegStart();
	distance = oppToStart - mycar->_distFromStartLine;
	if (distance > track->length/2.0f) {
		distance -= track->length;
	} else if (distance < -track->length/2.0f) {
		distance += track->length;
	}

	float SIDECOLLDIST = MIN(car->_dimension_x, mycar->_dimension_x);

	// Is opponent in relevant range -BACKCOLLDIST..FRONTCOLLDIST m.
	if (distance > -BACKCOLLDIST && distance < FRONTCOLLDIST) {
		// Is opponent in front and slower.
		if (distance > SIDECOLLDIST && getSpeed() < driver->getSpeed()) {
			state |= OPP_FRONT;

			distance -= MAX(car->_dimension_x, mycar->_dimension_x);
			distance -= LENGTH_MARGIN;

			// If the distance is small we compute it more accurate.
			if (distance < EXACT_DIST) {
				straight2f carFrontLine(
					mycar->_corner_x(FRNT_LFT),
					mycar->_corner_y(FRNT_LFT),
					mycar->_corner_x(FRNT_RGT) - mycar->_corner_x(FRNT_LFT),
					mycar->_corner_y(FRNT_RGT) - mycar->_corner_y(FRNT_LFT)
				);

				float mindist = FLT_MAX;
				int i;
				for (i = 0; i < 4; i++) {
					vec2f corner(car->_corner_x(i), car->_corner_y(i));
					float dist = carFrontLine.dist(corner);
					if (dist < mindist) {
						mindist = dist;
					}
				}

				if (mindist < distance) {
					distance = mindist;
				}
			}

			catchdist = driver->getSpeed()*distance/(driver->getSpeed() - getSpeed());

			float cardist = car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
			sidedist = cardist;
			cardist = fabs(cardist) - fabs(getWidth()/2.0f) - mycar->_dimension_y/2.0f;
			if (cardist < SIDE_MARGIN) {
				state |= OPP_COLL;
			}
		} else
		// Is opponent behind and faster.
		if (distance < -SIDECOLLDIST && getSpeed() > driver->getSpeed() - SPEED_PASS_MARGIN) {
			catchdist = driver->getSpeed()*distance/(getSpeed() - driver->getSpeed());
			state |= OPP_BACK;
			distance -= MAX(car->_dimension_x, mycar->_dimension_x);
			distance -= LENGTH_MARGIN;
		} else
		// Is opponent aside.
		if (distance > -SIDECOLLDIST &&
			distance < SIDECOLLDIST) {
			sidedist = car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
			state |= OPP_SIDE;
		} else
		// Opponent is in front and faster.
		if (distance > SIDECOLLDIST && getSpeed() > driver->getSpeed()) {
			state |= OPP_FRONT_FAST;
		}
	}

	// Check if we should let overtake the opponent.
	updateOverlapTimer(s, mycar);
	if (overlaptimer > OVERLAP_WAIT_TIME) {
		state |= OPP_LETPASS;
	}
}


// Compute the length to the start of the segment.
float Opponent::getDistToSegStart()
{
	if (car->_trkPos.seg->type == TR_STR) {
		return car->_trkPos.toStart;
	} else {
		return car->_trkPos.toStart*car->_trkPos.seg->radius;
	}
}


// Update overlaptimers of opponents.
void Opponent::updateOverlapTimer(tSituation *s, tCarElt *mycar)
{
	if (car->race.laps > mycar->race.laps) {
		if (getState() & (OPP_BACK | OPP_SIDE)) {
			overlaptimer += s->deltaTime;
		} else if (getState() & OPP_FRONT) {
			overlaptimer = LAP_BACK_TIME_PENALTY;
		} else {
			if (overlaptimer > 0.0f) {
				if (getState() & OPP_FRONT_FAST) {
					overlaptimer = MIN(0.0f, overlaptimer);
				} else {
					overlaptimer -= s->deltaTime;
				}
			} else {
				overlaptimer += s->deltaTime;
			}
		}
	} else {
		overlaptimer = 0.0;
	}
}


// Initialize the list of opponents.
Opponents::Opponents(tSituation *s, Driver *driver, Cardata *c)
{
	opponent = new Opponent[s->_ncars - 1];
	int i, j = 0;
	for (i = 0; i < s->_ncars; i++) {
		if (s->cars[i] != driver->getCarPtr()) {
			opponent[j].setCarPtr(s->cars[i]);
			opponent[j].setCarDataPtr(c->findCar(s->cars[i]));
			j++;
		}
	}
	Opponent::setTrackPtr(driver->getTrackPtr());
	nopponents = s->_ncars - 1;
}


Opponents::~Opponents()
{
	delete [] opponent;
}


void Opponents::update(tSituation *s, Driver *driver)
{
	int i;
	for (i = 0; i < s->_ncars - 1; i++) {
		opponent[i].update(s, driver);
	}
}


void Opponents::setTeamMate(char *teammate)
{
	int i;
	for (i = 0; i < nopponents; i++) {
		if (strcmp(opponent[i].getCarPtr()->_name, teammate) == 0) {
			opponent[i].markAsTeamMate();
			break;	// Name should be unique, so we can stop.
		}
	}
}




