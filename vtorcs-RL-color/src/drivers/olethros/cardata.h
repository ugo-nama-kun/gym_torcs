// -*- Mode: c++; tab-width: 4; c-basic-offset: 4 -*-
/***************************************************************************

    file                 : cardata.h
    created              : Thu Sep 23 12:31:33 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: cardata.h,v 1.9 2005/08/05 08:54:08 berniw Exp $

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
  This class holds global facts about cars, therefore no data relative to
  each other (for that is the class Opponents/Opponent responsible).
*/

#ifndef _OLETHROS_CARDATA_H_
#define _OLETHROS_CARDATA_H_

#include <stdio.h>
#include <math.h>
#include <car.h>
#include <robottools.h>
#include <raceman.h>

#include "linalg.h"
#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif
class SingleCardata {
 public:
	SingleCardata() {
		speed = 0.0;
		width = 1.0;
		trackangle = 0.0;
		angle = 0.0;
		car = NULL;
	}
	inline void init(CarElt *car) { this->car = car; }

	inline float getSpeedInTrackDirection() {
		return speed;
	}
	inline float getWidthOnTrack() { return width; }
	inline float getTrackangle() { return trackangle; }
	inline float getCarAngle() { return angle; }
	
	inline bool thisCar(tCarElt *car) { return (car == this->car); }

	void update();

 protected:
	static float getSpeed(tCarElt *car, float trackangle);

	float speed;		// speed in direction of the track.
	float width;		// the cars needed width on the track.
	float trackangle;	// Track angle at the opponents position.
	float angle;		// The angle of the car relative to the track tangent.

	tCarElt *car;		// For identification.
};


class Cardata {
 public:
	Cardata(tSituation* s);
	~Cardata();
	void update();
	SingleCardata *findCar(tCarElt *car);

 protected:
	SingleCardata *data;	// Array with car data.
	int ncars;				// # of elements in data.
};


#ifdef USE_OLETHROS_NAMESPACE
}
#endif

#endif // _OLETHROS_CARDATA_H_
