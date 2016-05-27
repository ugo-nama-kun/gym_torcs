/***************************************************************************

    file                 : opponent.h
    created              : Thu Apr 22 01:20:19 CET 2003
    copyright            : (C) 2003 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: opponent.h,v 1.2.2.1 2008/11/09 17:50:20 berniw Exp $

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

#include "linalg.h"
#include "driver.h"

#define OPP_IGNORE	0
#define OPP_FRONT	(1<<0)
#define OPP_BACK	(1<<1)
#define OPP_SIDE	(1<<2)
#define OPP_COLL	(1<<3)


class Driver;

/* Opponent maintains the data for one opponent */
class Opponent {
	public:
		Opponent();

		void setCarPtr(tCarElt *car) { this->car = car; }
		static void setTrackPtr(tTrack *track) { Opponent::track = track; }

		static float getSpeed(tCarElt *car);
		tCarElt *getCarPtr() { return car; }
		int getState() { return state; }
		float getCatchDist() { return catchdist; }
		float getDistance() { return distance; }
		float getSideDist() { return sidedist; }
		float getWidth() { return width; }
		float getSpeed() { return speed; }

		void update(tSituation *s, Driver *driver);

	private:
		float getDistToSegStart();

		tCarElt *car;
		float distance;		/* approximation of the real distance */
		float speed;		/* speed in direction of the track */
		float catchdist;	/* distance needed to catch the opponent */
		float width;		/* the cars needed width on the track */
		float sidedist;		/* approx distance of center of gravity of the cars */
		int state;

		/* class variables */
		static tTrack *track;

		/* constants */
		static float FRONTCOLLDIST;
		static float BACKCOLLDIST;
		static float SIDECOLLDIST;
		static float LENGTH_MARGIN;
		static float SIDE_MARGIN;
};


/* The Opponents class holds an array of all Opponents */

class Opponents {
	public:
		Opponents(tSituation *s, Driver *driver);
		~Opponents();

		void update(tSituation *s, Driver *driver);
		Opponent *getOpponentPtr() { return opponent; }
		int getNOpponents() { return nopponents; }

	private:
		Opponent *opponent;
		int nopponents;
};


#endif // _OPPONENT_H_
