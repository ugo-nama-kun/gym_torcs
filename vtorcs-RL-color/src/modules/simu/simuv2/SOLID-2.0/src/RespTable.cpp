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

#include <vector>

#include "RespTable.h"

const Response& RespTable::find(DtObjectRef obj1, DtObjectRef obj2) const {
  PairList::const_iterator i = pairList.find(make_ObjPair(obj1, obj2));
  if (i != pairList.end()) return (*i).second;
  SingleList::const_iterator j = singleList.find(obj1);
  if (j != singleList.end()) return (*j).second;
  j = singleList.find(obj2);
  if (j != singleList.end()) return (*j).second;
  return defaultResp;
}

typedef vector<DtObjectRef> PartnerList;
PartnerList partnerList;

void RespTable::cleanObject(DtObjectRef obj) {
  resetSingle(obj);
  for (PairList::const_iterator i = pairList.begin(); 
       i != pairList.end(); ++i) { 
    if ((*i).first.first == obj) { 
      partnerList.push_back((*i).first.second);
    }
    else if ((*i).first.second == obj) {
      partnerList.push_back((*i).first.first);
    }
  }
  while (!partnerList.empty()) {
    resetPair(obj, partnerList.back());
    partnerList.pop_back();
  }
}

