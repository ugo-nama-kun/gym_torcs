/* -*- Mode: C++; -*- */
// VER: $Id: Distribution.cpp,v 1.3 2005/08/05 09:02:57 berniw Exp $
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <learning/Distribution.h>

static const double PI = 3.14159265358979323846;

void setRandomSeed(unsigned int seed)
{
	srand(seed);
}
real urandom()
{
	real x;
	do {
		x = ((real) rand())/((real) (RAND_MAX));
	} while (x==1.0);
	return x;
}

real urandom(real min, real max)
{
	return min + ((max-min)*urandom());
}

real UniformDistribution::generate()
{
	return m + (urandom()-0.5)*s;
}

real UniformDistribution::pdf(real x)
{
	real dx=x-m;
	real sh=.5*s;
	if ((dx > -sh)&&(dx <= sh))
		return 1.0/s;
	return 0.0;
}



// Taken from numerical recipes in C
real NormalDistribution::generate()
{
	if(!cache) {
		normal_x = urandom();
		normal_y = urandom();
		normal_rho = sqrt(-2.0 * log(1.0 - normal_y));
		cache = true;
	} else {
		cache = false;
	}
	
	if (cache) {
		return normal_rho * cos(2.0 * PI * normal_x) * s + m;
	} else {
		return normal_rho * sin(2.0 * PI * normal_x) * s + m;	
	}
}

real NormalDistribution::pdf(real x)
{
	real d = (m-x)/s;
	return exp(-0.5 * d*d)/(sqrt(2.0 * PI) * s);
}

real LaplacianDistribution::generate()
{
	real x = urandom(-1.0, 1.0);
	real absx = fabs (x);
	real sgnx;
	if (x>0.0) {
		sgnx = 1.0;
	} else {
		sgnx = -1.0;
	}
	
	return m + sgnx * log(1.0 - absx) / l;

}

real LaplacianDistribution::pdf(real x)
{
	return 0.5*l * exp (-l*fabs(x-m));
}

real ExponentialDistribution::generate()
{
	real x = urandom();
	return - log (1.0 - x) / l;
}

real ExponentialDistribution::pdf(real x)
{
	real d = x - m;
	if (d>0.0) {
		return l * exp (-l*d);
	}
	return 0.0;
}


DiscreteDistribution::DiscreteDistribution() {
	p = NULL; n_outcomes=0;
}

DiscreteDistribution::DiscreteDistribution(int N) {
	p = NULL;
	n_outcomes = 0;
	p = (real*) malloc (sizeof(real) * N);
	n_outcomes = N;
	real invN = 1.0/((real) N);
	for (int i=0; i<N; i++) {
		p[i] = invN;
	}
}

DiscreteDistribution::~DiscreteDistribution() {
	free (p);
}

real DiscreteDistribution::generate()
{
	real d=urandom();
	real sum = 0.0;
	for (int i=0; i<n_outcomes; i++) {
		sum += p[i];
		if (d < sum) {
			return (real) i;
		}
	}
	return 0.0;
}

real DiscreteDistribution::pdf(real x)
{
	int i=(int) floor(x);
	if ((i>=0)&&(i<n_outcomes)) {
		return p[i];
	} 
	return 0.0;
}
