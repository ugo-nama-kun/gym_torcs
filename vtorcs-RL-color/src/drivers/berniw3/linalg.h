/***************************************************************************

    file                 : linalg.h
    created              : Mo Mar 11 13:51:00 CET 2002
    copyright            : (C) 2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: linalg.h,v 1.1 2004/01/20 13:45:23 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _LINALG_H_
#define _LINALG_H_

class v3d {
	public:
		double x, y, z;		/* coordinates */

		/* constructors */
		v3d() {}
		v3d(const v3d &src) { this->x = src.x; this->y = src.y; this->z = src.z; }
		v3d(double x, double y, double z) { this->x = x; this->y = y; this->z = z; }

		/* operators */
		v3d & operator=(const v3d &src);
		v3d operator-(void);
		v3d operator-(const v3d &src);
		v3d operator+(const v3d &src);
		v3d operator*(const double s);							/* multiply by scalar */
		v3d operator/(const double s);							/* divide by scalar */
		double operator*(const v3d &src);						/* dot product */
		friend v3d operator*(const double s, const v3d & src);	/* multiply by scalar */

		/* other methods */
		double len(void);
		void normalize(void);
		void crossProduct(const v3d* b, v3d* r);				/* r := this X b */
		void dirVector(const v3d* b, v3d* r);					/* r := this - b */
};


inline v3d & v3d::operator=(const v3d &src)
{
	x = src.x; y = src.y; z = src.z; return *this;
}


inline v3d v3d::operator-(void)
{
	return v3d(-this->x, -this->y, -this->z);
}


inline v3d v3d::operator-(const v3d &src)
{
	return v3d(this->x - src.x, this->y - src.y, this->z - src.z);
}


inline v3d v3d::operator+(const v3d &src)
{
	return v3d(this->x + src.x, this->y + src.y, this->z + src.z);
}


inline v3d v3d::operator*(const double s)
{
	return v3d(s*this->x, s*this->y, s*this->z);
}


inline double v3d::operator*(const v3d &src)
{
	return this->x*src.x + this->y*src.y + this->z*src.z;
}


inline v3d operator*(const double s, const v3d & src)
{
	return v3d(s*src.x, s*src.y, s*src.z);
}


inline v3d v3d::operator/(const double s)
{
	return v3d(this->x/s, this->y/s, this->z/s);
}


inline double v3d::len(void)
{
	return sqrt(x*x + y*y + z*z);
}


inline void v3d::normalize(void)
{
	double len = this->len();
	x /= len; y /= len; z /= len;
}


inline void v3d::crossProduct(const v3d* b, v3d* r) {
	r->x = this->y*b->z - this->z*b->y;
	r->y = this->z*b->x - this->x*b->z;
	r->z = this->x*b->y - this->y*b->x;
}

inline void v3d::dirVector(const v3d* b, v3d* r) {
	r->x = this->x - b->x; r->y = this->y - b->y; r->z = this->z - b->z;
}

#endif // _LINALG_H_

