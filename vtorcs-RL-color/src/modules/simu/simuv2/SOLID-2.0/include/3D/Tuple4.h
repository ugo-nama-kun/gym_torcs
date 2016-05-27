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

#ifndef _TUPLE4_H_
#define _TUPLE4_H_

#include "Basic.h"

#include <assert.h>
#include <iostream>

using namespace std;

class Tuple4 {
public:
  Tuple4() {}
  Tuple4(const float v[4]) { setValue(v); }
  Tuple4(const double v[4]) { setValue(v); }
  Tuple4(Scalar x, Scalar y, Scalar z, Scalar w) { setValue(x, y, z, w); }
  
  Scalar&       operator[](int i)       { return comp[i]; }
  const Scalar& operator[](int i) const { return comp[i]; }

  Scalar       *getValue()       { return comp; }
  const Scalar *getValue() const { return comp; }

  void setValue(const float v[4]) {
    comp[X] = v[X]; comp[Y] = v[Y]; comp[Z] = v[Z]; comp[W] = v[W];
  }
  
  void setValue(const double v[4]) {
    comp[X] = v[X]; comp[Y] = v[Y]; comp[Z] = v[Z]; comp[W] = v[W];
  }
  
  void setValue(Scalar x, Scalar y, Scalar z, Scalar w) {
    comp[X] = x; comp[Y] = y; comp[Z] = z; comp[W] = w;
  }
  
protected:
  Scalar comp[4];                            
};

inline bool operator==(const Tuple4& t1, const Tuple4& t2) {
  return t1[X] == t2[X] && t1[Y] == t2[Y] && t1[Z] == t2[Z] && t1[W] == t2[W];
}

inline ostream& operator<<(ostream& os, const Tuple4& t) {
  return os << t[X] << ' ' << t[Y] << ' ' << t[Z] << ' ' << t[W];
}

#endif
