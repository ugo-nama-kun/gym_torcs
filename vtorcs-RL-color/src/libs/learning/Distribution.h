/* -*- Mode: C++; -*- */
/* VER: $Id: Distribution.h,v 1.3 2005/08/05 09:02:58 berniw Exp $*/
// copyright (c) 2004 by Christos Dimitrakakis <dimitrak@idiap.ch>
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DISTRIBUTION_H
#define DISTRIBUTION_H

#include <learning/real.h>
#include <cmath>
#include <iostream>

void setRandomSeed(unsigned int seed);
real urandom();
real urandom(real min, real max);

/// Probability distribution
class Distribution {
 public:
	Distribution() {}
	virtual ~Distribution() {}
	virtual real generate() = 0; ///< generate a value from this distribution
	virtual real pdf(real x) = 0; ///< return the density at point x
};

/// \brief Parametric distribution.
/// For distribution simply defined via moments-related parameters.
class ParametricDistribution : public Distribution {
public:
	ParametricDistribution() {}
	virtual ~ParametricDistribution() {}
	virtual void setVariance (real var) = 0; ///< set the variance
 	virtual void setMean (real mean) = 0; ///< set the mean
};


/// Discrete probability distribution
class DiscreteDistribution : public Distribution {
public:
	int n_outcomes; ///< number of possible outcomes
	real* p; ///< probabilities of outcomes
	DiscreteDistribution();
	/// Make a discrete distribution with N outcomes
	DiscreteDistribution(int N); 
	virtual ~DiscreteDistribution();
	virtual real generate();
	virtual real pdf(real x);
};

/// Gaussian probability distribution
class NormalDistribution : public ParametricDistribution {
private:
	bool cache;
	real normal_x, normal_y, normal_rho;
public:
	real m; ///< mean
	real s; ///< standard deviation
	NormalDistribution() {m=0.0; s=1.0; cache = false;}
	/// Normal dist. with given mean and std
	NormalDistribution(real mean, real std)
	{
		setMean (mean);
		setVariance (std*std);
	}
	virtual ~NormalDistribution() {std::cout << "Normal dist destroyed\n";}
	virtual real generate();
	virtual real pdf(real x);
	virtual void setVariance (real var) 
	{s = sqrt(var);} 
	virtual void setMean (real mean)
	{m = mean;}
};

/// Uniform probability distribution
class UniformDistribution : public ParametricDistribution {
public:
	real m; ///< mean
	real s; ///< standard deviation
	UniformDistribution() {m=0.0; s=1.0;}
	/// Create a uniform distribution with mean \c mean and standard deviation \c std
	UniformDistribution(real mean, real std)
	{
		setMean (mean);
		setVariance (std*std);
	}
	virtual ~UniformDistribution() {}
	virtual real generate();
	virtual real pdf(real x);
	virtual void setVariance (real var) 
	{s = sqrt(12.0 * var);} 
	virtual void setMean (real mean)
	{m = mean;}
};


/// Laplacian probability distribution
class LaplacianDistribution : public ParametricDistribution {
public:
	real l; ///< lambda
	real m; ///< mean
	LaplacianDistribution() {m=0.0;l=1.0;}
	/// Create a Laplacian distribution with parameter \c lambda
	LaplacianDistribution(real lambda)
	{
		m = 0.0;
		l = lambda;
	}
	LaplacianDistribution(real mean, real var)
	{
		setMean (mean);
		setVariance (var);
	}
	virtual ~LaplacianDistribution() {}
	virtual real generate();
	virtual real pdf(real x);
	virtual void setVariance (real var)
	{l = sqrt(0.5 / var);}
	virtual void setMean (real mean)
	{m = mean;}
};

/// Exponential probability distribution
class ExponentialDistribution : public ParametricDistribution {
public:
	real l; ///< lambda
	real m; ///< mean
	ExponentialDistribution() {m=0.0; l=1.0;}
	/// Create an exponential distribution with parameter \c lambda
	ExponentialDistribution(real lambda)
	{
		l = lambda;
	}
	ExponentialDistribution(real mean, real var)
	{
		setMean(mean);
		setVariance(var);
	}
	virtual ~ExponentialDistribution() {}
	virtual real generate();
	virtual real pdf(real x);
	virtual void setVariance (real var)
	{l = sqrt(1.0 / var);}
	virtual void setMean (real mean)
	{m = mean;}
};

/// Multinomial gaussian probability distribution
class MultinomialGaussian : public Distribution {
public:
	int d; ///< number of dimensions
	MultinomialGaussian() {d=1;}
	/// create a multinomial with \c d dimensions
	MultinomialGaussian(int d)
	{
		this->d = d;
	}
	virtual ~MultinomialGaussian() {}
	virtual real generate();
	virtual real pdf(real x);
};


#endif
