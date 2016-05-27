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

#include <SOLID/solid.h>

#include <algorithm>

#include "Box.h"
#include "Cone.h"
#include "Cylinder.h"
#include "Sphere.h"
#include "Complex.h"
#include "Encounter.h"
#include "Object.h"
#include "Simplex.h"
#include "Polygon.h"
#include "Polyhedron.h"
#include "Response.h"
#include "RespTable.h"
#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
#  include <sys/types.h>
#endif

#include <map>
#include <set>
#include <vector>

typedef vector<Point> PointBuf;
typedef vector<unsigned int> IndexBuf;
typedef vector<const Polytope *> PolyList;
typedef vector<Complex *> ComplexList;
typedef map<DtObjectRef, Object *> ObjectList;
typedef set<Encounter> ProxList;

#ifdef WIN32
#define uint unsigned int
#endif

PointBuf pointBuf;
IndexBuf indexBuf;
PolyList polyList; 
ComplexList complexList;
ObjectList objectList;
RespTable respTable;
ProxList proxList;

DtPolyType currentType;
Complex *currentComplex = 0;
Object *currentObject = 0;
bool caching = true;

extern Scalar rel_error;

inline void move() { if (caching && currentObject) currentObject->move(); }


DtShapeRef dtBox(DtScalar x, DtScalar y, DtScalar z) {
  return new Box(x, y, z);
}

DtShapeRef dtCone(DtScalar radius, DtScalar height) {
  return new Cone(radius, height);
}

DtShapeRef dtCylinder(DtScalar radius, DtScalar height) {
  return new Cylinder(radius, height);
}

DtShapeRef dtSphere(DtScalar radius) {
  return new Sphere(radius);
}

DtShapeRef dtNewComplexShape() {
  if (!currentComplex) currentComplex = new Complex;
  return currentComplex;
}

void dtEndComplexShape() {
  if (currentComplex->getBase().getPointer() == 0) {
    Point *ptr = new Point[pointBuf.size()];
    copy(pointBuf.begin(), pointBuf.end(), ptr);
    currentComplex->setBase(ptr, true);
    pointBuf.erase(pointBuf.begin(), pointBuf.end());
  }
  currentComplex->finish(polyList.size(), &polyList[0]);
  polyList.erase(polyList.begin(), polyList.end());
  complexList.push_back(currentComplex);
  currentComplex = 0;
}

void dtBegin(DtPolyType type) { currentType = type; }
void dtEnd() { 
  dtVertexIndices(currentType, indexBuf.size(), &indexBuf[0]);
  indexBuf.erase(indexBuf.begin(), indexBuf.end());
}

void dtVertex(DtScalar x, DtScalar y, DtScalar z) {
  Point p(x, y, z);
  int i = pointBuf.size()-20;
  if (i < 0) i = 0;
  while ((uint)i < pointBuf.size() && !(pointBuf[i] == p)) ++i;
  if ((uint)i == pointBuf.size()) pointBuf.push_back(p);
  indexBuf.push_back(i);
}

void dtVertexBase(const void *base) { 
  currentComplex->setBase(base); 
}

void dtVertexIndex(DtIndex index) { indexBuf.push_back(index); }

void dtVertexIndices(DtPolyType type, DtCount count, 
			     const DtIndex *indices) {
  if (currentComplex) {
    const Polytope *poly;
    switch (type) {
    case DT_SIMPLEX:
      poly = new Simplex(currentComplex->getBase(), count, indices);
      break;
    case DT_POLYGON:
      poly = new Polygon(currentComplex->getBase(), count, indices);
      break;
    case DT_POLYHEDRON:
      if (currentComplex->getBase().getPointer() == 0) {
	currentComplex->setBase(&pointBuf[0]);
	poly = new Polyhedron(currentComplex->getBase(), count, indices);
	currentComplex->setBase(0);
      }
      else poly = new Polyhedron(currentComplex->getBase(), count, indices);
      break;
    }
    polyList.push_back(poly);
  }
}

void dtVertexRange(DtPolyType type, DtIndex first, DtCount count) {
  DtIndex *indices = new DtIndex[count];
  for (uint i = 0; i < count; ++i) indices[i] = first + i;
  dtVertexIndices(type, count, indices);
  delete [] indices;
}

void dtDeleteShape(DtShapeRef shape) { 
  if (((Shape *)shape)->getType() == COMPLEX) {
    ComplexList::iterator i = 
      find(complexList.begin(), complexList.end(), (Complex *)shape);
    if (i != complexList.end()) complexList.erase(i);
  }
  delete (Shape *)shape; 
}

void dtChangeVertexBase(DtShapeRef shape, const void *base) { 
  if (((Shape *)shape)->getType() == COMPLEX)
    ((Complex *)shape)->changeBase(base);
  for (ObjectList::const_iterator i = objectList.begin(); 
       i != objectList.end(); ++i) {
    if ((*i).second->shapePtr == (Shape *)shape) {
      (*i).second->move();
    }
  }   
}


// Object instantiation

void dtCreateObject(DtObjectRef object, DtShapeRef shape) {
  move();
  currentObject = objectList[object] = new Object(object, (Shape *)shape); 
}

