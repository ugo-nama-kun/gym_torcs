/***************************************************************************

    file                 : cardata.cpp
    created              : Thu Sep 23 12:31:37 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: cardata.cpp,v 1.2 2006/03/06 22:43:50 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "cardata.h"


void SingleCardata::update()
{
	trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
	speed = getSpeed(car, trackangle);
	angle = trackangle - car->_yaw;
	NORM_PI_PI(angle);
	width = car->_dimension_x*sin(angle) + car->_dimension_y*cos(angle);
}


// compute speed component parallel to the track.
float SingleCardata::getSpeed(tCarElt *car, float ltrackangle)
{
	vec2f speed, dir;
	speed.x = car->_speed_X;
	speed.y = car->_speed_Y;
	dir.x = cos(ltrackangle);
	dir.y = sin(ltrackangle);
	return speed*dir;
}



Cardata::Cardata(tSituation *s)
{
	ncars = s->_ncars;
	data = new SingleCardata[ncars];
	int i;
	for (i = 0; i < ncars; i++) {
		data[i].init(s->cars[i]);
	}
}


Cardata::~Cardata()
{
	delete [] data;
}


void Cardata::update()
{
	int i;
	for (i = 0; i < ncars; i++) {
		data[i].update();
	}
}


SingleCardata *Cardata::findCar(tCarElt *car)
{
	int i;
	for (i = 0; i < ncars; i++) {
		if (data[i].thisCar(car)) {
			return &data[i];
		}
	}
	return NULL;
}


