// -*- Mode: c++ -*-
/***************************************************************************

    file                 : geometry.h
    created              : Mon Nov 22 0:10:00 CET 2004
    copyright            : (C) 2004 by Christos Dimitrakakis
    email                : dimitrak@idiap.ch
    version              : $Id: geometry.cpp,v 1.9 2006/01/06 15:22:29 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "geometry.h"
#include <cstdlib>
#include <cmath>
#include <cstdio>
#include <exception>
#include <stdexcept>
#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif


Vector::Vector(int N_, enum BoundsCheckingStatus check)
{
	
	n = N_;
	maxN = n;
	if (n==0) {
		x = NULL;
	} else {
		x = (float*) malloc(sizeof(float)*n);
		for (int i=0; i<n; i++) {
			x[i] = 0.0;
		}
	}
	checking_bounds = check;
}

Vector::Vector (const Vector& rhs)
{
	n = rhs.n;
	maxN = n;
	if (n==0) {
		x = NULL;
	} else {
		x = (float*) malloc(sizeof(float)*n);
		for (int i=0; i<n; i++) {
			x[i] = rhs[i];
		}
	}
	checking_bounds = rhs.checking_bounds;
}

Vector::~Vector()
{
	if (x) {
		free(x);
	}
}

Vector& Vector::operator= (const Vector& rhs)
{
	Resize(rhs.n);
	for (int i=0; i<n; i++) {
		x[i] = rhs[i];
	}
	return *this;
}

float& Vector::operator[] (int index)
{
	if (checking_bounds) {
		if ((index<0) || (index>=n)) {
			throw std::out_of_range("index out of range");
		}
	}
	return x[index];
}
const float& Vector::operator[] (int index) const
{
	if (checking_bounds) {
		if ((index<0) || (index>=n)) {
			throw std::out_of_range("index out of range");
		}
	}
	return x[index];
}

void Vector::Resize(int N_)
{
	n = N_;
	if (n>maxN) {
		if (n==0) {
			x = (float*) malloc (sizeof(float)*n);
		} else {
			x = (float*) realloc(x, sizeof(float)*n);
		}
		maxN = n;
	}
}

float DotProd(Vector* A, Vector* B)
{
	int n = A->n;
	float sum = 0.0;
	for (int i=0; i<n; i++) {
		sum += A->x[i]*B->x[i];
	}
	return sum;
}
void Sub (Vector* A, Vector* B, Vector* R)
{
	for (int i=0; i<A->n; i++) {
		R->x[i] = A->x[i] - B->x[i];
	}
}
ParametricLine::ParametricLine (Vector* A, Vector* B)
{
	int N = A->n;
    R = new Vector(N);
    Q = new Vector(N);
	for (int i=0; i<N; i++) {
		R->x[i] = A->x[i];
		Q->x[i] = B->x[i] - R->x[i];
	}
}

ParametricLine::~ParametricLine()
{
	delete Q;
	delete R;
}

void ParametricLine::PointCoords (float t, Vector* X)
{
	for (int i=0; i<X->n; i++) {
		X->x[i] = Q->x[i] + t*R->x[i];
	}
}


Vector* GetNormalToLine(Vector* R)
{
	int N = R->Size();

	Vector* Q = new Vector(N);
	int i;
	int index = 0;
	for (i=0; i<N; i++) {
		if ((*R)[i]!=0.0f) {
			index = i;
			break;
		}
	}

	float sum = 0.0f;

	for (i=0; i<N; i++) {
		if (i!=index) {
			sum += (*R)[i];
			(*Q)[i] = 1.0f;
		}
	}
	(*Q)[index] = -sum/(*R)[index];
	float Z = sqrt(DotProd (Q,Q));
	for (i=0; i<N; i++) {
		(*Q)[i] /= Z;
	}
	return Q;
}


float IntersectLineLine(ParametricLine* A, ParametricLine* B)
{
	int N = A->R->n;
	Vector b(N);
	Sub (B->R, A->R, &b);
	Vector* a = A->Q;
	//Vector* b = D;
	Vector* c = B->Q;

	for (int i=0; i<N; i++) {
		for (int j=0; j<N; j++) {
			if (i!=j) {
				if ((*c)[i]==0.0f) {
					if ((*a)[i]!=0.0f) {
						float t = -b[i]/(*a)[i];
						return t;
					}
				} else {
					float d = (*c)[j]*(*a)[i] - (*c)[i]*(*a)[j];
					if (d!=0.0f) {
						float t = ((*c)[j]*b[i] - (*c)[i]*b[j]) / d;
						return t;
					}
				} // ci==0
			} // i!=j
		} // for j
	} // for i
	return 0.0f;
}

Vector* IntersectSphereLine(ParametricLine* line, Vector* C, float r)
{
	int N=C->n;
	Vector D(N);
	Sub (line->R, C, &D);
	float a = DotProd (line->Q, line->Q);
	float b = 2.0*DotProd (line->Q, &D);
	float c = DotProd(&D, &D) - r*r;
	Vector* t = new Vector(0);
	if (a==0) {
		t->Resize(1);
		t->x[0] = -c/b;
	} else {
		float q = b*b - 4*a*c;
		if (q==0) {
			t->Resize(1);
			t->x[0] = -b/(2*a);
		} else if (q>0) {
			t->Resize(2);
			t->x[0] = (sqrt(q)-b)/(2*a);
			t->x[1] = (-sqrt(q)-b)/(2*a);
			//printf ("%f %f\n", t->x[0], t->x[1]);
		} else {
			//fprintf (stderr, "no solutions: q=%f\n", q);
		}
	}
	return t;
}

float CalculateRadiusPoints (std::vector<Vector> P)
{
	int K = P.size();
	if (K!=3) {
		printf ("K=%d\n", K);
		throw std::invalid_argument ("P has size !=3");
	}
	int N = P[0].Size();

	ParametricLine W(&P[0], &P[1]);
	Vector* A = GetNormalToLine (W.Q);
	delete W.Q;
	W.Q = A;
	ParametricLine Z = ParametricLine(&P[1], &P[2]);
	Vector* B = GetNormalToLine (Z.Q);
	delete Z.Q;
	Z.Q = B;
	int i;
	for (i=0; i<N; i++) {
		(*W.R)[i] = (P[0][i] + P[1][i])/2.0f;
		(*Z.R)[i] = (P[1][i] + P[2][i])/2.0f;
	}
	float t = IntersectLineLine (&W, &Z);
	Vector C(N);
	for (i=0; i<N; i++) {
		C[i] = t*(*W.Q)[i] + (*W.R)[i];
	}
	float r = 0.0f;
	for (int k=0; k<K; k++) {
		float l = 0.0f;
		for (int i=0; i<N; i++) {
			float d = P[k][i] - C[i];
			l += d*d;
		}
		r += sqrt(l);
	}
	r /= (float) K;
	return r;
}


ParametricSphere::ParametricSphere(Vector* C, float r)
{
	int N = C->n;
	this->C = new Vector(N);
	for (int i=0; i<N; i++) {
		this->C->x[i] = C->x[i];
	}
	this->r = r;
}
ParametricSphere::ParametricSphere(int N)
{
	C = new Vector(N);
	r = 0;
}

ParametricSphere::~ParametricSphere()
{
	delete C;
}

/** Estimate an optimally fitting sphere.
 *
 */
