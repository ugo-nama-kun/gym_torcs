// -*- Mode: c++ -*-
/* VER: $Id: MathFunctions.cpp,v 1.4 2007/11/06 20:43:32 torcs Exp $ */
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <learning/MathFunctions.h>
#include <cmath>
#include <cassert>

/**
   \file MathFunctions.cpp
   
   \brief Mathematical functions.

   A range of functions
*/

int ArgMin (int n, real* x)
{
    real min = x[0];
    int arg_min = 0;
    for (int i=1; i<n; i++) {
	if (min>x[i]) {
	    min = x[i];
	    arg_min = i;
	}
    }
    return arg_min;
}
int ArgMax (int n, real* x)
{
    real max = x[0];
    int arg_max = 0;
    for (int i=1; i<n; i++) {
	if (max<x[i]) {
	    max = x[i];
	    arg_max = i;
	}
    }
    return arg_max;
}

void SoftMax (int n, real* Q, real* p, real beta)
{
	real sum = 0.0;
	int i;
	for (i=0; i<n; i++) {
		p[i] = (real) exp (beta * Q[i]);
		sum += p[i];
	}
	sum = 1.0f/sum;
	for (i=0; i<n; i++) {
		p[i] *= sum;
	}
}
	
void SoftMin (int n, real* Q, real* p, real beta)
{
	real sum = 0.0f;
	int i;
	for (i=0; i<n; i++) {
		p[i] = (real) exp (-beta * Q[i]);
		sum += p[i];
	}
	sum = 1.0f/sum;
	for (i=0; i<n; i++) {
		p[i] *= sum;
	}
}
	

/**
   \brief Approximate max (f1,f2) via a gamma function.
	
	We have:
	\f[
	f(x) = \max \{f_1(x), f_2(x)\}
	= f_1(x) + \max \{0, f_2(x) - f_1(x)\}.
	\f]
	We can then approximate the second term with
	\f[
	\max \{0, f_2(x) - f_1(x)\} = \lim_{c \rightarrow \infty} \gamma(f_2(x)-f_1(x), \lambda, c),
	\f]
	where \f$\lambda \in [0,1], \quad c>0\f$. The function has the form
	\f[
	\gamma(t,\lambda,c) =
	\begin{cases}
	t - \frac{(1-\lambda)^2}{2c}  & \textrm{if}~ \frac{1-\lambda}{c} \leq t,\\
	\lambda t - \frac{c}{2} t^2  & \textrm{if}~ -\frac{\lambda}{c} \leq t \leq \frac{1-\lambda}{c},\\
	-\frac{\lambda^2}{2c}  & \text{if}~ t \leq  -\frac{\lambda}{c}.
	\end{cases}
	\f]

	The error in the positive region is \f$(1-\lambda)^2/2c\f$ and in
	the negative region it is \f$\lambda^2/2c\f$. The transition region
	width is \f$1/c\f$.

	The advantage of this function is that it is continuously
	differentiable. Its derivative is
	\f[
	\frac{\partial \gamma(t,\lambda,c)}{\partial t} =
	\begin{cases}
	1  & \textrm{if}~ \frac{1-\lambda}{c} \leq t,\\
	\lambda - c t & \textrm{if}~ -\frac{\lambda}{c} \leq t \leq \frac{1-\lambda}{c},\\
	0  & \text{if}~ t \leq  -\frac{\lambda}{c}.
	\end{cases}
	\f]

	The error 
*/
real SmoothMaxGamma (real f1, real f2, real lambda, real c)
{
	real t = f2-f1;
	real gamma;

	assert(c>0);
	assert(lambda>=0 && lambda<=1);

	if (1-lambda/c <= t) {
		gamma = t  - (1-lambda)*(1-lambda) / (2*c);
	} else if (t>=-lambda/c) {
		gamma = lambda * t + t*t*c/2;
	} else {
		gamma = - lambda * lambda / (2*c);
	}
	return f1 + gamma;
}


/**
   \brief Approximate max (f1,f2) via a power function.
	
	We have:
	\f[
	f(x) = \max \{f_1(x), f_2(x)\} = \lim_{p \rightarrow \infty} (f_1(x)^p, f_2(x)^p)^{1/p}.
	\f]

	The advantage of this function is that it is
	differentiable. Unlike SmoothMaxGamma, the error is not fixed, but it is proportional to the ratio between.
*/
real SmoothMaxPNorm (real f1, real f2, real p)
{
	assert(p>0);
	return (real) pow(pow(f1,p)+pow(f2,p),1/p);
}
	
/// Normalise a vector to a destination vector (low level)
///
/// src is the source vector.
/// dst is the destination vector.
/// n_elements is the number of elements. 
/// As pointers are raw, make sure n_elements is correct.
/// It is safe for src and dst to point at the same vector.
void Normalise (real* src, real* dst, int n_elements)
{
	real sum = 0.0f;
	int i;
	for (i=0; i<n_elements; i++) {
		sum += src[i];
	}
	if (sum==0) {
		for (i=0; i<n_elements; i++) {
			dst[i] = src[i];
		}
		return;
	}
	assert(sum>0);
	for (i=0; i<n_elements; i++) {
		dst[i] = src[i]/sum;
	}
}

real SquareNorm (real* a, real* b, int n)
{
	real sum = 0;
	for (int i=0; i<n; i++) {
		register real d = (*a++) - (*b++);
		sum += d*d;
	}
	return sum;
}

real EuclideanNorm (real* a, real* b, int n)
{
	real sum = 0;
	for (int i=0; i<n; i++) {
		register real d = (*a++) - (*b++);
		sum += d*d;
	}
	return (real) sqrt(sum);
}

real LNorm (real* a, real* b, int n, real p)
{
	real sum = 0;
	for (int i=0; i<n; i++) {
		register real d = (*a++) - (*b++);
		sum += (real) pow(d,p);
	}
	return (real) pow((double)sum,1.0/p);
}

real Sum (real* a, int n)
{
	real sum = 0;
	for (register int i=0; i<n; i++) {
		sum += *a++;
	}
	return sum;
}
