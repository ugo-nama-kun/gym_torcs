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

#ifndef _TRANSFORM_H_
#define _TRANSFORM_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <3D/Point.h>
#include <3D/Matrix.h>

class Transform {
public:
  Transform() {}
  Transform(const float m[16]) { setValue(m); }
  Transform(const double m[16]) { setValue(m); }
 
  Point operator()(const Point& p) const {
    return Point(dot(basis[X], p) + origin[X], 
		 dot(basis[Y], p) + origin[Y], 
		 dot(basis[Z], p) + origin[Z]);
  }
  
  const Matrix& getBasis() const { return basis; }
  const Point& getOrigin() const { return origin; }

  void setValue(const float m[16]);
  void setValue(const double m[16]);
  
  void setIdentity();

  Transform& operator*=(const Transform& t);

  void translate(const Vector& v);
  void rotate(const Quaternion& q);
  void scale(Scalar x, Scalar y, Scalar z);

  void invert(const Transform& t);
  void mult(const Transform& t1, const Transform& t2);
  void multInverseLeft(const Transform& t1, const Transform& t2); 

private:
  enum { 
    IDENTITY = 0x00, 
    TRANSLATION = 0x01,
    ROTATION = 0x02,
    SCALING = 0x04,
    LINEAR = ROTATION | SCALING,
    AFFINE = TRANSLATION | LINEAR
  };

  Matrix basis;
  Point origin;
  unsigned int type;
};

#endif
