/***************************************************************************

    file                 : linalg.h
    created              : Wed Feb 18 01:20:19 CET 2003
    copyright            : (C) 2003-2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: linalg.h,v 1.8 2005/08/05 08:54:09 berniw Exp $

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

inline float sign(float d) {
	return (d >= 0.0f) ? 1.0f : -1.0f;
}


class v2d {
	public:
		/* constructors */
		v2d() {}
		v2d(const v2d &src) { this->x = src.x; this->y = src.y; }
		v2d(float x, float y) { this->x = x; this->y = y; }

		/* operators */
		v2d& operator=(const v2d &src);         /* assignment */
		v2d operator+(const v2d &src) const;    /* addition */
		v2d operator-(void) const;              /* negation */
		v2d operator-(const v2d &src) const;    /* subtraction */
		v2d operator*(const float s) const;     /* multiply with scalar */
		float operator*(const v2d &src) const;  /* dot product */
		friend v2d operator*(const float s, const v2d & src);

		/* methods */
		float len(void) const;
		void normalize(void);
		float dist(const v2d &p) const;
		float cosalpha(const v2d &p2, const v2d &center) const;
		v2d rotate(const v2d &c, float arc) const;

		/* data */
		float x;
		float y;
};


/* assignment */
inline v2d& v2d::operator=(const v2d &src)
{
    x = src.x; y = src.y; return *this;
}


/* add *this + src (vector addition) */
inline v2d v2d::operator+(const v2d &src) const
{
    return v2d(x + src.x, y + src.y);
}


/* negation of *this */
inline v2d v2d::operator-(void) const
{
    return v2d(-x, -y);
}


/* compute *this - src (vector subtraction) */
inline v2d v2d::operator-(const v2d &src) const
{
    return v2d(x - src.x, y - src.y);
}


/* scalar product */
inline float v2d::operator*(const v2d &src) const
{
    return src.x*x + src.y*y;
}


/* multiply vector with scalar (v2d*float) */
inline v2d v2d::operator*(const float s) const
{
    return v2d(s*x, s*y);
}


/* multiply scalar with vector (float*v2d) */
inline v2d operator*(const float s, const v2d & src)
{
    return v2d(s*src.x, s*src.y);
}


/* compute cosine of the angle between vectors *this-c and p2-c */
inline float v2d::cosalpha(const v2d &p2, const v2d &c) const
{
	v2d l1 = *this-c;
	v2d l2 = p2 - c;
	return (l1*l2)/(l1.len()*l2.len());
}


/* rotate vector arc radians around center c */
inline v2d v2d::rotate(const v2d &c, float arc) const
{
	v2d d = *this-c;
	float sina = (float) sin(arc), cosa = (float) cos(arc);
	return c + v2d(d.x*cosa-d.y*sina, d.x*sina+d.y*cosa);
}


/* compute the length of the vector */
inline float v2d::len(void) const
{
	return (float) sqrt(x*x+y*y);
}


/* distance between *this and p */
inline float v2d::dist(const v2d &p) const
{ 
	return (float) sqrt((p.x-x)*(p.x-x)+(p.y-y)*(p.y-y));
}


/* normalize the vector */
inline void v2d::normalize(void)
{
	float l = this->len();
	x /= l; y /= l;
}


class Straight {
    public:
        /* constructors */
        Straight() {}
        Straight(float x, float y, float dx, float dy)
            {  p.x = x; p.y = y; d.x = dx; d.y = dy; d.normalize(); }
        Straight(const v2d &anchor, const v2d &dir)
            { p = anchor; d = dir; d.normalize(); }

        /* methods */
        v2d intersect(const Straight &s) const;
        float dist(const v2d &p) const; 

        /* data */
        v2d p;          /* point on the straight */
        v2d d;          /* direction of the straight */
};


/* intersection point of *this and s */
inline v2d Straight::intersect(const Straight &s) const
{
    float t = -(d.x*(s.p.y-p.y)+d.y*(p.x-s.p.x))/(d.x*s.d.y-d.y*s.d.x);
    return s.p + s.d*t;  
}


/* distance of point s from straight *this */
inline float Straight::dist(const v2d &s) const
{
    v2d d1 = s - p;
    v2d d3 = d1 - d*d1*d;
    return d3.len();
}

#endif // _LINALG_H_