void EstimateSphere (std::vector<Vector> P, ParametricSphere* sphere)
{
	int N=P.size();
	if (N<=0) {
		throw std::invalid_argument ("P has size <=0 ");
	}
	
	int d=P[0].n;
	int iter = 1000;
	float a = 0.001f;
	float delta_prev = 100;
	float converge = 1.0;
	Vector mean(d);
	float** Q = new float* [N];
	float* Q_buffer = new float[N*d];
	int i, j;
	for (j=0; j<N; j++) {
		Q[j] = &Q_buffer[j*d];
	}
	for (i=0; i<d; i++) {
		mean[i] = 0.0;
		for (j=0; j<N; j++) {
			mean[i] += P[j][i];
		}
		mean[i] /= (float) N;
	}
	
	float scale = 0;
	for (i=0; i<d; i++) {
		for (j=0; j<N; j++) {
			Q[j][i] = P[j][i] - mean[i];
			if (fabs(Q[j][i]) > scale) {
				scale = fabs(Q[j][i]);
			}
		}
	}

	for (i=0; i<d; i++) {
		for (j=0; j<N; j++) {
			Q[j][i] /= scale;
		}
	}

	Vector center(d);
	float r = 1.0;//sphere->r / scale;
	for (i=0; i<d; i++) {
		center[i] =  ((*(sphere->C))[i] - mean[i]) / scale;
	}

	for (int n=0; n<iter; n++) {
		float delta_total = 0.0;
		for (int m=0; m<N; m++) {
			for (i=0; i<N; i++) {
				float delta = 0;
				float Er = 0;
				for (j=0; j<d; j++) {
					float de = (Q[i][j]-center[j]);
					Er += de*de;
				}
				delta = a*(Er-r*r);
				for (j=0; j<d; j++) {
					center[j] += delta*center[j];
					center[j] += Q[i][j]*delta;
					r += 2*r*delta;
				}
				delta_total += delta;
			}
			if (std::isnan(r)) {
				for (i=0; i<d; i++) {
					center[i] =  ((*(sphere->C))[i] - mean[i]) / scale;
				}
				r = 1.0;
				a = a*.1;
			}
			//printf ("%f\n", r);
		}
		float aconv=0.5;
		converge = aconv*converge + (1-aconv) * fabs(delta_total-delta_prev)/a;
		delta_prev = delta_total;
		if (converge < 0.0001f) {
			break;
		}
	}
			
	sphere->r = r * scale;
	for (i=0; i<d; i++) {
		(*(sphere->C))[i] = center[i]*scale + mean[i];
	}
	delete [] Q_buffer;
	delete [] Q;
}
