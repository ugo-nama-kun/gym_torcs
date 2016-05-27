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

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include "Object.h"
#include "AlgoTable.h"
#include "Convex.h" 
#include "Complex.h" 

#include <new>

Object::Object(DtObjectRef obj, ShapePtr shape) : 
  ref(obj), 
  shapePtr(shape),
  bbox(Point(INFINITY_, INFINITY_, INFINITY_), 
       Point(INFINITY_, INFINITY_, INFINITY_))
{
  curr.setIdentity();
  new (&lower[X]) Endpoint(X, MIN, this);    
  new (&lower[Y]) Endpoint(Y, MIN, this);    
  new (&lower[Z]) Endpoint(Z, MIN, this);    
  new (&upper[X]) Endpoint(X, MAX, this);
  new (&upper[Y]) Endpoint(Y, MAX, this);
  new (&upper[Z]) Endpoint(Z, MAX, this);
  proceed();
}

void Object::move() {
  bbox = shapePtr->bbox(curr);
  lower[X].move(bbox.getLower(X));
  lower[Y].move(bbox.getLower(Y));
  lower[Z].move(bbox.getLower(Z));
  upper[X].move(bbox.getUpper(X));
  upper[Y].move(bbox.getUpper(Y));
  upper[Z].move(bbox.getUpper(Z));
}

void Object::proceed() {
  prev = curr;
}

typedef AlgoTable<Intersect> IntersectTable;
typedef AlgoTable<Common_point> Common_pointTable;


bool intersectConvexConvex(const Shape& a, const Shape& b, 
			   const Transform& a2w, const Transform& b2w,
			   Vector& v) {
  return intersect((const Convex&)a, (const Convex&)b, a2w, b2w, v);
}

bool intersectComplexConvex(const Shape& a, const Shape& b, 
			    const Transform& a2w, const Transform& b2w,
			    Vector& v) {
  return intersect((const Complex&)a, (const Convex&)b, a2w, b2w, v);
}

bool intersectComplexComplex(const Shape& a, const Shape& b, 
			     const Transform& a2w, const Transform& b2w,
			     Vector& v) {
  return intersect((const Complex&)a, (const Complex&)b, a2w, b2w, v);
}

IntersectTable *intersectInitialize() {
  IntersectTable *p = new IntersectTable;
  p->addEntry(CONVEX, CONVEX, intersectConvexConvex);
  p->addEntry(COMPLEX, CONVEX, intersectComplexConvex);
  p->addEntry(COMPLEX, COMPLEX, intersectComplexComplex);
  return p;
}

bool intersect(const Object& a, const Object& b, Vector& v) {
  static IntersectTable *intersectTable = intersectInitialize();
  Intersect intersect = 
    intersectTable->lookup(a.shapePtr->getType(), b.shapePtr->getType());
  return intersect(*a.shapePtr, *b.shapePtr, a.curr, b.curr, v);
}

bool common_pointConvexConvex(const Shape& a, const Shape& b, 
			      const Transform& a2w, const Transform& b2w,
			      Vector& v, Point& pa, Point& pb) {
  return common_point((const Convex&)a, (const Convex&)b, a2w, b2w, v, pa, pb);
}

bool common_pointComplexConvex(const Shape& a, const Shape& b, 
			       const Transform& a2w, const Transform& b2w,
			       Vector& v, Point& pa, Point& pb) {
  return common_point((const Complex&)a, (const Convex&)b, a2w, b2w, v, pa, pb);
}

bool common_pointComplexComplex(const Shape& a, const Shape& b, 
				const Transform& a2w, const Transform& b2w,
				Vector& v, Point& pa, Point& pb) {
  return common_point((const Complex&)a, (const Complex&)b, a2w, b2w, v, pa, pb);
}

Common_pointTable *common_pointInitialize() {
  Common_pointTable *p = new Common_pointTable;
  p->addEntry(CONVEX, CONVEX, common_pointConvexConvex);
  p->addEntry(COMPLEX, CONVEX, common_pointComplexConvex);
  p->addEntry(COMPLEX, COMPLEX, common_pointComplexComplex);
  return p;
}

bool common_point(const Object& a, const Object& b, Vector& v, Point& pa, Point& pb) {
  static Common_pointTable *common_pointTable = common_pointInitialize();
  Common_point common_point = 
    common_pointTable->lookup(a.shapePtr->getType(), b.shapePtr->getType());
  return common_point(*a.shapePtr, *b.shapePtr, a.curr, b.curr, v, pa, pb);
}

bool prev_closest_points(const Object& a, const Object& b, 
			 Vector& v, Point& pa, Point& pb) {
  ShapePtr sa, sb;  
  if (a.shapePtr->getType() == COMPLEX) {
    if (b.shapePtr->getType() == COMPLEX) {
      if (!find_prim((const Complex&)*a.shapePtr, (const Complex&)*b.shapePtr, 
		     a.curr, b.curr, v, sa, sb)) return false;
      ((Complex *)a.shapePtr)->swapBase();
      if (b.shapePtr != a.shapePtr) ((Complex *)b.shapePtr)->swapBase();
      closest_points((const Convex&)*sa, (const Convex&)*sb, a.prev, b.prev, pa, pb); 
      ((Complex *)a.shapePtr)->swapBase();
      if (b.shapePtr != a.shapePtr) ((Complex *)b.shapePtr)->swapBase();
    }
    else {
      if (!find_prim((const Complex&)*a.shapePtr, (const Convex&)*b.shapePtr,  
		a.curr, b.curr, v, sa)) return false;
      ((Complex *)a.shapePtr)->swapBase();
      closest_points((const Convex&)*sa, (const Convex&)*b.shapePtr, a.prev, b.prev, pa, pb);
      ((Complex *)a.shapePtr)->swapBase();
    }
  }
  else {
    if (!intersect(a, b, v)) return false;
    closest_points((const Convex&)*a.shapePtr, (const Convex&)*b.shapePtr, a.prev, b.prev, pa, pb);
  } 
  
  return true;
}
