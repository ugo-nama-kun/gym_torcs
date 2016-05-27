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

#include "BBoxTree.h"
#include "Transform.h"

#include <algorithm>
#include <new>

class BBoxCompAxis {
public:
  int axis;
  bool operator()(const BBoxNode& p1, const BBoxNode& p2) const {
    return p1.bbox.getCenter()[axis] < p2.bbox.getCenter()[axis];
  }
  BBoxCompAxis(int a) : axis(a) {}
} bboxCompAxis[3] = { X, Y, Z };


extern BBoxInternal *free_node; 

void BBoxLeaf::fitBBox() { 
  bbox.setEmpty();
  for (int i = 0; i < poly->numVerts(); ++i) {
    bbox.include((*poly)[i]);
  }
}

BBoxInternal::BBoxInternal(int n, BBoxLeaf *l) {
  tag = INTERNAL;
  bbox.setEmpty();
  for (int j = 0; j < n; ++j) {
    bbox.include(l[j].bbox);
  }

  int axis = bbox.longestAxis();
  int i = 0, mid = n;
  while (i < mid) {
    if (l[i].bbox.getCenter()[axis] < bbox.getCenter()[axis]) ++i;
    else swap(l[i], l[--mid]);
  }
  if (mid == 0 || mid == n) mid = n / 2;
  
  if (mid >= 2) {
    rson = free_node;
    new(free_node++) BBoxInternal(mid, &l[0]);
  }
  else rson = &l[0];
  if (n - mid >= 2) {
    lson = free_node;
    new(free_node++) BBoxInternal(n - mid, &l[mid]);
  } 
  else lson = &l[mid];
}

#ifdef STATISTICS
int num_box_tests = 0;
#endif

inline bool sep_axes_test(const Vector& a, const Vector& b, 
			  const Matrix& abs_b2a, const Vector& pos_b2a,
			  const Matrix& abs_a2b, const Vector& pos_a2b) {  
#ifdef STATISTICS
  ++num_box_tests;
#endif

  if (a[X] + dot(abs_b2a[X], b) < fabs(pos_b2a[X])) return false; 
  if (a[Y] + dot(abs_b2a[Y], b) < fabs(pos_b2a[Y])) return false; 
  if (a[Z] + dot(abs_b2a[Z], b) < fabs(pos_b2a[Z])) return false; 

  if (b[X] + dot(abs_a2b[X], a) < fabs(pos_a2b[X])) return false; 
  if (b[Y] + dot(abs_a2b[Y], a) < fabs(pos_a2b[Y])) return false; 
  if (b[Z] + dot(abs_a2b[Z], a) < fabs(pos_a2b[Z])) return false; 

  return true;
}

inline bool intersect(const BBox& a, const BBox& b, 
		      const Transform& b2a, const Matrix& abs_b2a,
		      const Transform& a2b, const Matrix& abs_a2b) {
  return sep_axes_test(a.getExtent(), b.getExtent(), 
		       abs_b2a, b2a(b.getCenter()) - a.getCenter(), 
		       abs_a2b, a2b(a.getCenter()) - b.getCenter());
}



bool intersect(const BBoxNode *tree, const Convex& c, const BBox& bb, 
	       const Transform& b2a, Vector& v) {
  if (!intersect(tree->bbox, bb)) return false;
  
  if (tree->tag == BBoxNode::LEAF) 
    return intersect(*((const BBoxLeaf *)tree)->poly, c, b2a, v); 
  else {
    return intersect(((const BBoxInternal *)tree)->lson, c, bb, b2a, v) ||
      intersect(((const BBoxInternal *)tree)->rson, c, bb, b2a, v);
  }
}


bool intersect(const BBoxNode *a, const BBoxNode *b,
	       const Transform& b2a, const Matrix& abs_b2a,
	       const Transform& a2b, const Matrix& abs_a2b, Vector& v) { 
  if (!intersect(a->bbox, b->bbox, b2a, abs_b2a, a2b, abs_a2b)) return false;
  
  if (a->tag == BBoxNode::LEAF && b->tag == BBoxNode::LEAF) {
    return intersect(*((const BBoxLeaf *)a)->poly, 
		     *((const BBoxLeaf *)b)->poly, b2a, v);
  }
  else if (a->tag == BBoxNode::LEAF || 
	   (b->tag != BBoxNode::LEAF && a->bbox.size() < b->bbox.size())) {
    return 
      intersect(a, ((const BBoxInternal *)b)->lson, 
		b2a, abs_b2a, a2b, abs_a2b, v) ||
      intersect(a, ((const BBoxInternal *)b)->rson, 
		b2a, abs_b2a, a2b, abs_a2b, v);
  }
  else {
    return 
      intersect(((const BBoxInternal *)a)->lson, b, b2a, abs_b2a, a2b, abs_a2b, v) ||
      intersect(((const BBoxInternal *)a)->rson, b, b2a, abs_b2a, a2b, abs_a2b, v);
  }
}



