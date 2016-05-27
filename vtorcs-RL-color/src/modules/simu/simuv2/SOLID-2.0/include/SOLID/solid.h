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

#ifndef _SOLID_H_
#define _SOLID_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef double DtScalar;
typedef DtScalar DtVector[3];
typedef unsigned int DtIndex;
typedef unsigned int DtCount;

typedef void *DtObjectRef;
typedef void *DtShapeRef;

typedef enum DtPolyType {
  DT_SIMPLEX,
  DT_POLYGON,
  DT_POLYHEDRON
} DtPolyType;

typedef enum DtResponseType { 
  DT_NO_RESPONSE,
  DT_SIMPLE_RESPONSE,
  DT_SMART_RESPONSE,
  DT_WITNESSED_RESPONSE
} DtResponseType;

typedef struct DtCollData {
  DtVector point1;
  DtVector point2;
  DtVector normal;
} DtCollData;

typedef void (*DtResponse)(
  void *client_data,
  DtObjectRef object1,
  DtObjectRef object2,
  const DtCollData *coll_data);



/* Shape definition */

extern DtShapeRef dtBox(DtScalar x, DtScalar y, DtScalar z);
extern DtShapeRef dtCone(DtScalar radius, DtScalar height);
extern DtShapeRef dtCylinder(DtScalar radius, DtScalar height);
extern DtShapeRef dtSphere(DtScalar radius);

extern DtShapeRef dtNewComplexShape();
extern void dtEndComplexShape();

extern void dtBegin(DtPolyType type);
extern void dtEnd();

extern void dtVertex(DtScalar x, DtScalar y, DtScalar z);
extern void dtVertexBase(const void *base);
extern void dtVertexIndex(DtIndex index);
extern void dtVertexIndices(DtPolyType type, DtCount count, 
			    const DtIndex *indices);
extern void dtVertexRange(DtPolyType type, DtIndex first, DtCount count); 

extern void dtChangeVertexBase(DtShapeRef shape, const void *base);

extern void dtDeleteShape(DtShapeRef shape);


/* Object  */

extern void dtCreateObject(DtObjectRef object, DtShapeRef shape); 
extern void dtDeleteObject(DtObjectRef object);
extern void dtSelectObject(DtObjectRef object);

extern void dtLoadIdentity();

extern void dtLoadMatrixf(const float *m);
extern void dtLoadMatrixd(const double *m);

extern void dtMultMatrixf(const float *m);
extern void dtMultMatrixd(const double *m);

extern void dtTranslate(DtScalar x, DtScalar y, DtScalar z);
extern void dtRotate(DtScalar x, DtScalar y, DtScalar z, DtScalar w);
extern void dtScale(DtScalar x, DtScalar y, DtScalar z);


/* Response */

extern void dtSetDefaultResponse(DtResponse response, DtResponseType type, 
				 void *client_data);

extern void dtClearDefaultResponse();

extern void dtSetObjectResponse(DtObjectRef object, DtResponse response, 
				DtResponseType type, void *client_data);
extern void dtClearObjectResponse(DtObjectRef object);
extern void dtResetObjectResponse(DtObjectRef object);

extern void dtSetPairResponse(DtObjectRef object1, DtObjectRef object2, 
			      DtResponse response, DtResponseType type, 
			      void *client_data);
extern void dtClearPairResponse(DtObjectRef object1, DtObjectRef object2);
extern void dtResetPairResponse(DtObjectRef object1, DtObjectRef object2);


/* Global */

extern DtCount dtTest();
extern void dtProceed();

extern void dtEnableCaching();
extern void dtDisableCaching();

extern void dtSetTolerance(DtScalar tol);

#ifdef __cplusplus
}
#endif

#endif
