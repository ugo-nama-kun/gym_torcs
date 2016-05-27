/***************************************************************************

    file                 : v2_t.h
    created              : Due Apr 5 13:51:00 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: v2_t.h,v 1.2 2005/08/05 09:04:53 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	Template for 2d vector of primitive types (float, double). This template is NOT
	intended to work with classes which allocate memory. Be aware that there are more
	efficient methods for doing most of the operations (avoiding temp verctors and make
	better use of registers). Later I will try to improve the performance (SSE,
	"abuse" of templates to avoid temporaries).
*/


#ifndef _LINALG_V2_T_H_
#define _LINALG_V2_T_H_

#include <tmath/v3_t.h>


template<class T> class v2t;
#ifndef _MSC_VER
template<class T> v2t<T> operator*(const T s, const v2t<T> & src);
#endif // _MSC_VER

template<class T> class v2t {
	public:
		// Friends.
#ifndef _MSC_VER
		friend v2t<T> operator*<>(const T s, const v2t<T> &src);	// Multiply by scalar.
#endif // _MSC_VER

		// Constructors.
		v2t() {}
		v2t(const v2t<T> &src):x(src.x), y(src.y) {}
		v2t(const T x, const T y):x(x), y(y) {}
		v2t(const T s):x(s), y(s) {}

		// Operators.
		v2t<T>& operator=(const v2t<T> &src);						// Assignment.
		v2t<T>& operator=(const v3t<T> &src);						// Assignment from v3t<T>
		v2t<T>& operator+=(const v2t<T> &src);						// Addition.
		v2t<T>& operator-=(const v2t<T> &src);						// Subtraction.
		v2t<T>& operator*=(const T s);								// Multiply with scalar.
		v2t<T>& operator/=(const T s);								// Divide by scalar.

		v2t<T> operator-(void) const;								// Negation.
		v2t<T> operator+(const v2t<T> &src) const;					// Addition.
		v2t<T> operator-(const v2t<T> &src) const;					// Subtraction.

		v2t<T> operator*(const T s) const;							// Multiply with scalar.
		T operator*(const v2t<T> &src) const;						// Dot product.
		v2t<T> operator/(const T s) const;							// Divide by scalar.
		int operator==(const v2t<T> &src) const;					// all fields equal?
		int operator!=(const v2t<T> &src) const;					// not all fields equal?


		// Other methods.
		T len(void) const;
		void normalize(void);
		T dist(const v2t<T> &p) const;
		T cosalpha(const v2t<T> &p2, const v2t<T> &center) const;
		v2t<T> rotate(const v2t<T> &c, T arc) const;
		T fakeCrossProduct(const v2t<T>* b) const;					// result.z := this X b
		int approxEquals(const v2t<T> &cmp, T eps);					// Approximately equality per component, eps > 0.0. TODO: Test

		// Data.
		union {
			struct { T x, y; };
			T vec[2];
		};
};


// Assignment
template<class T> inline v2t<T>& v2t<T>::operator=(const v2t<T> &src)
{
	x = src.x; y = src.y;
	return *this;
}


// Assignment
template<class T> inline v2t<T>& v2t<T>::operator=(const v3t<T> &src)
{
	x = src.x; y = src.y;
	return *this;
}


// Addition
template<class T> inline v2t<T>& v2t<T>::operator+=(const v2t<T> &src)
{
	x += src.x; y += src.y;
	return *this;
}


// Subtraction.
template<class T> inline v2t<T>& v2t<T>::operator-=(const v2t<T> &src)
{
	x -= src.x; y -= src.y;
	return *this;
}


// Multiply with scalar.
template<class T> inline v2t<T>& v2t<T>::operator*=(const T s)
{
	x *= s; y *= s;
	return *this;
}


// Divide by scalar.
template<class T> inline v2t<T>& v2t<T>::operator/=(const T s)
{
	x /= s; y /= s;
	return *this;
}


// Add *this + src (vector addition) */
template<class T> inline v2t<T> v2t<T>::operator+(const v2t<T> &src) const
{
    return v2t(x + src.x, y + src.y);
}


// Negation of *this
template<class T> inline v2t<T> v2t<T>::operator-(void) const
{
    return v2t(-x, -y);
}


// Compute *this - src (vector subtraction).
template<class T> inline v2t<T> v2t<T>::operator-(const v2t<T> &src) const
{
    return v2t(x - src.x, y - src.y);
}


template<class T> inline int v2t<T>::operator==(const v2t<T> &s) const
{
	if (this->x == s.x && this->y == s.y) {
		return 1;
	} else {
		return 0;
	}
}


template<class T> inline int v2t<T>::operator!=(const v2t<T> &s) const
{
	return !(*this == s);
}


// Approximately equality per component.
template<class T> inline int v2t<T>::approxEquals(const v2t<T> &cmp, T eps)
{
	if (fabs(cmp.x - this->x) < eps &&
		fabs(cmp.y - this->y) < eps)
	{
		return 1;
	} else {
		return 0;
	}
}


// Dot (scalar) product.
template<class T> inline T v2t<T>::operator*(const v2t<T> &src) const
{
    return src.x*x + src.y*y;
}


// Multiply vector with scalar (v2t*T)
template<class T> inline v2t<T> v2t<T>::operator*(const T s) const
{
    return v2t(s*x, s*y);
}


// Multiply scalar with vector (T*v2t).
template<class T> inline v2t<T> operator*(const T s, const v2t<T> & src)
{
    return v2t<T>(s*src.x, s*src.y);
}


template<class T> inline v2t<T> v2t<T>::operator/(const T s) const
{
	return v2t(this->x/s, this->y/s);
}


// Compute cosine of the angle between vectors *this-c and p2-c.
template<class T> inline T v2t<T>::cosalpha(const v2t<T> &p2, const v2t<T> &c) const
{
	v2t l1 = *this-c;
	v2t l2 = p2 - c;
	return (l1*l2)/(l1.len()*l2.len());
}


// Rotate vector arc radians around center c.
template<class T> inline v2t<T> v2t<T>::rotate(const v2t<T> &c, T arc) const
{
	v2t d = *this-c;
	T sina = sin(arc), cosa = cos(arc);
	return c + v2t(d.x*cosa-d.y*sina, d.x*sina+d.y*cosa);
}


// Compute the length of the vector.
template<class T> inline T v2t<T>::len(void) const
{
	return sqrt(x*x+y*y);
}


// Distance between *this and p.
template<class T> inline T v2t<T>::dist(const v2t<T> &p) const
{
	return sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}


// Normalize the vector.
template<class T> inline void v2t<T>::normalize(void)
{
	T l = this->len();
	x /= l; y /= l;
}


// "Cross Product" with z=0, good to check the order of the vectors.
template<class T> inline T v2t<T>::fakeCrossProduct(const v2t<T>* b) const
{
	return this->x*b->y - this->y*b->x;
}


#endif // _LINALG_V2_T_H_

