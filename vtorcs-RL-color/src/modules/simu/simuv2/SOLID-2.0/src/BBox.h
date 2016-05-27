/*
  SOLID - Software Library for Interference Detection
  Copyright (C) 1997-1998  Gino van den Bergen

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

#ifndef _BBOX_H_
#define _BBOX_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <3D/Point.h>

class BBox {
public:
  BBox() {} 
  BBox(const Point& min, const Point& max) { setValue(min, max); }

  const Point& getCenter() const { return center; }
  const Vector& getExtent() const { return extent; }
  
  void setCenter(const Point& p)  { center = p; }
  void setExtent(const Vector& v) { extent = v; }

  void setValue(const Point& min, const Point& max) { 
    extent = (max - min) / 2;
    center = min + extent;
  }

  void enclose(const BBox& a, const BBox& b) {
    Point lower(min(a.getLower(X), b.getLower(X)),
		min(a.getLower(Y), b.getLower(Y)),
		min(a.getLower(Z), b.getLower(Z)));
    Point upper(max(a.getUpper(X), b.getUpper(X)),
		max(a.getUpper(Y), b.getUpper(Y)),
		max(a.getUpper(Z), b.getUpper(Z)));
    setValue(lower, upper);
  }

  void setEmpty() { 
    center.setValue(0, 0, 0); 
    extent.setValue(-INFINITY_, -INFINITY_, -INFINITY_);
  }

  void include (const Point& p) {
    Point lower(min(getLower(X), p[X]),
		min(getLower(Y), p[Y]),
		min(getLower(Z), p[Z]));
    Point upper(max(getUpper(X), p[X]),
		max(getUpper(Y), p[Y]),
		max(getUpper(Z), p[Z]));
    setValue(lower, upper);
  }

  void include (const BBox& b) { enclose(*this, b); }

  Scalar getLower(int i) const { return center[i] - extent[i]; }
  Scalar getUpper(int i) const { return center[i] + extent[i]; }

  Scalar size() const { return max(max(extent[X], extent[Y]), extent[Z]); }
  int longestAxis() const { return extent.closestAxis(); }

  friend bool intersect(const BBox& a, const BBox& b);

private:
  Point center;
  Vector extent;
};

inline bool intersect(const BBox& a, const BBox& b) {
  return fabs(a.center[X] - b.center[X]) <= a.extent[X] + b.extent[X] &&
         fabs(a.center[Y] - b.center[Y]) <= a.extent[Y] + b.extent[Y] &&
	 fabs(a.center[Z] - b.center[Z]) <= a.extent[Z] + b.extent[Z];
}

#endif


