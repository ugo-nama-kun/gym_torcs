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

#include "Transform.h"

void Transform::setValue(const float m[16]) {
  basis.setValue(m);
  origin.setValue(&m[12]);
  type = AFFINE;
}

void Transform::setValue(const double m[16]) {
  basis.setValue(m);
  origin.setValue(&m[12]);
  type = AFFINE;
}

Transform& Transform::operator*=(const Transform& t) {
  origin += basis * t.origin;
  basis *= t.basis;
  type |= t.type; 
  return *this;
}

void Transform::translate(const Vector& v) { 
  origin += basis * v; 
  type |= TRANSLATION;
}

void Transform::rotate(const Quaternion& q) { 
  basis *= Matrix(q); 
  type |= ROTATION; 
}

void Transform::scale(Scalar x, Scalar y, Scalar z) { 
  basis *= Matrix(x, y, z);  
  type |= SCALING;
}

void Transform::setIdentity() {
  basis.setIdentity();
  origin.setValue(0, 0, 0);
  type = IDENTITY;
}

void Transform::invert(const Transform& t) {
  basis = t.type & SCALING ? inverse(t.basis) : transpose(t.basis);
  origin.setValue(-dot(basis[X], t.origin), 
		  -dot(basis[Y], t.origin), 
		  -dot(basis[Z], t.origin));  
  type = t.type;
}

void Transform::mult(const Transform& t1, const Transform& t2) {
  basis = t1.basis * t2.basis;
  origin = t1(t2.origin);
  type = t1.type | t2.type;
}

void Transform::multInverseLeft(const Transform& t1, const Transform& t2) {
  Vector v = t2.origin - t1.origin;
  if (t1.type & SCALING) {
    Matrix inv = inverse(t1.basis);
    basis = inv * t2.basis;
    origin = inv * v;
  }
  else {
    basis = multTransposeLeft(t1.basis, t2.basis);
    origin = v * t1.basis;
  }
  type = t1.type | t2.type;
}
