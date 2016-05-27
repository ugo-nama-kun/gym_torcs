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

#include "Convex.h"
#include "BBox.h"
#include "Transform.h"

Scalar rel_error = 1e-6; // relative error in the computed distance
Scalar abs_error = 1e-10; // absolute error if the distance is almost zero

BBox Convex::bbox(const Transform& t) const {
  Point min(t.getOrigin()[X] + 
	    dot(t.getBasis()[X], support(-t.getBasis()[X])) - abs_error,
	    t.getOrigin()[Y] + 
	    dot(t.getBasis()[Y], support(-t.getBasis()[Y])) - abs_error,
	    t.getOrigin()[Z] + 
	    dot(t.getBasis()[Z], support(-t.getBasis()[Z])) - abs_error); 
  Point max(t.getOrigin()[X] + 
	    dot(t.getBasis()[X], support(t.getBasis()[X])) + abs_error,
	    t.getOrigin()[Y] + 
	    dot(t.getBasis()[Y], support(t.getBasis()[Y])) + abs_error,
	    t.getOrigin()[Z] + 
	    dot(t.getBasis()[Z], support(t.getBasis()[Z])) + abs_error); 
  return BBox(min, max);
}

static Point p[4];    // support points of object A in local coordinates 
static Point q[4];    // support points of object B in local coordinates 
static Vector y[4];   // support points of A - B in world coordinates

static int bits;      // identifies current simplex
static int last;      // identifies last found support point
static int last_bit;  // last_bit = 1<<last
static int all_bits;  // all_bits = bits|last_bit 

static Scalar det[16][4]; // cached sub-determinants

#ifdef STATISTICS
int num_iterations = 0;
int num_irregularities = 0;
#endif



void compute_det() {
  static Scalar dp[4][4];

  for (int i = 0, bit = 1; i < 4; ++i, bit <<=1) 
    if (bits & bit) dp[i][last] = dp[last][i] = dot(y[i], y[last]);
  dp[last][last] = dot(y[last], y[last]);

  det[last_bit][last] = 1;
  for (int j = 0, sj = 1; j < 4; ++j, sj <<= 1) {
    if (bits & sj) {
      int s2 = sj|last_bit;
      det[s2][j] = dp[last][last] - dp[last][j]; 
      det[s2][last] = dp[j][j] - dp[j][last];
      for (int k = 0, sk = 1; k < j; ++k, sk <<= 1) {
	if (bits & sk) {
	  int s3 = sk|s2;
	  det[s3][k] = det[s2][j] * (dp[j][j] - dp[j][k]) + 
	               det[s2][last] * (dp[last][j] - dp[last][k]);
	  det[s3][j] = det[sk|last_bit][k] * (dp[k][k] - dp[k][j]) + 
	               det[sk|last_bit][last] * (dp[last][k] - dp[last][j]);
	  det[s3][last] = det[sk|sj][k] * (dp[k][k] - dp[k][last]) + 
	                  det[sk|sj][j] * (dp[j][k] - dp[j][last]);
	}
      }
    }
  }
  if (all_bits == 15) {
    det[15][0] = det[14][1] * (dp[1][1] - dp[1][0]) + 
                 det[14][2] * (dp[2][1] - dp[2][0]) + 
                 det[14][3] * (dp[3][1] - dp[3][0]);
    det[15][1] = det[13][0] * (dp[0][0] - dp[0][1]) + 
                 det[13][2] * (dp[2][0] - dp[2][1]) + 
                 det[13][3] * (dp[3][0] - dp[3][1]);
    det[15][2] = det[11][0] * (dp[0][0] - dp[0][2]) + 
                 det[11][1] * (dp[1][0] - dp[1][2]) +  
                 det[11][3] * (dp[3][0] - dp[3][2]);
    det[15][3] = det[7][0] * (dp[0][0] - dp[0][3]) + 
                 det[7][1] * (dp[1][0] - dp[1][3]) + 
                 det[7][2] * (dp[2][0] - dp[2][3]);
  }
}

inline bool valid(int s) {  
  for (int i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
    if (all_bits & bit) {
      if (s & bit) { if (det[s][i] <= 0) return false; }
      else if (det[s|bit][i] > 0) return false;
    }
  }
  return true;
}

inline void compute_vector(int bits, Vector& v) {
  Scalar sum = 0;
  v.setValue(0, 0, 0);
  for (int i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
    if (bits & bit) {
      sum += det[bits][i];
      v += y[i] * det[bits][i];
    }
  }
  v *= 1 / sum;
}

inline void compute_points(int bits, Point& p1, Point& p2) {
  Scalar sum = 0;
  p1.setValue(0, 0, 0);
  p2.setValue(0, 0, 0);
  for (int i = 0, bit = 1; i < 4; ++i, bit <<= 1) {
    if (bits & bit) {
      sum += det[bits][i];
      p1 += p[i] * det[bits][i];
      p2 += q[i] * det[bits][i];
    }
  }
  Scalar s = 1 / sum;
  p1 *= s;
  p2 *= s;
}

#ifdef USE_BACKUP_PROCEDURE

inline bool proper(int s) {  
  for (int i = 0, bit = 1; i < 4; ++i, bit <<= 1)
    if ((s & bit) && det[s][i] <= 0) return false; 
  return true;
}

#endif

