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

#ifndef _QUATERNION_H_
#define _QUATERNION_H_

#include "Tuple4.h"
#include "Vector.h"

class Quaternion : public Tuple4 {
public:
  Quaternion() {}
  Quaternion(const float v[4]) : Tuple4(v) {}
  Quaternion(const double v[4]) : Tuple4(v) {}
  Quaternion(Scalar x, Scalar y, Scalar z, Scalar w) : Tuple4(x, y, z, w) {}
  Quaternion(const Vector& axis, Scalar angle) { setRotation(axis, angle); }
  Quaternion(Scalar yaw, Scalar pitch, Scalar roll) { 
    setEuler(yaw, pitch, roll); 
  }

  void setRotation(const Vector& axis, Scalar angle) {
    Scalar d = axis.length();
    assert(!eqz(d));
    Scalar s = sin(angle / 2) / d;
    setValue(axis[X] * s, axis[Y] * s, axis[Z] * s, cos(angle / 2));
  }

  void setEuler(Scalar yaw, Scalar pitch, Scalar roll) {
    Scalar cosYaw = cos(yaw / 2);
    Scalar sinYaw = sin(yaw / 2);
    Scalar cosPitch = cos(pitch / 2);
    Scalar sinPitch = sin(pitch / 2);
    Scalar cosRoll = cos(roll / 2);
    Scalar sinRoll = sin(roll / 2);
    setValue(cosRoll * sinPitch * cosYaw + sinRoll * cosPitch * sinYaw,
	     cosRoll * cosPitch * sinYaw - sinRoll * sinPitch * cosYaw,
	     sinRoll * cosPitch * cosYaw - cosRoll * sinPitch * sinYaw,
	     cosRoll * cosPitch * cosYaw + sinRoll * sinPitch * sinYaw);
  }
  
  Quaternion& operator+=(const Quaternion& q);
  Quaternion& operator-=(const Quaternion& q);
  Quaternion& operator*=(const Quaternion& q);
  Quaternion& operator*=(Scalar s);
  Quaternion& operator/=(Scalar s);
  
  Scalar length2() const;
  Scalar length() const;

  bool approxZero() const;

  void normalize();
  Quaternion normalized() const;

  void conjugate();
  Quaternion conjugate() const;

  void invert();
  Quaternion inverse() const;

  static Quaternion random();
};

Quaternion operator+(const Quaternion& q1, const Quaternion& q2);
Quaternion operator-(const Quaternion& q1, const Quaternion& q2);
Quaternion operator-(const Quaternion& q);
Quaternion operator*(const Quaternion& q1, const Quaternion& q2);
Quaternion operator*(const Quaternion& q, Scalar s);
Quaternion operator/(const Quaternion& q, Scalar s);

Scalar dot(const Quaternion& q1, const Quaternion& q2);

Scalar length2(const Quaternion& q);
Scalar length(const Quaternion& q);

bool approxZero(const Quaternion& q);
bool approxEqual(const Quaternion& q1, const Quaternion& q2);



inline Quaternion& Quaternion::operator+=(const Quaternion& q) {
  comp[X] += q[X]; comp[Y] += q[Y]; comp[Z] += q[Z]; comp[W] += q[W];
  return *this;
}

inline Quaternion& Quaternion::operator-=(const Quaternion& q) {
  comp[X] -= q[X]; comp[Y] -= q[Y]; comp[Z] -= q[Z]; comp[W] -= q[W];
  return *this;
}
 
inline Quaternion& Quaternion::operator*=(const Quaternion& q) {
  setValue(comp[W] * q[X] + comp[X] * q[W] + comp[Y] * q[Z] - comp[Z] * q[Y],
	   comp[W] * q[Y] + comp[Y] * q[W] + comp[Z] * q[X] - comp[X] * q[Z],
	   comp[W] * q[Z] + comp[Z] * q[W] + comp[X] * q[Y] - comp[Y] * q[X],
	   comp[W] * q[W] - comp[X] * q[X] - comp[Y] * q[Y] - comp[Z] * q[Z]);
  return *this;
}

