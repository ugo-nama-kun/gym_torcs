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

#ifndef _TUPLE3_H_
#define _TUPLE3_H_

#include "Basic.h"

#include <assert.h>
#include <iostream>

using namespace std;

class Tuple3 {
public:
  Tuple3() {}
  Tuple3(const float v[3]) { setValue(v); }
  Tuple3(const double v[3]) { setValue(v); }
  Tuple3(Scalar x, Scalar y, Scalar z) { setValue(x, y, z); }
  
  Scalar&       operator[](int i)       { return comp[i]; }
  const Scalar& operator[](int i) const { return comp[i]; }

  Scalar       *getValue()       { return comp; }
  const Scalar *getValue() const { return comp; }

  void setValue(const float v[3]) {
    comp[X] = v[X]; comp[Y] = v[Y]; comp[Z] = v[Z];
  }
  
  void setValue(const double v[3]) {
    comp[X] = v[X]; comp[Y] = v[Y]; comp[Z] = v[Z];
  }
  
  void setValue(Scalar x, Scalar y, Scalar z) {
    comp[X] = x; comp[Y] = y; comp[Z] = z;
  }

protected:
  Scalar comp[3];                            
};

inline bool operator==(const Tuple3& t1, const Tuple3& t2) {
  return t1[X] == t2[X] && t1[Y] == t2[Y] && t1[Z] == t2[Z];
}

inline ostream& operator<<(ostream& os, const Tuple3& t) {
  return os << t[X] << ' ' << t[Y] << ' ' << t[Z];
}

#endif
