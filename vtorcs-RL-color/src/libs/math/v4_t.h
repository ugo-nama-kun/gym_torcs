/***************************************************************************

    file                 : v4_t.h
    created              : Due Apr 5 13:51:00 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: v4_t.h,v 1.2 2005/08/05 09:04:53 berniw Exp $

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
	Template for 4d vector of primitive types (float, double). This template is NOT
	intended to work with classes which allocate memory. Be aware that there are more
	efficient methods for doing most of the operations (avoiding temp verctors and make
	better use of registers). Later I will try to improve the performance (SSE,
	"abuse" of templates to avoid temporaries).
*/

#ifndef _LINALG_V4_T_H_
#define _LINALG_V4_T_H_


template<class T> class v4t;
#ifndef _MSC_VER
template<class T> v4t<T> operator* (const T s, const v4t<T> & src);
#endif //_MSC_VER


template<class T> class v4t {
	public:
		// Friends.
#ifndef _MSC_VER
		friend v4t<T> operator* <>(const T s, const v4t<T> &src);	// Multiply by scalar.
#endif //_MSC_VER

		// Constructors.
		v4t() {}
		v4t(const v4t<T> &src):x(src.x), y(src.y), z(src.z), w(src.w) {}
		v4t(const T x, const T y, const T z, const T w):x(x), y(y), z(z), w(w) {}
		v4t(const T s):x(s), y(s), z(s), w(s) {}

		// Operators.
		v4t<T>& operator=(const v4t<T> &src);						// Assignment.
		v4t<T>& operator+=(const v4t<T> &src);						// Addition.
		v4t<T>& operator-=(const v4t<T> &src);						// Subtraction.
		v4t<T>& operator*=(const T s);								// Multiply with scalar.
		v4t<T>& operator/=(const T s);								// Divide by scalar.

		v4t<T> operator-(void) const;								// Negation.
		v4t<T> operator+(const v4t<T> &src) const;					// Addition.
		v4t<T> operator-(const v4t<T> &src) const;					// Subtraction.

		v4t<T> operator*(const T s) const;							// Multiply with scalar.
		T operator*(const v4t<T> &src) const;						// Dot product.

		v4t<T> operator/(const T s) const;							// Divide by scalar.
		int operator==(const v4t<T> &src) const;					// all fields equal?
		int operator!=(const v4t<T> &src) const;					// not all fields equal?

		// Other methods.
		T len(void) const;
		void normalize(void);
		T dist(const v4t<T> &p) const;
		int approxEquals(const v4t<T> &cmp, T eps);					// Approximately equality per component, eps > 0.0. TODO: Test

		// Data.
		union {
			struct { T x, y, z, w; };
			T vec[4];
		};
};


// Assignment
template<class T> inline v4t<T>& v4t<T>::operator=(const v4t<T> &src)
{
	x = src.x; y = src.y; z = src.z; w = src.w;
	return *this;
}


// Addition
template<class T> inline v4t<T>& v4t<T>::operator+=(const v4t<T> &src)
{
	x += src.x; y += src.y; z += src.z; w += src.w;
	return *this;
}


// Subtraction.
template<class T> inline v4t<T>& v4t<T>::operator-=(const v4t<T> &src)
{
	x -= src.x; y -= src.y; z -= src.z; w -= src.w;
	return *this;
}


// Multiply with scalar.
template<class T> inline v4t<T>& v4t<T>::operator*=(const T s)
{
	x *= s; y *= s; z *= s; w *= s;
	return *this;
}


// Divide by scalar.
template<class T> inline v4t<T>& v4t<T>::operator/=(const T s)
{
	x /= s; y /= s; z /= s; w /= s;
	return *this;
}


// Add *this + src (vector addition) */
template<class T> inline v4t<T> v4t<T>::operator+(const v4t<T> &src) const
{
    return v4t(x + src.x, y + src.y, z + src.z, w + src.w);
}


// Negation of *this
template<class T> inline v4t<T> v4t<T>::operator-(void) const
{
    return v4t(-x, -y, -z, -w);
}


// Compute *this - src (vector subtraction).
template<class T> inline v4t<T> v4t<T>::operator-(const v4t<T> &src) const
{
    return v4t(x - src.x, y - src.y, z - src.z, w - src.w);
}


template<class T> inline int v4t<T>::operator==(const v4t<T> &s) const
{
	if (this->x == s.x && this->y == s.y && this->z == s.z && this->w == s.w) {
		return 1;
	} else {
		return 0;
	}
}


template<class T> inline int v4t<T>::operator!=(const v4t<T> &s) const
{
	return !(*this == s);
}


// Approximately equality per component.
template<class T> inline int v4t<T>::approxEquals(const v4t<T> &cmp, T eps)
{
	if (fabs(cmp.x - this->x) < eps &&
		fabs(cmp.y - this->y) < eps &&
		fabs(cmp.z - this->z) < eps &&
		fabs(cmp.w - this->w) < eps)
	{
		return 1;
	} else {
		return 0;
	}
}


// Dot (scalar) product.
template<class T> inline T v4t<T>::operator*(const v4t<T> &src) const
{
    return src.x*x + src.y*y + src.z*z + src.w*w;
}


// Multiply vector with scalar (v4t*T)
template<class T> inline v4t<T> v4t<T>::operator*(const T s) const
{
    return v4t(s*x, s*y, s*z, s*w);
}


// friend, Multiply scalar with vector (T*v4t).
template<class T> inline v4t<T> operator*(const T s, const v4t<T> & src)
{
    return v4t<T>(s*src.x, s*src.y, s*src.z, s*src.w);
}


template<class T> inline v4t<T> v4t<T>::operator/(const T s) const
{
	return v4t(this->x/s, this->y/s, this->z/s, this->w/s);
}


// Compute the length of the vector.
template<class T> inline T v4t<T>::len(void) const
{
	return sqrt(x*x + y*y + z*z + w*w);
}


// Distance between *this and p.
template<class T> inline T v4t<T>::dist(const v4t<T> &p) const
{
	return sqrt((p.x-x)*(p.x-x) + (p.y-y)*(p.y-y) + (p.z-z)*(p.z-z) + (p.w-w)*(p.w-w));
}


// Normalize the vector.
template<class T> inline void v4t<T>::normalize(void)
{
	T l = this->len();
	x /= l; y /= l; z /= l; w /= l;
}


#endif // _LINALG_V4_T_H_

