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

#ifdef _MSC_VER
#pragma warning(disable:4786) // identifier was truncated to '255'
#endif // _MSC_VER

#include "Endpoint.h"
#include "Object.h"
#include "Encounter.h"

#include <new>

void addPair(ObjectPtr object1, ObjectPtr object2);
void removePair(ObjectPtr object1, ObjectPtr object2);

class EndpointList {
public:
  Endpoint head;
  Endpoint tail;

  EndpointList() {
    head.succ = &tail;
    head.pos = -INFINITY_;
    tail.pred = &head;
    tail.pos = INFINITY_;
  }
};

static EndpointList endpointList[3];

inline void Endpoint::insert(Endpoint *p) {
  succ = p;
  pred = p->pred;
  succ->pred = this;
  pred->succ = this;
} 

inline bool operator<(const Endpoint& a, const Endpoint& b) {
  return a.pos < b.pos || (a.pos == b.pos && a.side < b.side);
}

inline void update(const Endpoint& a, const Endpoint& b) {
  if (a.side != b.side && a.objPtr != b.objPtr) {
    if (a.side == MAX) {
      if (intersect(a.objPtr->getBBox(), b.objPtr->getBBox()))
        addPair(a.objPtr, b.objPtr);
    }
    else removePair(a.objPtr, b.objPtr);
  }
}

Endpoint::Endpoint(int axis, Side s, const Object *obj) : 
  side(s), 
  objPtr(obj), 
  pos(INFINITY_) 
{ insert(&endpointList[axis].tail); }

void Endpoint::move(Scalar x) {
  int sign = sgn(x - pos);
  pos = x;
  switch (sign) {
  case -1:
    if (*this < *pred) {
      remove();
      do update(*pred, *this);
      while (*this < *(pred = pred->pred));
      succ = pred->succ;
      pred->succ = this;
      succ->pred = this;
    }
    break;
  case 1:
    if (*succ < *this) {
      remove();
      do update(*this, *succ);
      while (*(succ = succ->succ) < *this);
      pred = succ->pred;
      succ->pred = this;
      pred->succ = this;
    }
    break;
  }
}






