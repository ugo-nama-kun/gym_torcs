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

#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "Tuple3.h"

class Vector : public Tuple3 {
public:
  Vector() {}
  Vector(const float v[3]) : Tuple3(v) {}
  Vector(const double v[3]) : Tuple3(v) {}
  Vector(Scalar x, Scalar y, Scalar z) : Tuple3(x, y, z) {}
  
  Vector& operator+=(const Vector& v);
  Vector& operator-=(const Vector& v);
  Vector& operator*=(Scalar s);
  Vector& operator/=(Scalar s);
  
  Scalar length2() const;
  Scalar length() const;

  bool approxZero() const;

  void normalize();
  Vector normalized() const;

  int closestAxis() const;

  static Vector random();
};

Vector operator+(const Vector& v1, const Vector& v2);
Vector operator-(const Vector& v1, const Vector& v2);
Vector operator-(const Vector& v);
Vector operator*(const Vector& v, Scalar s);
Vector operator*(Scalar s, const Vector& v);
Vector operator/(const Vector& v, Scalar s);

Scalar dot(const Vector& v1, const Vector& v2);

Scalar length2(const Vector& v);
Scalar length(const Vector& v);

bool approxZero(const Vector& v);
bool approxEqual(const Vector& v1, const Vector& v2);

Scalar angle(const Vector& v1, const Vector& v2);
Vector cross(const Vector& v1, const Vector& v2);
Scalar triple(const Vector& v1, const Vector& v2, const Vector& v3);



inline Vector& Vector::operator+=(const Vector& v) {
  comp[X] += v[X]; comp[Y] += v[Y]; comp[Z] += v[Z];
  return *this;
}

inline Vector& Vector::operator-=(const Vector& v) {
  comp[X] -= v[X]; comp[Y] -= v[Y]; comp[Z] -= v[Z];
  return *this;
}
 
inline Vector& Vector::operator*=(Scalar s) {
  comp[X] *= s; comp[Y] *= s; comp[Z] *= s;
  return *this;
}

inline Vector& Vector::operator/=(Scalar s) {
  assert(!eqz(s));
  return *this *= 1 / s;
}

inline Vector operator+(const Vector& v1, const Vector& v2) {
  return Vector(v1[X] + v2[X], v1[Y] + v2[Y], v1[Z] + v2[Z]);
}

inline Vector operator-(const Vector& v1, const Vector& v2) {
  return Vector(v1[X] - v2[X], v1[Y] - v2[Y], v1[Z] - v2[Z]);
}

inline Vector operator-(const Vector& v) {
  return Vector(-v[X], -v[Y], -v[Z]);
}

inline Vector operator*(const Vector& v, Scalar s) {
  return Vector(v[X] * s, v[Y] * s, v[Z] * s);
}

inline Vector operator*(Scalar s, const Vector& v) { return v * s; }

inline Vector operator/(const Vector& v, Scalar s) {
  assert(!eqz(s));
  return v * (1 / s);
}

inline Scalar dot(const Vector& v1, const Vector& v2) {
  return v1[X] * v2[X] + v1[Y] * v2[Y] + v1[Z] * v2[Z];
}

inline Scalar Vector::length2() const { return dot(*this, *this); }
inline Scalar Vector::length() const { return sqrt(length2()); }

inline bool Vector::approxZero() const { return length2() < EPSILON2; }

inline void Vector::normalize() { *this /= length(); }
inline Vector Vector::normalized() const { return *this / length(); }

inline int Vector::closestAxis() const {
  Scalar a[2];
  int axis = (a[X] = fabs(comp[X])) < (a[Y] = fabs(comp[Y])) ? Y : X;
  return a[axis] < fabs(comp[Z]) ? Z : axis;
}

inline Vector Vector::random() {
  Scalar z = 2 * rnd() - 1;
  Scalar r = sqrt(1 - z * z);
  Scalar t = TWO_PI * rnd();
  return Vector(r * cos(t), r * sin(t), z);
}

inline Scalar length2(const Vector& v) { return v.length2(); }
inline Scalar length(const Vector& v) { return v.length(); }

inline bool approxZero(const Vector& v) { return v.approxZero(); }
inline bool approxEqual(const Vector& v1, const Vector& v2) { 
  return approxZero(v1 - v2); 
}

inline Scalar angle(const Vector& v1, const Vector& v2) {
  Scalar s = sqrt(v1.length2() * v2.length2());
  assert(!eqz(s));
  return acos(dot(v1, v2) / s);
}

inline Vector cross(const Vector& v1, const Vector& v2) {
  return Vector(v1[Y] * v2[Z] - v1[Z] * v2[Y],
		v1[Z] * v2[X] - v1[X] * v2[Z],
		v1[X] * v2[Y] - v1[Y] * v2[X]);
}

inline Scalar triple(const Vector& v1, const Vector& v2, const Vector& v3) {
  return v1[X] * (v2[Y] * v3[Z] - v2[Z] * v3[Y]) + 
         v1[Y] * (v2[Z] * v3[X] - v2[X] * v3[Z]) + 
         v1[Z] * (v2[X] * v3[Y] - v2[Y] * v3[X]);
}

#endif

