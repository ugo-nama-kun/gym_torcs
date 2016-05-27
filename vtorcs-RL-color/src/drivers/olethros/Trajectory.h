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

#ifndef TRAJECTORY_H
#define TRAJECTORY_H

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <list>
#include <vector>
#include "TrackData.h"

class Trajectory
{
 public:
    std::vector<float> w; ///< parameters
    std::vector<float> dw; ///< parameter steps
    std::vector<float> dw2; ///<  parameter gradients
    std::vector<float> accel; ///<  maximum acceleration
    std::vector<int> indices; ///< data indices
    static Point GetPoint (Segment& s, float w);
    void Optimise(SegmentList track, int max_iter, float alpha, const char* fname, bool reset = true);
};

#endif
