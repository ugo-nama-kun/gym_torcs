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

#ifndef _BBOXTREE_H_
#define _BBOXTREE_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include "BBox.h"
#include "Polytope.h"

class Convex;
class Transform;
class Matrix;

class BBoxNode {
public:
  enum TagType { LEAF, INTERNAL };

  BBox bbox;

  TagType tag;
};

class BBoxLeaf : public BBoxNode {
public:
  const Polytope *poly;

  BBoxLeaf() {}
  BBoxLeaf(const Polytope *p) : poly(p) { 
    tag = LEAF;
    fitBBox(); 
  }

  void fitBBox();
};

class BBoxInternal : public BBoxNode {
public:
  BBoxNode *lson;
  BBoxNode *rson;

  BBoxInternal() {}
  BBoxInternal(int n, BBoxLeaf *l);

  void refitBBox() { bbox.enclose(lson->bbox, rson->bbox); }
};



bool intersect(const BBoxNode *tree, const Convex& c, const BBox& bb,
	       const Transform& b2a, Vector& v);

bool intersect(const BBoxNode *a, const BBoxNode *b,
	       const Transform& b2a, const Matrix& abs_b2a,
	       const Transform& a2b, const Matrix& abs_a2b, Vector& v);

bool find_prim(const BBoxNode *tree, const Convex& c, const BBox& bb,
	       const Transform& b2a, Vector& v, ShapePtr& p);

bool find_prim(const BBoxNode *a, const BBoxNode *b,
	       const Transform& b2a, const Matrix& abs_b2a,
	       const Transform& a2b, const Matrix& abs_a2b,
	       Vector& v, ShapePtr& pa, ShapePtr& pb);

bool common_point(const BBoxNode *tree, const Convex& c, const BBox& bb,
		  const Transform& b2a, Vector& v, Point& pa, Point& pb);

bool common_point(const BBoxNode *a, const BBoxNode *b,
		  const Transform& b2a, const Matrix& abs_b2a,
		  const Transform& a2b, const Matrix& abs_a2b,
		  Vector& v, Point& pa, Point& pb);

#endif






