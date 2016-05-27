/*
  3D - C++ Class Library for 3D Transformations
  Copyright (C) 1996-1998  Gino van den Bergen

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free
  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  Please send remarks, questions and bug reports to gino@win.tue.nl,
  or write to:
                  Gino van den Bergen
		  Department of Mathematics and Computing Science
		  Eindhoven University of Technology
		  P.O. Box 513, 5600 MB Eindhoven, The Netherlands
*/

#ifndef _POINT_H_
#define _POINT_H_

#include <algorithm>

#include "Vector.h"

class Point : public Vector {
public:
  Point() {}
  Point(const float p[3]) : Vector(p) {} 
  Point(const double p[3]) : Vector(p) {}
  Point(Scalar x, Scalar y, Scalar z) : Vector(x, y, z) {}

  Point& operator+=(const Vector& v);
  Point& operator-=(const Vector& v);
  Point& operator=(const Vector& v);

  void setInf(const Point& p);
  void setSup(const Point& p);
};

Point operator+(const Point& p, const Vector& v);
Point operator-(const Point& p, const Vector& v);

Vector operator-(const Point& p1, const Point& p2);

bool operator<(const Point& p1, const Point& p2);

Point inf(const Point& p1, const Point& p2);
Point sup(const Point& p1, const Point& p2);

Point affine(const Point& p1, const Point& p2, Scalar t);

Scalar distance(const Point& p1, const Point& p2);
Scalar distance2(const Point& p1, const Point& p2);


inline Point& Point::operator+=(const Vector& v) {
  comp[X] += v[X]; comp[Y] += v[Y]; comp[Z] += v[Z];
  return *this;
}

inline Point& Point::operator-=(const Vector& v) {
  comp[X] -= v[X]; comp[Y] -= v[Y]; comp[Z] -= v[Z];
  return *this;
}

inline Point& Point::operator=(const Vector& v) {
  comp[X] = v[X]; comp[Y] = v[Y]; comp[Z] = v[Z];
  return *this;
}

inline void Point::setInf(const Point& p) {
  set_min(comp[X], p[X]); set_min(comp[Y], p[Y]); set_min(comp[Z], p[Z]);
}

inline void Point::setSup(const Point& p) {
  set_max(comp[X], p[X]); set_max(comp[Y], p[Y]); set_max(comp[Z], p[Z]);
}

inline Point operator+(const Point& p, const Vector& v) {
  return Point(p[X] + v[X], p[Y] + v[Y], p[Z] + v[Z]);
}

inline Point operator-(const Point& p, const Vector& v) {
  return Point(p[X] - v[X], p[Y] - v[Y], p[Z] - v[Z]);
}

inline Vector operator-(const Point& p1, const Point& p2) {
  return Vector(p1[X] - p2[X], p1[Y] - p2[Y], p1[Z] - p2[Z]);
}

inline bool operator<(const Point& p1, const Point& p2) {
  return p1[X] < p2[X] && p1[Y] < p2[Y] && p1[Z] < p2[Z];
}

inline Point inf(const Point& p1, const Point& p2) { 
  return Point(min(p1[X], p2[X]), min(p1[Y], p2[Y]), min(p1[Z], p2[Z]));
}

inline Point sup(const Point& p1, const Point& p2) { 
  return Point(max(p1[X], p2[X]), max(p1[Y], p2[Y]), max(p1[Z], p2[Z]));
}

inline Point affine(const Point& p1, const Point& p2, Scalar t) {
  return Point(p1[X] + (p2[X] - p1[X]) * t,
	       p1[Y] + (p2[Y] - p1[Y]) * t,
	       p1[Z] + (p2[Z] - p1[Z]) * t);
}

inline Scalar distance(const Point& p1, const Point& p2) { 
  return length(p1 - p2); 
}

inline Scalar distance2(const Point& p1, const Point& p2) { 
  return length2(p1 - p2); 
}

#endif
