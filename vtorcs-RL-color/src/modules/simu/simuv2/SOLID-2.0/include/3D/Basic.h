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

#ifndef _BASIC_H_
#define _BASIC_H_

#include <math.h>
#include <stdlib.h>

/* typedef double Scalar; */
#define Scalar double

const Scalar DEGS_PER_RAD = 57.29577951308232286465;
const Scalar RADS_PER_DEG =  0.01745329251994329547;
const Scalar TWO_PI =        6.28318530717958623200;
const Scalar EPSILON =       1.0e-10;
const Scalar EPSILON2 =      1.0e-20;
const Scalar INFINITY_ =      1.0e50;


/* inline Scalar abs(Scalar x) { return x < 0 ? -x : x; } */
inline Scalar rnd() { return (Scalar(rand()) + 0.5) / (Scalar(RAND_MAX) + 1); }
inline int    sgn(Scalar x) { return x < 0 ? -1 : x > 0 ? 1 : 0; }
inline bool   eqz(Scalar x) { return fabs(x) <= EPSILON; }

inline Scalar min(Scalar x, Scalar y) { return x > y ? y : x; }
inline Scalar max(Scalar x, Scalar y) { return x < y ? y : x; }

inline void set_min(Scalar& x, Scalar y) { if (x > y) x = y; }
inline void set_max(Scalar& x, Scalar y) { if (x < y) x = y; }

inline Scalar rads(Scalar x) { return x * RADS_PER_DEG; }
inline Scalar degs(Scalar x) { return x * DEGS_PER_RAD; }

enum { X = 0, Y = 1, Z = 2, W = 3 };

#endif