inline Quaternion& Quaternion::operator*=(Scalar s) {
  comp[X] *= s; comp[Y] *= s; comp[Z] *= s; comp[W] *= s;
  return *this;
}

inline Quaternion& Quaternion::operator/=(Scalar s) {
  assert(!eqz(s));
  return *this *= 1 / s;
}

inline Quaternion operator+(const Quaternion& q1, const Quaternion& q2) {
  return Quaternion(q1[X] + q2[X], q1[Y] + q2[Y], q1[Z] + q2[Z], q1[W] + q2[W]);
}

inline Quaternion operator-(const Quaternion& q1, const Quaternion& q2) {
  return Quaternion(q1[X] - q2[X], q1[Y] - q2[Y], q1[Z] - q2[Z], q1[W] - q2[W]);
}

inline Quaternion operator-(const Quaternion& q) {
  return Quaternion(-q[X], -q[Y], -q[Z], -q[W]);
}

inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2) {
  return 
    Quaternion(q1[W] * q2[X] + q1[X] * q2[W] + q1[Y] * q2[Z] - q1[Z] * q2[Y],
	       q1[W] * q2[Y] + q1[Y] * q2[W] + q1[Z] * q2[X] - q1[X] * q2[Z],
	       q1[W] * q2[Z] + q1[Z] * q2[W] + q1[X] * q2[Y] - q1[Y] * q2[X],
	       q1[W] * q2[W] - q1[X] * q2[X] - q1[Y] * q2[Y] - q1[Z] * q2[Z]); 
}

inline Quaternion operator*(const Quaternion& q, Scalar s) {
  return Quaternion(q[X] * s, q[Y] * s, q[Z] * s, q[W] * s);
}

inline Quaternion operator*(Scalar s, const Quaternion& q) { return q * s; }

inline Quaternion operator/(const Quaternion& q, Scalar s) {
  assert(!eqz(s));
  return q * (1 / s);
}

inline Scalar dot(const Quaternion& q1, const Quaternion& q2) {
  return q1[X] * q2[X] + q1[Y] * q2[Y] + q1[Z] * q2[Z] + q1[W] * q2[W];
}

inline Scalar Quaternion::length2() const { return dot(*this, *this); }
inline Scalar Quaternion::length() const { return sqrt(length2()); }

inline bool Quaternion::approxZero() const { return length2() < EPSILON2; }

inline void Quaternion::normalize() { *this /= length(); }
inline Quaternion Quaternion::normalized() const { return *this / length(); }

inline void Quaternion::conjugate() {
  comp[X] = -comp[X]; comp[Y] = -comp[Y]; comp[Z] = -comp[Z];
}

inline Quaternion Quaternion::conjugate() const {
  return Quaternion(-comp[X], -comp[Y], -comp[Z], comp[W]);
}
  
inline void Quaternion::invert() {
  conjugate();
  *this /= length2();
}

inline Quaternion Quaternion::inverse() const {
  return conjugate() / length2();
}

inline Scalar length2(const Quaternion& q) { return q.length2(); }
inline Scalar length(const Quaternion& q) { return q.length(); }

inline bool approxZero(const Quaternion& q) { return q.approxZero(); }
inline bool approxEqual(const Quaternion& q1, const Quaternion& q2) { 
  return approxZero(q1 - q2); 
}

// From: "Uniform Random Rotations", Ken Shoemake, Graphics Gems III, 
//       pg. 124-132
inline Quaternion Quaternion::random() {
  Scalar x0 = rnd();
  Scalar r1 = sqrt(1 - x0), r2 = sqrt(x0);
  Scalar t1 = TWO_PI * rnd(), t2 = TWO_PI * rnd();
  Scalar c1 = cos(t1), s1 = sin(t1);
  Scalar c2 = cos(t2), s2 = sin(t2);
  return Quaternion(s1 * r1, c1 * r1, s2 * r2, c2 * r2);
}

#endif



