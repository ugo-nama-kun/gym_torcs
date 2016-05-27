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

#include "Complex.h"
#include "Transform.h"
#include "BBoxTree.h"

#include <new>

BBoxInternal *free_node;

Complex::~Complex() {
  if (count >= 2) delete [] root;
  for (int i = 0; i < count; ++i) delete leaves[i].poly;
  delete [] leaves;
  /* if (free_base) delete [] (void *)base.getPointer(); */  /* warning: deleting `void*' is undefined */
}

BBox Complex::bbox(const Transform& t) const {
  Matrix abs_b = absolute(t.getBasis());  
  BBox bb;
  bb.setCenter(t(root->bbox.getCenter()));
  bb.setExtent(Vector(dot(abs_b[X], root->bbox.getExtent()),
		      dot(abs_b[Y], root->bbox.getExtent()),
		      dot(abs_b[Z], root->bbox.getExtent())));
  return bb;
}

void Complex::changeBase(const void *ptr) {
  base = ptr;
  for (int i = 0; i < count; ++i) leaves[i].fitBBox();
  for (int j = count-1; j;) ((BBoxInternal *)root)[--j].refitBBox(); 
}

void Complex::finish(int n, const Polytope **p) {
  proceed();
  leaves = new BBoxLeaf[n];
  count = n;
  for (int i = 0; i < n; ++i) new(&leaves[i]) BBoxLeaf(p[i]);
  if (n >= 2) {
    free_node = new BBoxInternal[n-1];
    root = free_node;
    new(free_node++) BBoxInternal(n, leaves);
  }
  else root = &leaves[0];
}


bool intersect(const Complex& a, const Convex& b, 
	       const Transform& a2w, const Transform& b2w,
	       Vector& v) {
  Transform b2a;
  b2a.multInverseLeft(a2w, b2w);
  BBox bb = b.bbox(b2a);
  return intersect(a.root, b, bb, b2a, v);
}

bool intersect(const Complex& a, const Complex& b, 
	       const Transform& a2w, const Transform& b2w,
	       Vector& v) {
  Transform b2a, a2b;
  b2a.multInverseLeft(a2w, b2w);
  a2b.invert(b2a);
  Matrix abs_b2a = absolute(b2a.getBasis());
  Matrix abs_a2b = absolute(a2b.getBasis());
  return intersect(a.root, b.root, b2a, abs_b2a, a2b, abs_a2b, v);
}

bool find_prim(const Complex& a, const Convex& b, 
	       const Transform& a2w, const Transform& b2w,
	       Vector& v, ShapePtr& p) {
  Transform b2a;
  b2a.multInverseLeft(a2w, b2w);
  BBox bb = b.bbox(b2a);
  return find_prim(a.root, b, bb, b2a, v, p);
}

bool find_prim(const Complex& a, const Complex& b, 
	       const Transform& a2w, const Transform& b2w,
	       Vector& v, ShapePtr& pa, ShapePtr& pb) {
  Transform b2a, a2b;
  b2a.multInverseLeft(a2w, b2w);
  a2b.invert(b2a);
  Matrix abs_b2a = absolute(b2a.getBasis());
  Matrix abs_a2b = absolute(a2b.getBasis());
  return find_prim(a.root, b.root, b2a, abs_b2a, a2b, abs_a2b, v, pa, pb);
}

bool common_point(const Complex& a, const Convex& b, 
		  const Transform& a2w, const Transform& b2w,
		  Vector& v, Point& pa, Point& pb) {
  Transform b2a;
  b2a.multInverseLeft(a2w, b2w);
  BBox bb = b.bbox(b2a);
  return common_point(a.root, b, bb, b2a, v, pb, pa);
}

bool common_point(const Complex& a, const Complex& b, 
		  const Transform& a2w, const Transform& b2w,
		  Vector& v, Point& pa, Point& pb) {
  Transform b2a, a2b;
  b2a.multInverseLeft(a2w, b2w);
  a2b.invert(b2a);
  Matrix abs_b2a = absolute(b2a.getBasis());
  Matrix abs_a2b = absolute(a2b.getBasis());
  return common_point(a.root, b.root, b2a, abs_b2a, a2b, abs_a2b, v, pa, pb);
}
