/***************************************************************************

    file                 : spline.cpp
    created              : Wed Mai 14 20:10:00 CET 2003
    copyright            : (C) 2003-2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: spline.cpp,v 1.1.2.1 2008/05/28 21:34:45 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "spline.h"


Spline::Spline(int dim, SplinePoint *s)
{
	this->s = s;
	this->dim = dim;
}


float Spline::evaluate(float z)
{
	int i, a, b;
	float t, a0, a1, a2, a3, h;

	// Binary search for interval.
	a = 0; b = dim - 1;
	do {
		i = (a + b) / 2;
		if (s[i].x <= z) {
			a = i;
		} else {
			b = i;
		}
	} while ((a + 1) != b);

	// Evaluate.
	i = a;
	h = s[i+1].x - s[i].x;
	t = (z-s[i].x) / h;
	a0 = s[i].y;
	a1 = s[i+1].y - a0;
	a2 = a1 - h*s[i].s;
	a3 = h * s[i+1].s - a1;
	a3 -= a2;
	return a0 + (a1 + (a2 + a3*t) * (t-1))*t;
}

