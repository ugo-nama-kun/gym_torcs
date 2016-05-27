/***************************************************************************

    file                 : opponent.cpp
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: opponent.cpp,v 1.7 2005/04/10 23:37:01 olethros Exp $

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

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros {
#endif

// class variables and constants.
tTrack* Opponent::track;
const float Opponent::FRONTCOLLDIST = 200.0;			// [m] distance on the track to check other cars.
const float Opponent::BACKCOLLDIST = 70.0;				// [m] distance on the track to check other cars.
const float Opponent::LENGTH_MARGIN = 3.0;				// [m] safety margin.
const float Opponent::SIDE_MARGIN = 1.0;				// [m] safety margin.
const float Opponent::TIME_MARGIN = 2.0;				// [s] safety margin.
const float Opponent::EXACT_DIST = 12.0;				// [m] if the estimated distance is smaller, compute it more accurate
const float Opponent::LAP_BACK_TIME_PENALTY = -30.0;	// [s]
const float Opponent::OVERLAP_WAIT_TIME = 5.0;			// [s] overlaptimer must reach this time before we let the opponent pass.
const float Opponent::SPEED_PASS_MARGIN = 5.0;			// [m/s] avoid overlapping opponents to stuck behind me.


Opponent::Opponent()
{
	brake_overtake_filter = 1.0f;
}


void Opponent::update(tSituation *s, Driver *driver)
{
	tCarElt *mycar = driver->getCarPtr();

	// Init state of opponent to ignore.
	state = OPP_IGNORE;

	// If the car is out of the simulation ignore it.
	if (car->_state & RM_CAR_STATE_NO_SIMU) {
		return;
	}

	// update brake fiter
	float dt = s->deltaTime;
	brake_overtake_filter *= exp(-dt*.5);
	
	// Updating distance along the middle.
	float oppToStart = car->_trkPos.seg->lgfromstart + getDistToSegStart();
	distance = oppToStart - mycar->_distFromStartLine;
	if (distance > track->length/2.0) {
		distance -= track->length;
	} else if (distance < -track->length/2.0) {
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
				Straight carFrontLine(
					mycar->_corner_x(FRNT_LFT),
					mycar->_corner_y(FRNT_LFT),
					mycar->_corner_x(FRNT_RGT) - mycar->_corner_x(FRNT_LFT),
					mycar->_corner_y(FRNT_RGT) - mycar->_corner_y(FRNT_LFT)
				);

				float mindist = FLT_MAX;
				int i;
				for (i = 0; i < 4; i++) {
					v2d corner(car->_corner_x(i), car->_corner_y(i));
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
			cardist = fabs(cardist) - fabs(getWidth()/2.0) - mycar->_dimension_y/2.0;
			// smooth collision ? 
			float du = fabs(getSpeed() - driver->getSpeed());
			float t_impact = 10;
			if (du>0) {
				t_impact = fabs(distance/du);
			}
			if (cardist < SIDE_MARGIN
				&& t_impact < TIME_MARGIN) {
				state |= OPP_COLL;
				//printf ("OPP_COLL\n");
			}
		} else
		// Is opponent behind and faster.
		if (distance < -SIDECOLLDIST
			&& getSpeed() > driver->getSpeed() - SPEED_PASS_MARGIN) {
			catchdist = driver->getSpeed()*distance/(getSpeed() - driver->getSpeed());
			state |= OPP_BACK;
			distance -= MAX(car->_dimension_x, mycar->_dimension_x);
			distance -= LENGTH_MARGIN;
		} else
		// Is opponent aside.
		if (distance > -SIDECOLLDIST
			&& distance < SIDECOLLDIST) {
			sidedist = car->_trkPos.toMiddle - mycar->_trkPos.toMiddle;
			state |= OPP_SIDE;
			//printf ("OPP_SIDE\n");
		} else
		// Opponent is in front and faster.
		if (distance > SIDECOLLDIST && getSpeed() > driver->getSpeed()) {
			state |= OPP_FRONT_FAST;
			//printf ("OPP_FRONT_FAST\n");
		}
	}

	// Check if we should let overtake the opponent.
	updateOverlapTimer(s, mycar);
	if (overlaptimer > OVERLAP_WAIT_TIME) {
		state |= OPP_LETPASS;
		//printf ("OPP_LETPASS\n");
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
			if (overlaptimer > 0.0) {
				if (getState() & OPP_FRONT_FAST) {
					overlaptimer = MIN(0.0, overlaptimer);
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
	tCarElt* mycar = driver->getCarPtr();
	nopponents_behind = 0;
	nopponents_infront = 0;
	for (i = 0; i < nopponents; i++) {
		opponent[i].update(s, driver);
		if (1) {//opponent[i].getState() != OPP_IGNORE) {
			tCarElt* ocar = opponent[i].getCarPtr();
			if (ocar->_pos > mycar->_pos) {
				nopponents_behind++;
			} else {
				nopponents_infront++;
			}
		}
	}
}

#ifdef USE_OLETHROS_NAMESPACE
}
#endif
