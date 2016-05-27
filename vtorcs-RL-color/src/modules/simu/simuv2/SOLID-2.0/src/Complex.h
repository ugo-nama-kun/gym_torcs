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

#ifndef _COMPLEX_H_
#define _COMPLEX_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <algorithm>

#include "Shape.h"
#include "VertexBase.h"

class BBoxNode;
class BBoxLeaf;

class Polytope;
class Convex;
class Transform;
class BBox;

class Complex : public Shape {
public:
  Complex() {}
  ~Complex();

  ShapeType getType() const { return COMPLEX; } 
  BBox bbox(const Transform& t) const;

  const VertexBase& getBase() const { return base; } 
  void setBase(const void *ptr, bool free = false) { 
    base = ptr; free_base = free; 
  } 
  void changeBase(const void *ptr);
  void proceed() { prev_base = base; }
  void swapBase() { swap(base, prev_base); } 

  void finish(int n, const Polytope **p);

  friend bool intersect(const Complex& a, const Convex& b,
			const Transform& a2w, const Transform& b2w,
			Vector& v);
  
  friend bool intersect(const Complex& a, const Complex& b,
			const Transform& a2w, const Transform& b2w,
			Vector& v);
  
  friend bool find_prim(const Complex& a, const Convex& b, 
			const Transform& a2w, const Transform& b2w,
			Vector& v, ShapePtr& p);
  
  friend bool find_prim(const Complex& a, const Complex& b,
			const Transform& a2w, const Transform& b2w,
			Vector& v, ShapePtr& pa, ShapePtr& pb);
  
  friend  bool common_point(const Complex& a, const Convex& b,
			    const Transform& a2w, const Transform& b2w,
			    Vector& v, Point& pa, Point& pb);
  
  friend bool common_point(const Complex& a, const Complex& b,
			   const Transform& a2w, const Transform& b2w,
			   Vector& v, Point& pa, Point& pb);
			
private:
  VertexBase base;
  VertexBase prev_base;
  bool free_base;
  BBoxLeaf *leaves;
  BBoxNode *root;
  int count;
};

#endif
