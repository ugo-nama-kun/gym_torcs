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

#include "Polyhedron.h"

#ifdef QHULL

extern "C" {
#include <qhull_a.h>
}

#include <vector>
#include <new>  

typedef vector<unsigned int> IndexBuf;

char qh_version[] = "SOLID 2.0";

Polyhedron::Polyhedron(const VertexBase& b, int c, const unsigned int v[]) :
    Polytope(b, c, v), cobound(0) {
  boolT ismalloc;
  int curlong, totlong, exitcode;
  char options[200];
  
  facetT *facet;
  vertexT *vertex;
  vertexT **vertexp;

  coordT *array = new coordT[numVerts()*3];
  coordT *p = &array[0];
  int i;
  for (i = 0; i < numVerts(); ++i) {
    *p++ = (*this)[i][X];
    *p++ = (*this)[i][Y];
    *p++ = (*this)[i][Z];
  }

  ismalloc = False; 	// True if qh_freeqhull should 'free(array)'
  qh_init_A(stdin, stdout, stderr, 0, NULL);
  if (exitcode = setjmp(qh errexit)) exit(exitcode);
  sprintf(options, "qhull Qx i s Tcv C-0");
  qh_initflags(options);
  qh_init_B(&array[0], numVerts(), 3, ismalloc);
  qh_qhull();
  qh_check_output();

  IndexBuf* indexBuf = new IndexBuf[numVerts()];
  IndexBuf facetIndices;
  FORALLfacets {
    setT *vertices = qh_facet3vertex(facet);
    FOREACHvertex_(vertices) facetIndices.push_back(qh_pointid(vertex->point));
    for (int i = 0, j = facetIndices.size()-1; 
	 i < facetIndices.size(); j = i++)
      indexBuf[facetIndices[j]].push_back(facetIndices[i]);
    facetIndices.erase(facetIndices.begin(), facetIndices.end());
  }

  cobound = new IndexArray[numVerts()];
  for (i = 0; i < numVerts(); ++i) 
    if (indexBuf[i].size())
      new(&cobound[i]) IndexArray(indexBuf[i].size(), &indexBuf[i][0]);

  curr_vertex = 0;
  while (indexBuf[curr_vertex].size() == 0) ++curr_vertex;

  delete [] indexBuf;
  delete [] array;

  qh NOerrexit = True;
  qh_freeqhull(!qh_ALL);
  qh_memfreeshort(&curlong, &totlong);
} 

Polyhedron::~Polyhedron() {
  delete [] cobound;
}

Point Polyhedron::support(const Vector& v) const {
  int last_vertex = -1;
  Scalar h = dot((*this)[curr_vertex], v), d;
  for (;;) {
    IndexArray& curr_cobound = cobound[curr_vertex];
    int i = 0, n = curr_cobound.size();
    while (i != n && 
	   (curr_cobound[i] == last_vertex || (d = dot((*this)[curr_cobound[i]], v)) <= h))
      ++i;
    if (i == n) break;
    last_vertex = curr_vertex;
    curr_vertex = curr_cobound[i];
    h = d;
  }
  return (*this)[curr_vertex];
}

#else

Polyhedron::Polyhedron(const VertexBase& b, int c, const unsigned int v[]) :
  Polytope(b, c, v), cobound(0), curr_vertex(0) {}

Polyhedron::~Polyhedron() {}

Point Polyhedron::support(const Vector& v) const {
  int c = 0;
  Scalar h = dot((*this)[0], v), d;
  for (int i = 1; i < numVerts(); ++i) {
    if ((d = dot((*this)[i], v)) > h) { c = i; h = d; }
  }
  return (*this)[c];
}

#endif