inline bool closest(Vector& v) {
  compute_det();
  for (int s = bits; s; --s) {
    if ((s & bits) == s) {
      if (valid(s|last_bit)) {
	bits = s|last_bit;
 	compute_vector(bits, v);
	return true;
      }
    }
  }
  if (valid(last_bit)) {
    bits = last_bit;
    v = y[last];
    return true;
  }
  // Original GJK calls the backup procedure at this point.

#ifdef USE_BACKUP_PROCEDURE

  Scalar min_dist2 = INFINITY_;
  for (int s = all_bits; s; --s) {
    if ((s & all_bits) == s) {
      if (proper(s)) {
	Vector u;
 	compute_vector(s, u);
	Scalar dist2 = u.length2();
	if (dist2 < min_dist2) {
	  min_dist2 = dist2;
	  bits = s;
	  v = u;
	}
      }
    }
  }

#endif 

  return false;
}

// The next function is used for detecting degenerate cases that cause 
// termination problems due to rounding errors.  
   
inline bool degenerate(const Vector& w) {
  for (int i = 0, bit = 1; i < 4; ++i, bit <<= 1) 
    if ((all_bits & bit) && y[i] == w)  return true;
  return false;
}

bool intersect(const Convex& a, const Convex& b,
	       const Transform& a2w, const Transform& b2w,
	       Vector& v) {
  Vector w;

  bits = 0;
  all_bits = 0;

#ifdef STATISTICS
  num_iterations = 0;
#endif

  do {
    last = 0;
    last_bit = 1;
    while (bits & last_bit) { ++last; last_bit <<= 1; }
    w = a2w(a.support((-v) * a2w.getBasis())) - 
      b2w(b.support(v * b2w.getBasis())); 
    if (dot(v, w) > 0) return false;
    if (degenerate(w)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
    y[last] = w;
    all_bits = bits|last_bit;
#ifdef STATISTICS
    ++num_iterations;
#endif
    if (!closest(v)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
  } 
  while (bits < 15 && !approxZero(v)); 
  return true;
}

bool intersect(const Convex& a, const Convex& b, const Transform& b2a, 
	       Vector& v) {
  Vector w;

  bits = 0;
  all_bits = 0;

#ifdef STATISTICS
  num_iterations = 0;
#endif

  do {
    last = 0;
    last_bit = 1;
    while (bits & last_bit) { ++last; last_bit <<= 1; }
    w = a.support(-v) - b2a(b.support(v * b2a.getBasis()));
    if (dot(v, w) > 0) return false;
    if (degenerate(w)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
    y[last] = w;
    all_bits = bits|last_bit;
#ifdef STATISTICS
    ++num_iterations;
#endif
    if (!closest(v)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
  } 
  while (bits < 15 && !approxZero(v)); 
  return true;
}





bool common_point(const Convex& a, const Convex& b,
		  const Transform& a2w, const Transform& b2w,
		  Vector& v, Point& pa, Point& pb) {
  Vector w;

  bits = 0;
  all_bits = 0;

#ifdef STATISTICS
  num_iterations = 0;
#endif

  do {
    last = 0;
    last_bit = 1;
    while (bits & last_bit) { ++last; last_bit <<= 1; }
    p[last] = a.support((-v) * a2w.getBasis());
    q[last] = b.support(v * b2w.getBasis());
    w = a2w(p[last]) - b2w(q[last]);
    if (dot(v, w) > 0) return false;
    if (degenerate(w)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
    y[last] = w;
    all_bits = bits|last_bit;
#ifdef STATISTICS
    ++num_iterations;
#endif
    if (!closest(v)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
  }
  while (bits < 15 && !approxZero(v) ) ;
  compute_points(bits, pa, pb);
  return true;
}

bool common_point(const Convex& a, const Convex& b, const Transform& b2a,
		  Vector& v, Point& pa, Point& pb) {
  Vector w;

  bits = 0;
  all_bits = 0;

#ifdef STATISTICS
  num_iterations = 0;
#endif

  do {
    last = 0;
    last_bit = 1;
    while (bits & last_bit) { ++last; last_bit <<= 1; }
    p[last] = a.support(-v);
    q[last] = b.support(v * b2a.getBasis());
    w = p[last] - b2a(q[last]);
    if (dot(v, w) > 0) return false;
    if (degenerate(w)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
    y[last] = w;
    all_bits = bits|last_bit;
#ifdef STATISTICS
    ++num_iterations;
#endif
    if (!closest(v)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      return false;
    }
  }
  while (bits < 15 && !approxZero(v) );   
  compute_points(bits, pa, pb);
  return true;
}

#ifdef STATISTICS
void catch_me() {}
#endif


void closest_points(const Convex& a, const Convex& b,
		    const Transform& a2w, const Transform& b2w,
		    Point& pa, Point& pb) {
  static Vector zero(0, 0, 0);
  
  Vector v = a2w(a.support(zero)) - b2w(b.support(zero));
  Scalar dist = v.length();

  Vector w;

  bits = 0;
  all_bits = 0;
  Scalar mu = 0;

#ifdef STATISTICS
  num_iterations = 0;
#endif

  while (bits < 15 && dist > abs_error) {
    last = 0;
    last_bit = 1;
    while (bits & last_bit) { ++last; last_bit <<= 1; }
    p[last] = a.support((-v) * a2w.getBasis());
    q[last] = b.support(v * b2w.getBasis());
    w = a2w(p[last]) - b2w(q[last]);
    set_max(mu, dot(v, w) / dist);
    if (dist - mu <= dist * rel_error) break; 
    if (degenerate(w)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      break;
    }
    y[last] = w;
    all_bits = bits|last_bit;
#ifdef STATISTICS
    ++num_iterations;
    if (num_iterations > 1000) catch_me();
#endif
    if (!closest(v)) {
#ifdef STATISTICS
      ++num_irregularities;
#endif
      break;
    }
    dist = v.length();
  }
  compute_points(bits, pa, pb);
}



