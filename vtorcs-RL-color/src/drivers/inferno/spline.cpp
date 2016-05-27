/***************************************************************************

    file                 : spline.cpp
    created              : Mon Apr 17 13:51:00 CET 2000
    copyright            : (C) 2000-2006 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: spline.cpp,v 1.1.2.1 2008/05/27 22:33:17 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <math.h>
#include <stdlib.h>
#include "spline.h"


/*	solving tridiagonal nxn matrix with Givens-Rotations in linear time O(n)
	[ a1 b1 0   0   0 .......... ]
	[ c1 a2 b2  0   0 ...........]
	[ 0  c2 a3 b3   0 ...........]
	[ ...........................]
	[ ................... b(n-1) ]
	[ ................ c(n-1) an ]
*/

void tridiagonal(int dim, SplineEquationData *tmp, double *x)
{
	double cos, sin, h, t;
	int i;

	dim--;
	tmp[dim].b = 0.0;
	for (i = 0; i < dim; i++) {
		if (tmp[i].c != 0.0) {
			t = tmp[i].a / tmp[i].c;
			sin = 1.0 / sqrt(1.0 + t*t);
			cos = t * sin;
			tmp[i].a = tmp[i].a*cos + tmp[i].c*sin;
			h = tmp[i].b;
			tmp[i].b = h*cos + tmp[i+1].a*sin;
			tmp[i+1].a = -h*sin + tmp[i+1].a*cos;
			tmp[i].c = tmp[i+1].b*sin;
			tmp[i+1].b = tmp[i+1].b*cos;

			h = x[i];
			x[i] = h*cos + x[i+1]*sin;
			x[i+1] = -h*sin + x[i+1]*cos;
		}
	}

	x[dim] = x[dim]/tmp[dim].a;
	x[dim-1] = (x[dim-1] - tmp[dim-1].b*x[dim]) / tmp[dim-1].a;

	for (i = dim - 2; i >= 0; i--) {
		x[i] = (x[i] - tmp[i].b*x[i+1] - tmp[i].c*x[i+2]) / tmp[i].a;
	}
}


/* solving tridiagonal nxn matrix for two vectors with Givens-Rotations in linear time O(n) */
void tridiagonal2(int dim, SplineEquationData2 *tmp)
{
	double cos, sin, h, t;
	int i;

	dim--;
	tmp[dim].b = 0.0;
	for (i = 0; i < dim; i++) {
		if (tmp[i].c != 0.0) {
			t = tmp[i].a / tmp[i].c;
			sin = 1.0 / sqrt(1.0 + t*t);
			cos = t * sin;
			tmp[i].a = tmp[i].a*cos + tmp[i].c*sin;
			h = tmp[i].b;
			tmp[i].b = h*cos + tmp[i+1].a*sin;
			tmp[i+1].a = -h*sin + tmp[i+1].a*cos;
			tmp[i].c = tmp[i+1].b*sin;
			tmp[i+1].b = tmp[i+1].b*cos;

			h = tmp[i].x1;
			tmp[i].x1 = h*cos + tmp[i+1].x1*sin;
			tmp[i+1].x1 = -h*sin + tmp[i+1].x1*cos;

			h = tmp[i].x2;
			tmp[i].x2 = h*cos + tmp[i+1].x2*sin;
			tmp[i+1].x2 = -h*sin + tmp[i+1].x2*cos;
		}
	}

	tmp[dim].x1 = tmp[dim].x1 / tmp[dim].a;
	tmp[dim-1].x1 = (tmp[dim-1].x1 - tmp[dim-1].b*tmp[dim].x1) / tmp[dim-1].a;

	tmp[dim].x2 = tmp[dim].x2 / tmp[dim].a;
	tmp[dim-1].x2 = (tmp[dim-1].x2 - tmp[dim-1].b*tmp[dim].x2) / tmp[dim-1].a;

	for (i = dim - 2; i >= 0; i--) {
		tmp[i].x1 = (tmp[i].x1 - tmp[i].b*tmp[i+1].x1 - tmp[i].c*tmp[i+2].x1) / tmp[i].a;
		tmp[i].x2 = (tmp[i].x2 - tmp[i].b*tmp[i+1].x2 - tmp[i].c*tmp[i+2].x2) / tmp[i].a;
	}
}


