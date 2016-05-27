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

#ifndef _CONVEX_H_
#define _CONVEX_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <3D/Point.h>
#include "Shape.h"
#include "BBox.h"
#include "Transform.h"

class Convex : public Shape {
public:
  ShapeType getType() const { return CONVEX; } 

  virtual ~Convex() {}
  virtual Point support(const Vector& v) const = 0;
  virtual BBox bbox(const Transform& t) const;
};


bool intersect(const Convex& a, const Convex& b,
	       const Transform& a2w, const Transform& b2w,
	       Vector& v);

bool intersect(const Convex& a, const Convex& b, const Transform& b2a, 
	       Vector& v);
			
bool common_point(const Convex& a, const Convex& b, 
		  const Transform& a2w, const Transform& b2w,
		  Vector& v, Point& pa, Point& pb);

bool common_point(const Convex& a, const Convex& b, const Transform& b2a,
		  Vector& v, Point& pa, Point& pb);

void closest_points(const Convex&, const Convex&, 
		    const Transform&, const Transform&,
		    Point&, Point&);

#endif
