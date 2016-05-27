// -*- Mode: c++ -*-
// copyright (c) 2006 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <list>
#include <vector>


class Point
{
 protected:
    float _length;
 public:
    float x;
    float y;
    float z;

    Point()
    {
        _length = -1.0f;
    }
    Point(float x, float y, float z  = 0.0)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        _length = -1.0f;
    }
    float Length()
    {
        if (_length<0) {
            _length = sqrt(x*x + y*y + z*z);
        }
        return _length;
    }
    void Normalise()
    {
        float s = 1.0f/Length();
        x *= s;
        y *= s;
        z *= s;
        _length = 1.0f;
    }
    Point& operator= (const Point& rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }
    Point& operator-= (const Point& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    Point& operator+= (const Point& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    Point operator+ (const Point& rhs)
    {
        Point lhs;
        lhs.x = x + rhs.x;
        lhs.y = y + rhs.y;
        lhs.z = z + rhs.z;
        return lhs;
    }
    Point operator- (const Point& rhs)
    {
        Point lhs;
        lhs.x = x - rhs.x;
        lhs.y = y - rhs.y;
        lhs.z = z - rhs.z;
        return lhs;
    }
    Point& operator*= (const float& rhs)
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    Point operator* (const float& rhs)
    {
        Point lhs;
        lhs.x = x*rhs;
        lhs.y = y*rhs;
        lhs.z = z*rhs;
        return lhs;
    }
    Point& operator/= (const float& rhs)
    {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    Point operator/ (const float& rhs)
    {
        Point lhs;
        lhs.x = x/rhs;
        lhs.y = y/rhs;
        lhs.z = z/rhs;
        return lhs;
    }

};

class Segment
{
 public:
    Point left;
    Point right;
    Segment (Point left, Point right)
    {
        this->left = left;
        this->right = right;
    }
};


class SegmentList
{
 protected:
    std::vector<Segment> segments;
 public:
    void Add(Segment segment)
    {
        segments.push_back(segment);
    }

    Segment& operator[](int i)
    {
        return segments[i];
    }

    int size()
    {
        return segments.size();
    }

    void PrintSegments()
    {
        int N = segments.size();
        for (int i=0; i<N; ++i) {
            printf ("%f %f %f %f\n",
                    segments[i].left.x, segments[i].left.y,
                    segments[i].right.x, segments[i].right.y);
        }
    }
};

class TrackData
{
    float width_l;
    float width_r;
    float angle;
    float step;
    Point mid;
 public:
    TrackData();
    void setWidth(float width);
    void setLeftWidth(float width);
    void setRightWidth(float width);
    void setStep(float step);
    void AddStraight (SegmentList& segments, float length, float end_width_l, float end_width_r);
    void AddCurve (SegmentList& segments, float arc, float radius, float end_width_l, float end_width_r);
};

#endif
