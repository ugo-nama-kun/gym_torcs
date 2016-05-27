/***************************************************************************

    file                 : strategy.cpp
    created              : Wed Sep 22 15:32:21 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: strategy.cpp,v 1.9 2006/01/10 16:52:46 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	Very simple stategy sample implementation.
*/


#include "strategy.h"
#include "opponent.h"

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif

const float SimpleStrategy::MAX_FUEL_PER_METER = 0.0008f;	// [kg/m] fuel consumtion.
const int SimpleStrategy::PIT_DAMMAGE = 1000;				// [-]


SimpleStrategy::SimpleStrategy() :
	fuelchecked(false),
	fuelperlap(0.0),
	lastpitfuel(0.0)
{
	// Nothing so far.
}


SimpleStrategy::~SimpleStrategy()
{
	// Nothing so far.
}


// Trivial strategy: fill in as much fuel as required for the whole race, or if the tank is
// too small fill the tank completely.
void SimpleStrategy::setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s)
{
	// Load and set parameters. An estimate of fuel per meter is used
	// to calculate necessary fuel. This is overridden by a fuel per
	// lap estimate, if available.
	float fuel_per_meter = GfParmGetNum (*carParmHandle, OLETHROS_SECT_PRIV,
										 OLETHROS_ATT_FUELCONSUMPTION,
										 (char*) NULL,
										 MAX_FUEL_PER_METER);

	float fuel = GfParmGetNum(*carParmHandle, OLETHROS_SECT_PRIV, OLETHROS_ATT_FUELPERLAP, (char*) NULL, t->length*fuel_per_meter);

	expectedfuelperlap = fuel;
	float maxfuel = GfParmGetNum(*carParmHandle, SECT_CAR, PRM_TANK, (char*) NULL, 100.0);
	fuel *= (s->_totLaps + 1.0);
	lastfuel = MIN(fuel, maxfuel);
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*) NULL, lastfuel);
}


void SimpleStrategy::update(tCarElt* car, tSituation *s)
{
	// Fuel statistics update.
	int id = car->_trkPos.seg->id;
	// Range must include enough segments to be executed once guaranteed.
	if (id >= 0 && id < 5 && !fuelchecked) {
		if (car->race.laps > 1) {
			fuelperlap = MAX(fuelperlap, (lastfuel+lastpitfuel-car->priv.fuel));
		}
		lastfuel = car->priv.fuel;
		lastpitfuel = 0.0;
		fuelchecked = true;
	} else if (id > 5) {
		fuelchecked = false;
	}
}


bool SimpleStrategy::needPitstop(tCarElt* car, tSituation *s, Opponents* opponents)
{
	// Do we need to refuel?
	int laps = car->_remainingLaps-car->_lapsBehindLeader;
	if (laps > 0) {
		float cmpfuel = (fuelperlap == 0.0) ? expectedfuelperlap : fuelperlap;
		if (car->_fuel < 1.5*cmpfuel &&
			car->_fuel < laps*cmpfuel)
		{
				return true;
		}
	}

	// Do we need to repair?
	if (car->_dammage > PIT_DAMMAGE) {
		return true;
	}

	return false;
}


float SimpleStrategy::pitRefuel(tCarElt* car, tSituation *s)
{
	float fuel;
	float cmpfuel = (fuelperlap == 0.0) ? expectedfuelperlap : fuelperlap;
	fuel = MAX(MIN((car->_remainingLaps+1.0)*cmpfuel - car->_fuel,
					car->_tank - car->_fuel),
			   0.0);
	lastpitfuel = fuel;
	return fuel;
}


int SimpleStrategy::pitRepair(tCarElt* car, tSituation *s)
{
	return car->_dammage;
}

float SimpleStrategy::getSpeedFactor(tCarElt* car, tSituation* s, Opponents* opponents)
{
	return 1.0f;
}


ManagedStrategy::ManagedStrategy() : SimpleStrategy()
{
	speed_factor = 1.0f;
}


ManagedStrategy::~ManagedStrategy()
{
}


bool ManagedStrategy::needPitstop(tCarElt* car, tSituation *s, Opponents* opponents)
{
	// Do we need to refuel?
	int laps = car->_remainingLaps-car->_lapsBehindLeader;
	if (laps > 0) {
		float cmpfuel = (fuelperlap == 0.0) ? expectedfuelperlap : fuelperlap;
		if (car->_fuel < 1.5*cmpfuel &&
			car->_fuel < laps*cmpfuel)
		{
				return true;
		}
	}

	return RepairDamage(car, opponents);
}