/* compute the slopes of the spline points with periodic constraints */
void slopesp(int dim, const double *const x, const double *const y, double *const ys)
{
	SplineEquationData2 *tmp = (SplineEquationData2 *) malloc(sizeof(SplineEquationData2)*dim);
	int i;

	dim--;
	for (i = 0; i < dim; i++) {
		tmp[i].h = x[i+1] - x[i];
		tmp[i].d = (y[i+1]-y[i]) / (tmp[i].h*tmp[i].h);
	}

	for (i = 1; i < dim; i++) {
		tmp[i].a = 2.0/tmp[i-1].h + 2.0/tmp[i].h;
		tmp[i].b = 1.0/tmp[i].h;
		tmp[i].c = tmp[i].b;
		ys[i] = 3.0 * (tmp[i].d+tmp[i-1].d);
	}

	tmp[0].b = 1.0/tmp[0].h;
	tmp[0].c = tmp[0].b;
	tmp[0].a = (2.0*tmp[0].b + 1.0/tmp[dim-1].h);
	tmp[dim-1].a = 2.0/tmp[dim-2].h + 1.0/tmp[dim-1].h;

	for (i = 1;  i < dim; i++) {
		tmp[i].x1 = 0.0; tmp[i].x2 = 3.0 * (tmp[i].d+tmp[i-1].d);
	}

	tmp[0].x1 = 1.0;
	tmp[dim-1].x1 = 1.0;
	tmp[0].x2 = 3.0 * (tmp[0].d+tmp[dim-1].d);

	tridiagonal2(dim, tmp);

	double factor = (tmp[0].x2+tmp[dim-1].x2) / (tmp[0].x1+tmp[dim-1].x1+tmp[dim-1].h);
	for (i = 0; i < dim; i++) {
		ys[i] = tmp[i].x2 - factor*tmp[i].x1;
	}
	ys[dim] = ys[0];

	free(tmp);
}


/* compute the slopes of the spline points with natural constraints */
void slopesn(int dim, const double *const x, const double *const y, double *const ys)
{
	SplineEquationData *tmp = (SplineEquationData *) malloc(sizeof(SplineEquationData)*dim);
	int i;

	dim--;
	for (i = 0; i < dim; i++) {
		tmp[i].h = x[i+1]-x[i];
		tmp[i].d = (y[i+1]-y[i]) / (tmp[i].h*tmp[i].h);
	}

	for (i = 1; i < dim; i++) {
		tmp[i].a = 2.0/tmp[i-1].h + 2.0/tmp[i].h;
		tmp[i].b = 1.0/tmp[i].h;
		tmp[i].c = tmp[i].b;
		ys[i] = 3.0 * (tmp[i].d+tmp[i-1].d);
	}

	tmp[0].b = 1.0/tmp[0].h;
	tmp[0].c = tmp[0].b;
	tmp[0].a = 2.0*tmp[0].b;
	tmp[dim].a = 2.0/tmp[dim-1].h;
	ys[0] = 3.0*tmp[0].d;
	ys[dim] = 3.0*tmp[dim-1].d;

	tridiagonal(dim+1, tmp, ys);

	free(tmp);
}


/* compute the slopes for 2-dim curve, sums euclidian distances as parameter, periodic */
void parametricslopesp(
	int dim,
	const double *const x,
	const double *const y,
	double *const xs,
	double *const ys,
	double *const s
)
{
	s[0] = 0.0;
	for (int i = 1; i < dim; i++) {
		s[i] = s[i-1] + sqrt((x[i]-x[i-1])*(x[i]-x[i-1]) + (y[i]-y[i-1])*(y[i]-y[i-1]));
	}
	slopesp(dim, s, x, xs);
	slopesp(dim, s, y, ys);
}


/* compute the slopes for 2-dim curve, sums euclidian distances as parameter, natural */
void parametricslopesn(
	int dim,
	const double *const x,
	const double *const y,
	double *const xs,
	double *const ys,
	double *const s
)
{
	s[0] = 0.0;
	for (int i = 1; i < dim; i++) {
		s[i] = s[i-1] + sqrt((x[i]-x[i-1])*(x[i]-x[i-1]) + (y[i]-y[i-1])*(y[i]-y[i-1]));
	}
	slopesn(dim, s, x, xs);
	slopesn(dim, s, y, ys);
}


/* compute the y value for a given z */
double spline(
	int dim,
	double z,
	const double *const x,
	const double *const y,
	const double *const ys
)
{
	int i, a, b;
	double t, a0, a1, a2, a3, h;

	a = 0; b = dim-1;
	do {
		i = (a + b) / 2;
		if (x[i] <= z) {
			a = i;
		} else {
			b = i;
		}
	} while ((a + 1) != b);
    i = a;
	h = x[i+1] - x[i];
	t = (z-x[i]) / h;
	a0 = y[i];
	a1 = y[i+1] - a0;
	a2 = a1 - h*ys[i];
	a3 = h * ys[i+1] - a1;
	a3 -= a2;
	return a0 + (a1 + (a2 + a3*t) * (t-1.0))*t;
}

