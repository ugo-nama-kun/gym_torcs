/***************************************************************************

    file                 : spline.h
    created              : Wed Mai 14 19:53:00 CET 2003
    copyright            : (C) 2003-2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: spline.h,v 1.1.2.1 2008/05/28 21:34:45 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SPLINE_H_
#define _SPLINE_H_

class SplinePoint {
	public:
		float x;	// x coordinate.
		float y;	// y coordinate.
		float s;	// slope.
};


class Spline {
	public:
		Spline(int dim, SplinePoint *s);

		float evaluate(float z);

	private:
		SplinePoint *s;
		int dim;
};

#endif // _SPLINE_H_