bool find_prim(const BBoxNode *tree, const Convex& c, const BBox& bb, 
	       const Transform& b2a, Vector& v, ShapePtr& p) {
  if (!intersect(tree->bbox, bb)) return false;
  
  if (tree->tag == BBoxNode::LEAF) {
    if  (intersect(*((const BBoxLeaf *)tree)->poly, c, b2a, v)) {
      p = ((const BBoxLeaf *)tree)->poly;
      return true;
    }
    else return false;
  }
  else {
    return find_prim(((const BBoxInternal *)tree)->lson, c, bb, b2a, v, p) ||
      find_prim(((const BBoxInternal *)tree)->rson, c, bb, b2a, v, p);
  }
}


bool find_prim(const BBoxNode *a, const BBoxNode *b,
	       const Transform& b2a, const Matrix& abs_b2a,
	       const Transform& a2b, const Matrix& abs_a2b,
	       Vector& v, ShapePtr& pa, ShapePtr& pb) { 
  if (!intersect(a->bbox, b->bbox, b2a, abs_b2a, a2b, abs_a2b)) return false;
  
  if (a->tag == BBoxNode::LEAF && b->tag == BBoxNode::LEAF) {
    if (intersect(*((const BBoxLeaf *)a)->poly, 
		  *((const BBoxLeaf *)b)->poly, b2a, v)) {
      pa = ((const BBoxLeaf *)a)->poly;
      pb = ((const BBoxLeaf *)b)->poly;
      return true;
    }
    else return false;
  }
  else if (a->tag == BBoxNode::LEAF || 
	   (b->tag != BBoxNode::LEAF && a->bbox.size() < b->bbox.size())) {
    return 
      find_prim(a, ((const BBoxInternal *)b)->lson, 
		b2a, abs_b2a, a2b, abs_a2b, v, pa, pb) ||
      find_prim(a, ((const BBoxInternal *)b)->rson, 
		b2a, abs_b2a, a2b, abs_a2b, v, pa, pb);
  }
  else {
    return 
      find_prim(((const BBoxInternal *)a)->lson, b, b2a, abs_b2a, a2b, abs_a2b, v, pa, pb) ||
      find_prim(((const BBoxInternal *)a)->rson, b, b2a, abs_b2a, a2b, abs_a2b, v, pa, pb);
  }
}



bool common_point(const BBoxNode *tree, const Convex& c, const BBox& bb, 
		  const Transform& b2a, Vector& v, Point& pa, Point& pb) {
  if (!intersect(tree->bbox, bb)) return false;
  
  if (tree->tag == BBoxNode::LEAF) 
    return common_point(*((const BBoxLeaf *)tree)->poly, c, b2a, v, pa, pb); 
  else {
    return common_point(((const BBoxInternal *)tree)->lson, c, bb, b2a, v, pa, pb) ||
      common_point(((const BBoxInternal *)tree)->rson, c, bb, b2a, v, pa, pb);
  }
}


bool common_point(const BBoxNode *a, const BBoxNode *b,
		  const Transform& b2a, const Matrix& abs_b2a,
		  const Transform& a2b, const Matrix& abs_a2b,
		  Vector& v, Point& pa, Point& pb) { 
  if (!intersect(a->bbox, b->bbox, b2a, abs_b2a, a2b, abs_a2b)) return false;
  
  if (a->tag == BBoxNode::LEAF && b->tag == BBoxNode::LEAF) {
    return common_point(*((const BBoxLeaf *)a)->poly, 
			*((const BBoxLeaf *)b)->poly, b2a, v, pa, pb);
  }
  else if (a->tag == BBoxNode::LEAF || 
	   (b->tag != BBoxNode::LEAF && a->bbox.size() < b->bbox.size())) {
    return 
      common_point(a, ((const BBoxInternal *)b)->lson, 
		   b2a, abs_b2a, a2b, abs_a2b, v, pa, pb) ||
      common_point(a, ((const BBoxInternal *)b)->rson, 
		   b2a, abs_b2a, a2b, abs_a2b, v, pa, pb);
  }
  else {
    return 
      common_point(((const BBoxInternal *)a)->lson, b, b2a, abs_b2a, a2b, abs_a2b, v, pa, pb) ||
      common_point(((const BBoxInternal *)a)->rson, b, b2a, abs_b2a, a2b, abs_a2b, v, pa ,pb);
  }
}




