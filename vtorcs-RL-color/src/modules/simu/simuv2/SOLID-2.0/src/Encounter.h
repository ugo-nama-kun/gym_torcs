/*
  SOLID - Software Library for Interference Detection
  Copyright (C) 1997-1998  Gino van den Bergen

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

#ifndef _ENCOUNTER_H_
#define _ENCOUNTER_H_

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include <3D/Vector.h>
#include "Object.h"

typedef const Object *ObjectPtr;

class Encounter {
public:
  ObjectPtr obj1;
  ObjectPtr obj2;
  Vector sep_axis;
  
  Encounter(ObjectPtr o1, ObjectPtr o2) {
    if (o2->shapePtr->getType() < o1->shapePtr->getType() || 
	(o2->shapePtr->getType() == o1->shapePtr->getType() && o2 < o1)) { 
      obj1 = o2; 
      obj2 = o1; 
    }
    else { 
      obj1 = o1; 
      obj2 = o2; 
    }
    sep_axis.setValue(0, 0, 0);
  }
};

inline bool operator<(const Encounter& x, const Encounter& y) { 
  return x.obj1 < y.obj1 || (!(y.obj1 < x.obj1) && x.obj2 < y.obj2); 
}

#endif
