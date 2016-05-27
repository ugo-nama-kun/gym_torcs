/*
  3D - C++ Class Library for 3D Transformations
  Copyright (C) 1996-1998  Gino van den Bergen

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

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "Vector.h"
#include "Quaternion.h"

typedef Scalar Mat3[3][3];

// Row-major 3x3 matrix

class Matrix {
public:
  Matrix() {}
  Matrix(const float *m) { setValue(m); }
  Matrix(const double *m) { setValue(m); }
  Matrix(const Quaternion& q) { setRotation(q); }
  Matrix(Scalar x, Scalar y, Scalar z) { setScaling(x, y, z); }
  Matrix(Scalar xx, Scalar xy, Scalar xz,
	 Scalar yx, Scalar yy, Scalar yz,
	 Scalar zx, Scalar zy, Scalar zz) { 
    setValue(xx, xy, xz, yx, yy, yz, zx, zy, zz);
  }
  
  Vector&       operator[](int i)       { return *(Vector *)elem[i]; }
  const Vector& operator[](int i) const { return *(Vector *)elem[i]; }
  
  Mat3&       getValue()       { return elem; }
  const Mat3& getValue() const { return elem; }

  void setValue(const float *m) {
    elem[X][X] = *m++; elem[Y][X] = *m++; elem[Z][X] = *m++; m++;
    elem[X][Y] = *m++; elem[Y][Y] = *m++; elem[Z][Y] = *m++; m++;
    elem[X][Z] = *m++; elem[Y][Z] = *m++; elem[Z][Z] = *m;
  }

  void setValue(const double *m) {
    elem[X][X] = *m++; elem[Y][X] = *m++; elem[Z][X] = *m++; m++;
    elem[X][Y] = *m++; elem[Y][Y] = *m++; elem[Z][Y] = *m++; m++;
    elem[X][Z] = *m++; elem[Y][Z] = *m++; elem[Z][Z] = *m;
  }

  void setValue(Scalar xx, Scalar xy, Scalar xz, 
		Scalar yx, Scalar yy, Scalar yz, 
		Scalar zx, Scalar zy, Scalar zz) {
    elem[X][X] = xx; elem[X][Y] = xy; elem[X][Z] = xz;
    elem[Y][X] = yx; elem[Y][Y] = yy; elem[Y][Z] = yz;
    elem[Z][X] = zx; elem[Z][Y] = zy; elem[Z][Z] = zz;
  }
  
  void setRotation(const Quaternion& q) {
    Scalar d = q.length2();
    assert(!eqz(d));
    Scalar s = 2 / d;
    Scalar xs = q[X] * s,   ys = q[Y] * s,   zs = q[Z] * s;
    Scalar wx = q[W] * xs,  wy = q[W] * ys,  wz = q[W] * zs;
    Scalar xx = q[X] * xs,  xy = q[X] * ys,  xz = q[X] * zs;
    Scalar yy = q[Y] * ys,  yz = q[Y] * zs,  zz = q[Z] * zs;
    setValue(1 - (yy + zz),       xy - wz,       xz + wy,
	     xy + wz      , 1 - (xx + zz),       yz - wx,
	     xz - wy      , yz + wx      , 1 - (xx + yy));
  }
  
  void setScaling(Scalar x, Scalar y, Scalar z) {
    setValue(x, 0, 0, 0, y, 0, 0, 0, z); 
  }
  
  void setIdentity() { setValue(1, 0, 0, 0, 1, 0, 0, 0, 1); }
  
  Matrix& operator*=(const Matrix& m); 

  Scalar tdot(int i, const Vector& v) const {
    return elem[X][i] * v[X] + elem[Y][i] * v[Y] + elem[Z][i] * v[Z];
  }
  
  Scalar determinant() const;
  Matrix absolute() const;
  Matrix transpose() const;
  Matrix adjoint() const;
  Matrix inverse() const; 
  
protected:
  Mat3 elem;
};

Vector operator*(const Matrix& m, const Vector& v);
Vector operator*(const Vector& v, const Matrix& m);
Matrix operator*(const Matrix& m1, const Matrix& m2);

Matrix multTransposeLeft(const Matrix& m1, const Matrix& m2);

Matrix transpose(const Matrix& m);
Matrix adjoint(const Matrix& m);
Matrix inverse(const Matrix& m);
Matrix absolute(const Matrix& m);

ostream& operator<<(ostream& os, const Matrix& m);



inline Matrix& Matrix::operator*=(const Matrix& m) {
  setValue(elem[X][X] * m[X][X] + elem[X][Y] * m[Y][X] + elem[X][Z] * m[Z][X],
	   elem[X][X] * m[X][Y] + elem[X][Y] * m[Y][Y] + elem[X][Z] * m[Z][Y],
	   elem[X][X] * m[X][Z] + elem[X][Y] * m[Y][Z] + elem[X][Z] * m[Z][Z],
	   elem[Y][X] * m[X][X] + elem[Y][Y] * m[Y][X] + elem[Y][Z] * m[Z][X],
	   elem[Y][X] * m[X][Y] + elem[Y][Y] * m[Y][Y] + elem[Y][Z] * m[Z][Y],
	   elem[Y][X] * m[X][Z] + elem[Y][Y] * m[Y][Z] + elem[Y][Z] * m[Z][Z],
	   elem[Z][X] * m[X][X] + elem[Z][Y] * m[Y][X] + elem[Z][Z] * m[Z][X],
	   elem[Z][X] * m[X][Y] + elem[Z][Y] * m[Y][Y] + elem[Z][Z] * m[Z][Y],
	   elem[Z][X] * m[X][Z] + elem[Z][Y] * m[Y][Z] + elem[Z][Z] * m[Z][Z]);
  return *this;
}

inline Scalar Matrix::determinant() const { 
  return triple((*this)[X], (*this)[Y], (*this)[Z]);
}

inline Matrix Matrix::absolute() const {
  return Matrix(fabs(elem[X][X]), fabs(elem[X][Y]), fabs(elem[X][Z]),
		fabs(elem[Y][X]), fabs(elem[Y][Y]), fabs(elem[Y][Z]),
		fabs(elem[Z][X]), fabs(elem[Z][Y]), fabs(elem[Z][Z]));
}

inline Matrix Matrix::transpose() const {
  return Matrix(elem[X][X], elem[Y][X], elem[Z][X],
		elem[X][Y], elem[Y][Y], elem[Z][Y],
		elem[X][Z], elem[Y][Z], elem[Z][Z]);
}

inline Matrix Matrix::adjoint() const {
  return Matrix(elem[Y][Y] * elem[Z][Z] - elem[Y][Z] * elem[Z][Y],
		elem[X][Z] * elem[Z][Y] - elem[X][Y] * elem[Z][Z],
		elem[X][Y] * elem[Y][Z] - elem[X][Z] * elem[Y][Y],
		elem[Y][Z] * elem[Z][X] - elem[Y][X] * elem[Z][Z],
		elem[X][X] * elem[Z][Z] - elem[X][Z] * elem[Z][X],
		elem[X][Z] * elem[Y][X] - elem[X][X] * elem[Y][Z],
		elem[Y][X] * elem[Z][Y] - elem[Y][Y] * elem[Z][X],
		elem[X][Y] * elem[Z][X] - elem[X][X] * elem[Z][Y],
		elem[X][X] * elem[Y][Y] - elem[X][Y] * elem[Y][X]);
}

inline Matrix Matrix::inverse() const {
  Vector co(elem[Y][Y] * elem[Z][Z] - elem[Y][Z] * elem[Z][Y],
	    elem[Y][Z] * elem[Z][X] - elem[Y][X] * elem[Z][Z], 
	    elem[Y][X] * elem[Z][Y] - elem[Y][Y] * elem[Z][X]);
  Scalar d = dot((*this)[X], co);
  assert(!eqz(d));
  Scalar s = 1 / d;
  return Matrix(co[X] * s,
		(elem[X][Z] * elem[Z][Y] - elem[X][Y] * elem[Z][Z]) * s,
		(elem[X][Y] * elem[Y][Z] - elem[X][Z] * elem[Y][Y]) * s,
		co[Y] * s,
		(elem[X][X] * elem[Z][Z] - elem[X][Z] * elem[Z][X]) * s,
		(elem[X][Z] * elem[Y][X] - elem[X][X] * elem[Y][Z]) * s,
		co[Z] * s,
		(elem[X][Y] * elem[Z][X] - elem[X][X] * elem[Z][Y]) * s,
		(elem[X][X] * elem[Y][Y] - elem[X][Y] * elem[Y][X]) * s);
}

inline Vector operator*(const Matrix& m, const Vector& v) {
  return Vector(dot(m[X], v), dot(m[Y], v), dot(m[Z], v));
}

inline Vector operator*(const Vector& v, const Matrix& m) {
  return Vector(m.tdot(X, v), m.tdot(Y, v), m.tdot(Z, v));
}

inline Matrix operator*(const Matrix& m1, const Matrix& m2) {
  return Matrix(
    m1[X][X] * m2[X][X] + m1[X][Y] * m2[Y][X] + m1[X][Z] * m2[Z][X],
    m1[X][X] * m2[X][Y] + m1[X][Y] * m2[Y][Y] + m1[X][Z] * m2[Z][Y],
    m1[X][X] * m2[X][Z] + m1[X][Y] * m2[Y][Z] + m1[X][Z] * m2[Z][Z],
    m1[Y][X] * m2[X][X] + m1[Y][Y] * m2[Y][X] + m1[Y][Z] * m2[Z][X],
    m1[Y][X] * m2[X][Y] + m1[Y][Y] * m2[Y][Y] + m1[Y][Z] * m2[Z][Y],
    m1[Y][X] * m2[X][Z] + m1[Y][Y] * m2[Y][Z] + m1[Y][Z] * m2[Z][Z],
    m1[Z][X] * m2[X][X] + m1[Z][Y] * m2[Y][X] + m1[Z][Z] * m2[Z][X],
    m1[Z][X] * m2[X][Y] + m1[Z][Y] * m2[Y][Y] + m1[Z][Z] * m2[Z][Y],
    m1[Z][X] * m2[X][Z] + m1[Z][Y] * m2[Y][Z] + m1[Z][Z] * m2[Z][Z]);
}

inline Matrix multTransposeLeft(const Matrix& m1, const Matrix& m2) {
  return Matrix(
    m1[X][X] * m2[X][X] + m1[Y][X] * m2[Y][X] + m1[Z][X] * m2[Z][X],
    m1[X][X] * m2[X][Y] + m1[Y][X] * m2[Y][Y] + m1[Z][X] * m2[Z][Y],
    m1[X][X] * m2[X][Z] + m1[Y][X] * m2[Y][Z] + m1[Z][X] * m2[Z][Z],
    m1[X][Y] * m2[X][X] + m1[Y][Y] * m2[Y][X] + m1[Z][Y] * m2[Z][X],
    m1[X][Y] * m2[X][Y] + m1[Y][Y] * m2[Y][Y] + m1[Z][Y] * m2[Z][Y],
    m1[X][Y] * m2[X][Z] + m1[Y][Y] * m2[Y][Z] + m1[Z][Y] * m2[Z][Z],
    m1[X][Z] * m2[X][X] + m1[Y][Z] * m2[Y][X] + m1[Z][Z] * m2[Z][X],
    m1[X][Z] * m2[X][Y] + m1[Y][Z] * m2[Y][Y] + m1[Z][Z] * m2[Z][Y],
    m1[X][Z] * m2[X][Z] + m1[Y][Z] * m2[Y][Z] + m1[Z][Z] * m2[Z][Z]);
}

inline Scalar determinant(const Matrix& m) { return m.determinant(); }
inline Matrix absolute(const Matrix& m) { return m.absolute(); }
inline Matrix transpose(const Matrix& m) { return m.transpose(); }
inline Matrix adjoint(const Matrix& m) { return m.adjoint(); }
inline Matrix inverse(const Matrix& m) { return m.inverse(); }

inline ostream& operator<<(ostream& os, const Matrix& m) {
  return os << m[X] << endl << m[Y] << endl << m[Z] << endl;
}

#endif
