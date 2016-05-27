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

#ifndef _OBJECT_H_
#define _OBJECT_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <SOLID/solid.h>

#include "Transform.h"
#include "BBox.h"
#include "Endpoint.h"
#include "Shape.h"

class Object {
public:
  Object(DtObjectRef obj, ShapePtr shape); 
  
  void move();
  void proceed();

  void translate(const Vector& v) { curr.translate(v); }
  void rotate(const Quaternion& q) { curr.rotate(q); }
  void scale(Scalar x, Scalar y, Scalar z) { curr.scale(x, y, z); }
  
  void setIdentity() { curr.setIdentity(); }

  void setMatrix(const float v[16]) { curr.setValue(v); }
  void setMatrix(const double v[16]) { curr.setValue(v); }

  void multMatrix(const float v[16]) { curr *= Transform(v); }
  void multMatrix(const double v[16]) { curr *= Transform(v); }

  const BBox& getBBox() const { return bbox; }

  Transform curr;
  Transform prev;
  DtObjectRef ref;
  ShapePtr shapePtr;
  BBox bbox;
  Endpoint lower[3];
  Endpoint upper[3];
};

bool intersect(const Object&, const Object&, Vector& v);
bool common_point(const Object&, const Object&, Vector&, Point&, Point&);
bool prev_closest_points(const Object&, const Object&, 
			 Vector&, Point&, Point&);
#endif