void dtSelectObject(DtObjectRef object) {
  ObjectList::iterator i = objectList.find(object);
  if (i != objectList.end()) {
    move();
    currentObject = (*i).second;
  }
}

void dtDeleteObject(DtObjectRef object) {
  ObjectList::iterator i = objectList.find(object);
  if (i != objectList.end()) {
    if (currentObject == (*i).second) currentObject = 0;
    delete (*i).second;
    objectList.erase(i);
  }
  respTable.cleanObject(object);
}

void dtTranslate(DtScalar x, DtScalar y, DtScalar z) {
  if (currentObject) currentObject->translate(Vector(x, y, z));
}

void dtRotate(DtScalar x, DtScalar y, DtScalar z, DtScalar w) {
  if (currentObject) currentObject->rotate(Quaternion(x, y, z, w));
}

void dtScale(DtScalar x, DtScalar y, DtScalar z) {
  if (currentObject) currentObject->scale(x, y, z);
}

void dtLoadIdentity() { 
  if (currentObject) currentObject->setIdentity();
}

void dtLoadMatrixf(const float *m) { 
  if (currentObject) currentObject->setMatrix(m);
}

void dtLoadMatrixd(const double *m) { 
  if (currentObject) currentObject->setMatrix(m);
}

void dtMultMatrixf(const float *m) { 
  if (currentObject) currentObject->multMatrix(m);
}

void dtMultMatrixd(const double *m) { 
  if (currentObject) currentObject->multMatrix(m);
}

// Response

void dtSetDefaultResponse(DtResponse response, DtResponseType type,
				  void *client_data) {
  respTable.setDefault(Response(response, type, client_data));
}

void dtClearDefaultResponse() {
  respTable.setDefault(Response());
}

void dtSetObjectResponse(DtObjectRef object, DtResponse response, 
				 DtResponseType type, void *client_data) {
  respTable.setSingle(object, Response(response, type, client_data));
}

void dtClearObjectResponse(DtObjectRef object) {
  respTable.setSingle(object, Response());
}

void dtResetObjectResponse(DtObjectRef object) {
  respTable.resetSingle(object);
}

void dtSetPairResponse(DtObjectRef object1, DtObjectRef object2, 
			       DtResponse response, DtResponseType type, 
			       void * client_data) {
  respTable.setPair(object1, object2, Response(response, type, client_data));
}

void dtClearPairResponse(DtObjectRef object1, DtObjectRef object2) {
  respTable.setPair(object1, object2, Response());
}

void dtResetPairResponse(DtObjectRef object1, DtObjectRef object2) {
  respTable.resetPair(object1, object2);
}

// Runtime

void dtProceed() {
  for (ComplexList::iterator i = complexList.begin(); 
       i != complexList.end(); ++i) 
    (*i)->proceed();
  for (ObjectList::const_iterator j = objectList.begin(); 
       j != objectList.end(); ++j)
    (*j).second->proceed();
}

void dtEnableCaching() {
  for (ObjectList::const_iterator i = objectList.begin();
       i != objectList.end(); ++i)
    (*i).second->move();
  caching = true;
}

void dtDisableCaching() { caching = false; }

void dtSetTolerance(DtScalar tol) { rel_error = tol; }

void addPair(ObjectPtr object1, ObjectPtr object2) {
  proxList.insert(Encounter(object1, object2));
}

void removePair(ObjectPtr object1, ObjectPtr object2) {
  proxList.erase(Encounter(object1, object2)); 
}

bool object_test(Encounter& e) {
  static Point p1, p2;
  const Response& resp = respTable.find(e.obj1->ref, e.obj2->ref);
  switch (resp.type) {
  case DT_SIMPLE_RESPONSE:
    if (intersect(*e.obj1, *e.obj2, e.sep_axis)) { 
      resp(e.obj1->ref, e.obj2->ref);
      return true; 
    }
    break;
  case DT_SMART_RESPONSE:
    if (prev_closest_points(*e.obj1, *e.obj2, e.sep_axis, p1, p2)) {
      Vector v = e.obj1->prev(p1) - e.obj2->prev(p2);
      resp(e.obj1->ref, e.obj2->ref, p1, p2, v);
      return true; 
    }
    break;
  case DT_WITNESSED_RESPONSE:
    if (common_point(*e.obj1, *e.obj2, e.sep_axis, p1, p2)) { 
      resp(e.obj1->ref, e.obj2->ref, p1, p2, Vector(0, 0, 0));
      return true; 
    }
    break;
  // Eric Espie: warning
  case DT_NO_RESPONSE:
    break;
  }
  return false;
}

DtCount dtTest() {
  move();
  DtCount count = 0;
  if (caching) {
    for (ProxList::iterator i = proxList.begin(); i != proxList.end(); ++i) 
      if (object_test((Encounter &)*i)) ++count;
  }
  else {
    for (ObjectList::const_iterator j = objectList.begin(); 
	 j != objectList.end(); ++j)
      for (ObjectList::const_iterator i = objectList.begin(); 
	   i != j; ++i) {
	Encounter e((*i).second, (*j).second);
	if (object_test(e)) ++count;
      } 
  }
  return count;
}
