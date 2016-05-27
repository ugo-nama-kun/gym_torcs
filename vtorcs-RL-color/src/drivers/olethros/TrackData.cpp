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
#include "TrackData.h"
#include <tgf.h>

TrackData::TrackData()
{
    mid.x = mid.y = mid.z = 0.0f;
    width_l = width_r = 10.0;
    angle = 0.0;
    step= 5.0;
}

void TrackData::setWidth(float width)
{
    width_l = width_r = width / 2.0f;
    assert (width_r > - width_l);
}    

void TrackData::setLeftWidth(float width)
{
    width_l = width;
    assert (width_r > - width_l);
}

void TrackData::setRightWidth(float width)
{
    width_r = width;
    assert (width_r > - width_l);
}

void TrackData::setStep(float step)
{
    assert (step>0.0f);
    this->step = step;
}

void TrackData::AddStraight (SegmentList& segments, float length, float end_width_l, float end_width_r)
{
    int N = 1 + (int) floor(length/step);
    float s = length / (float) N;
    float d_width_l = (end_width_l - width_l) / (float) N;
    float d_width_r = (end_width_r - width_r) / (float) N;
    float hpi = PI/2.0f;
    for (int i=0; i<N; ++i) {
        mid.x += s*sin(angle);
        mid.y += s*cos(angle);
        Point left(mid.x + width_l*sin(angle - hpi),
                   mid.y + width_l*cos(angle - hpi),
                   mid.z);
        Point right(mid.x + width_r*sin(angle + hpi),
                    mid.y + width_r*cos(angle + hpi),
                    mid.z);
        segments.Add (Segment (left, right));
        width_l+=d_width_l;
        width_r+=d_width_r;
    }
        
    width_l = end_width_l;
    width_r = end_width_r;
}

/// arc in radians
void TrackData::AddCurve (SegmentList& segments, float arc, float radius, float end_width_l, float end_width_r)
{
    arc = arc * PI/180.0f;
    float length = fabs(arc) * radius;
    int N = 1 + (int) floor(length/step);
    float s = length / (float) N;
    float d_width_l = (end_width_l - width_l) / (float) N;
    float d_width_r = (end_width_r - width_r) / (float) N;
    float d_angle = arc / (float) N;
    float start_angle = angle;
    float hpi = (float) (PI/2.0);
    for (int i=0; i<N; ++i) {
        mid.x += s*sin(angle);
        mid.y += s*cos(angle);
        Point left(mid.x + width_l*sin(angle - hpi),
                   mid.y + width_l*cos(angle - hpi),
                   mid.z);
        Point right(mid.x + width_r*sin(angle + hpi),
                    mid.y + width_r*cos(angle + hpi),
                    mid.z);
        segments.Add (Segment (left, right));
        angle += d_angle;
        width_l += d_width_l;
        width_r += d_width_r;
    }
    width_l = end_width_l;
    width_r = end_width_r;
    angle = start_angle + arc;
}
