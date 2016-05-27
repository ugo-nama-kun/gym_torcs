/***************************************************************************

    file                 : opponent.h
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: opponent.h,v 1.1.2.2 2008/11/09 17:50:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OPPONENT_H_
#define _OPPONENT_H_

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

#include "driver.h"
#include "cardata.h"

#define OPP_IGNORE		0
#define OPP_FRONT		(1<<0)
#define OPP_BACK		(1<<1)
#define OPP_SIDE		(1<<2)
#define OPP_COLL		(1<<3)
#define OPP_LETPASS		(1<<4)
#define OPP_FRONT_FAST	(1<<5)


class Driver;

// Opponent maintains the data for one opponent RELATIVE to the drivers car.
class Opponent {
	public:
		Opponent();

		void setCarPtr(tCarElt *car) { this->car = car; }
		void setCarDataPtr(SingleCardata *cardata) { this->cardata = cardata; }
		static void setTrackPtr(tTrack *track) { Opponent::track = track; }

		tCarElt *getCarPtr() { return car; }
		int getState() { return state; }
		float getCatchDist() { return catchdist; }
		float getDistance() { return distance; }
		float getSideDist() { return sidedist; }
		float getWidth() { return cardata->getWidthOnTrack(); }
		float getSpeed() { return cardata->getSpeedInTrackDirection(); }
		float getOverlapTimer() { return overlaptimer; }

		bool isTeamMate() { return teammate; }
		int getDamage() { return car->_dammage; }
		void markAsTeamMate() { teammate = true; }

		void update(tSituation *s, Driver *driver);

	private:
		float getDistToSegStart();
		void updateOverlapTimer(tSituation *s, tCarElt *mycar);

		float distance;		// approximation of the real distance, negative if the opponent is behind.
		float catchdist;	// distance needed to catch the opponent (linear estimate).
		float sidedist;		// approx distance of center of gravity of the cars.
		int state;			// State variable to characterize the relation to the opponent, e. g. opponent is behind.
		float overlaptimer;

		tCarElt *car;
		SingleCardata *cardata;		// Pointer to global data about this opponent.
		bool teammate;				// Is this opponent a team mate of me (configure it in setup XML)?

		// class variables.
		static tTrack *track;

		// constants.
		static const float FRONTCOLLDIST;
		static const float BACKCOLLDIST;
		static const float LENGTH_MARGIN;
		static const float SIDE_MARGIN;
		static const float EXACT_DIST;
		static const float LAP_BACK_TIME_PENALTY;
		static const float OVERLAP_WAIT_TIME;
		static const float SPEED_PASS_MARGIN;
};


// The Opponents class holds an array of all Opponents.
class Opponents {
	public:
		Opponents(tSituation *s, Driver *driver, Cardata *cardata);
		~Opponents();

		void update(tSituation *s, Driver *driver);
		Opponent *getOpponentPtr() { return opponent; }
		int getNOpponents() { return nopponents; }
		void setTeamMate(char *teammate);

	private:
		Opponent *opponent;
		int nopponents;
};


#endif // _OPPONENT_H_