/**
   \brief Should we repair the damage or not?

   Calculate the probability of being overtaken (meaning that we lose
   a position or fail to gain one), given some information
   about the race. We denote this as \f$P(O=1|\cdot)\f$

   We define the probability of being overtaken given the current
   damage as \f$P(O=1|D=d) = d/d_{max}\f$. We define the decision to
   make a pit stop with the random variable \f$S\f$. This allows us to
   further define \f$E[D|R=0]=d, E[D|R=1]=0\f$. The probability of not
   being overtaken depending on whether we take a pitstop or not (and
   disregarding opponents) is
   
   \f[
   P(O=0|D=d, R=0) = 1-d/d_{max}
   \f]

   \f[
   P(O=0|D=d, R=1) = 1
   \f]

   Then we define


   \f$P(O=1|T=t_i, L=l, R=1)\f$ as the probability of not losing a
   position to opponent i if we take a pitstop with l raps remaining as

   \f[
   P(O=0|t_i, l, R=1) = 1 - e^{-\beta t_i l},
   \f]

   \f[
   P(O=0|t_i, l, R=0) = 1
   \f]

   where \f$\beta\f$ is some variable, which has a constant value of
   0.01 in this implementation.
   
   The probability of not being overtaken by any opponent is
   \f[
   P(O=0|\mathcal{T}, l) = \prod_i  \big(P(O=0|t_i,l,R=0) P(R=0) +  P(O=0|t_i,l,R=1) P(R=1)\big)
   \f]

   We can write that as

   \f[
   P(O=0|\mathcal{T}, l) = \prod_i \big((1-R) +  (1-e^{-\beta t_i l}) R \big).
   \f]
   
   Finally, we have 

   \f[
   P(O=1|R=0, d, \mathcal{T}, l) = 1 - (1-d/d_{max}) =  d/d_max
   \f]

   \f[
   P(O=1|R=1, d, \mathcal{T}, l) = 1 - \prod_i (1-e^{-\beta t_i l})
   \f]
   
   We then take the decision with the lowest probability of being
   overtaken.

 */
bool ManagedStrategy::RepairDamage(tCarElt* car, Opponents* opponents)
{
	if (car->_dammage < PIT_DAMMAGE) {
		return false;
	}
	
	double PR0 = ((double) car->_dammage - PIT_DAMMAGE)/10000.0f;
	double PR1 = 1.0f;
	double laps = (double) (car->_remainingLaps - car->_lapsBehindLeader);
	double catchtime; 
	
	if (laps < 1.0f) {
		return false;
	}

	double margin = 30.0f / laps;
	
	if (car->_pos != 1) {
		catchtime = car->_timeBehindLeader;
		PR1 *= 1.0f/(1.0f + exp(-0.1f*(catchtime-margin)));
		//PR1 *= (1-exp(-0.01*(catchtime+laps)));
		if (car->_pos!=2) {
			catchtime = (double) car->_timeBehindPrev; 
			PR1 *= 1.0f/(1.0f + exp(-0.1f*(catchtime-margin)));
		}
	}
	if (opponents->getNOpponentsBehind() != 0) {
		catchtime = car->_timeBeforeNext;
		PR1 *= 1.0f/(1.0f + exp(-0.1f*(catchtime-margin)));
	}
	

	PR1 = 1.0 - PR1; // convert to prob. of getting passed

	// Do we need to refuel?
	if (laps > 0) {
		float cmpfuel = (fuelperlap == 0.0) ? expectedfuelperlap : fuelperlap;
		cmpfuel *= laps; // needed fuel.
		int needed_pitstops = (int) (1.0f + floor (cmpfuel-car->_fuel)/car->_tank);
		int needed_pitstops_now = (int) (2.0f + floor (cmpfuel-car->_tank)/car->_tank);
		// if by stopping now we get same number of pitstops,
		// lower the probability of getting passed.
		if (needed_pitstops_now==needed_pitstops) {
			//printf ("stops: %d %d %f\n", needed_pitstops, needed_pitstops_now, cmpfuel);
			PR1 *= 0.1f;
		}
	}



	if (PR1 < PR0) {
		//printf ("%f %f %d %d %d\n", PR1, PR0, opponents->getNOpponentsBehind(), opponents->getNOpponentsInFront(), opponents->getNOpponents());

		return true;
	} 
	return false;
}

float ManagedStrategy::getSpeedFactor(tCarElt* car, tSituation* s, Opponents* opponents)
{
	float new_speed_factor = speed_factor;

	if (car->_pos == 1 && opponents->getNOpponentsBehind() != 0) {
		float catchtime = (float) car->_timeBeforeNext;
		float margin = 30.0f;
		if (catchtime > margin) {
			float d = 0.02f * (margin - catchtime);
			float a = exp(-d*d);
			new_speed_factor = a + 0.9f * (1.0f-a);
		}
		
		if (fabs (new_speed_factor - speed_factor) > 0.01f) {
			//printf ("SF: %f -> %f\n", speed_factor, new_speed_factor);
			speed_factor = new_speed_factor;
		}
	}

	return speed_factor;
}


#ifdef USE_OLETHROS_NAMESPACE
}
#endif
