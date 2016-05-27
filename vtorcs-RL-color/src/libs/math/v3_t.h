/***************************************************************************

    file                 : v3_t.h
    created              : Due Apr 5 13:51:00 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: v3_t.h,v 1.2 2005/08/05 09:04:53 berniw Exp $

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
	Template for 3d vector of primitive types (float, double). This template is NOT
	intended to work with classes which allocate memory. Be aware that there are more
	efficient methods for doing most of the operations (avoiding temp verctors and make
	better use of registers). Later I will try to improve the performance (SSE,
	"abuse" of templates to avoid temporaries).
*/

#ifndef _LINALG_V3_T_H_
#define _LINALG_V3_T_H_


template<class T> class v3t;
#ifndef _MSC_VER
template<class T> v3t<T> operator*(const T s, const v3t<T> & src);
template<class T> v3t<T> crossProduct(const v3t<T> &a, const v3t<T> &b);
#endif //_MSC_VER


template<class T> class v3t {
	public:
		// Friends.
#ifndef _MSC_VER
		friend v3t<T> operator*<>(const T s, const v3t<T> & src);	// Multiply by scalar.
		friend v3t<T> crossProduct<>(const v3t<T> &a, const v3t<T> &b); // return a X b
#endif //_MSC_VER

		// Constructors.
		v3t() {}
		v3t(const v3t<T> &src):x(src.x), y(src.y), z(src.z) {}
		v3t(const T x, const T y, const T z):x(x), y(y), z(z) {}
		v3t(const T s):x(s), y(s), z(s) {}

		// Operators.
		v3t<T>& operator=(const v3t<T> &src);						// Assignment.
		v3t<T>& operator+=(const v3t<T> &src);						// Addition.
		v3t<T>& operator-=(const v3t<T> &src);						// Subtraction.
		v3t<T>& operator*=(const T s);								// Multiply with scalar.
		v3t<T>& operator/=(const T s);								// Divide by scalar.

		v3t<T> operator-(void) const;								// Negation.
		v3t<T> operator+(const v3t<T> &src) const;					// Addition.
		v3t<T> operator-(const v3t<T> &src) const;					// Subtraction.


		v3t<T> operator*(const T s) const;							// Multiply by scalar.
		T operator*(const v3t<T> &src) const;						// Dot product.
		v3t<T> operator/(const T s) const;							// Divide by scalar.
		int operator==(const v3t<T> &src) const;					// all fields equal?
		int operator!=(const v3t<T> &src) const;					// not all fields equal?


		// Other methods.
		T len(void);
		void normalize(void);
		void crossProduct(const v3t<T> &b, v3t<T> &r) const;		// r := this X b

		void dirVector(const v3t<T>* b, v3t<T>* r);					// r := this - b
		int approxEquals(const v3t<T> &cmp, T eps);					// Approximately equality per component, eps > 0.0. TODO: Test

		union {
			struct { T x, y, z; };
			T vec[3];
		};
};


template<class T> inline v3t<T> & v3t<T>::operator=(const v3t<T> &src)
{
	x = src.x; y = src.y; z = src.z;
	return *this;
}


// Addition
template<class T> inline v3t<T>& v3t<T>::operator+=(const v3t<T> &src)
{
	x += src.x; y += src.y; z += src.z;
	return *this;
}


// Subtraction.
template<class T> inline v3t<T>& v3t<T>::operator-=(const v3t<T> &src)
{
	x -= src.x; y -= src.y; z -= src.z;
	return *this;
}


// Multiply with scalar.
template<class T> inline v3t<T>& v3t<T>::operator*=(const T s)
{
	x *= s; y *= s; z *= s;
	return *this;
}


// Divide by scalar.
template<class T> inline v3t<T>& v3t<T>::operator/=(const T s)
{
	x /= s; y /= s; z /= s;
	return *this;
}


template<class T> inline v3t<T> v3t<T>::operator-(void) const
{
	return v3t(-this->x, -this->y, -this->z);
}


template<class T> inline v3t<T> v3t<T>::operator-(const v3t<T> &src) const
{
	return v3t(this->x - src.x, this->y - src.y, this->z - src.z);
}


template<class T> inline v3t<T> v3t<T>::operator+(const v3t<T> &src) const
{
	return v3t(this->x + src.x, this->y + src.y, this->z + src.z);
}


template<class T> inline int v3t<T>::operator==(const v3t<T> &s) const
{
	if (this->x == s.x && this->y == s.y && this->z == s.z) {
		return 1;
	} else {
		return 0;
	}
}


template<class T> inline int v3t<T>::operator!=(const v3t<T> &s) const
{
	return !(*this == s);
}


// Approximately equality per component.
template<class T> inline int v3t<T>::approxEquals(const v3t<T> &cmp, T eps)
{
	if (fabs(cmp.x - this->x) < eps &&
		fabs(cmp.y - this->y) < eps &&
		fabs(cmp.z - this->z) < eps)
	{
		return 1;
	} else {
		return 0;
	}
}


template<class T> inline v3t<T> v3t<T>::operator*(const T s) const
{
	return v3t(s*this->x, s*this->y, s*this->z);
}


template<class T> inline T v3t<T>::operator*(const v3t<T> &src) const
{
	return this->x*src.x + this->y*src.y + this->z*src.z;
}


template<class T> inline v3t<T> v3t<T>::operator/(const T s) const
{
	return v3t(this->x/s, this->y/s, this->z/s);
}


template<class T> inline T v3t<T>::len(void)
{
	return sqrt(x*x + y*y + z*z);
}


template<class T> inline void v3t<T>::normalize(void)
{
	T len = this->len();
	x /= len; y /= len; z /= len;
}


template<class T> inline void v3t<T>::crossProduct(const v3t<T>& b, v3t<T>& r) const
{
	r.x = this->y*b.z - this->z*b.y;
	r.y = this->z*b.x - this->x*b.z;
	r.z = this->x*b.y - this->y*b.x;
}


template<class T> inline void v3t<T>::dirVector(const v3t<T>* b, v3t<T>* r)
{
	r->x = this->x - b->x;
	r->y = this->y - b->y;
	r->z = this->z - b->z;
}


// Friends.

// Friend, scalar*vector.
template<class T> inline v3t<T> operator*(const T s, const v3t<T> & src)
{
	return v3t<T>(s*src.x, s*src.y, s*src.z);
}


// Friend, Vector cross product.
template<class T> inline v3t<T> crossProduct(const v3t<T>& a, const v3t<T>& b)
{
	return v3t<T>(a.y*b.z - a.z*b.y,
				  a.z*b.x - a.x*b.z,
				  a.x*b.y - a.y*b.x);
}


#endif // _LINALG_V3_T_H_

