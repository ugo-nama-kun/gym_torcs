/***************************************************************************

    file                 : strategy.cpp
    created              : Wed Sep 22 15:32:21 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: strategy.cpp,v 1.1.2.1 2008/05/28 21:34:45 berniw Exp $

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

const float SimpleStrategy::MAX_FUEL_PER_METER = 0.0008f;	// [kg/m] fuel consumtion.
const int SimpleStrategy::PIT_DAMMAGE = 5000;				// [-]


SimpleStrategy::SimpleStrategy() :
	m_fuelchecked(false),
	m_fuelperlap(0.0f),
	m_lastpitfuel(0.0f),
	m_fuelsum(0.0f)
{
	// Nothing so far.
}


SimpleStrategy::~SimpleStrategy()
{
	// Nothing so far.
}


// Trivial strategy: fill in as much fuel as required for the whole race, or if the tank is
// too small fill the tank completely.
void SimpleStrategy::setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index)
{
	// Load and set parameters.
	float fuel = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_FUELPERLAP, (char*) NULL, t->length*MAX_FUEL_PER_METER);
	m_expectedfuelperlap = fuel;
	float maxfuel = GfParmGetNum(*carParmHandle, SECT_CAR, PRM_TANK, (char*) NULL, 100.0f);
	fuel *= (s->_totLaps + 1.0f);
	m_lastfuel = MIN(fuel, maxfuel);
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*) NULL, m_lastfuel);
}


void SimpleStrategy::update(tCarElt* car, tSituation *s)
{
	// Fuel statistics update.
	int id = car->_trkPos.seg->id;
	// Range must include enough segments to be executed once guaranteed.
	if (id >= 0 && id < 5 && !m_fuelchecked) {
		if (car->race.laps > 1) {
			m_fuelperlap = MAX(m_fuelperlap, (m_lastfuel+m_lastpitfuel-car->priv.fuel));
			m_fuelsum += (m_lastfuel+m_lastpitfuel-car->priv.fuel);
		}
		m_lastfuel = car->priv.fuel;
		m_lastpitfuel = 0.0;
		m_fuelchecked = true;
	} else if (id > 5) {
		m_fuelchecked = false;
	}
}


bool SimpleStrategy::needPitstop(tCarElt* car, tSituation *s)
{
	// Question makes only sense if there is a pit.
	if (car->_pit != NULL) {	
		// Do we need to refuel?
		int laps = car->_remainingLaps - car->_lapsBehindLeader;
		if (laps > 0) {
			float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
			// TODO: Investigate if buggy for two pit stops in one lap, BUG?
			if (car->_fuel < 1.5*cmpfuel &&
				car->_fuel < laps*cmpfuel)
			{
					return true;
			}
		}
	
		// Do we need to repair and the pit is free?
		if (car->_dammage > PIT_DAMMAGE && isPitFree(car)) {
			return true;
		}
	}
	return false;
}


bool SimpleStrategy::isPitFree(tCarElt* car)
{
	if (car->_pit != NULL) {	
		if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE) {
			return true;
		}
	}
	return false;
}


float SimpleStrategy::pitRefuel(tCarElt* car, tSituation *s)
{
	float fuel;
	float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
	fuel = MAX(MIN((car->_remainingLaps + 1.0f)*cmpfuel - car->_fuel,
					car->_tank - car->_fuel),
			   0.0f);
	m_lastpitfuel = fuel;
	return fuel;
}


int SimpleStrategy::pitRepair(tCarElt* car, tSituation *s)
{
	return car->_dammage;
}


void SimpleStrategy2::update(tCarElt* car, tSituation *s)
{
	// Fuel statistics update.
	int id = car->_trkPos.seg->id;
	// Range must include enough segments to be executed once guaranteed.
	if (id >= 0 && id < 5 && !m_fuelchecked) {
		if (car->race.laps > 1) {
			//m_fuelperlap = MAX(m_fuelperlap, (m_lastfuel + m_lastpitfuel - car->priv.fuel));
			m_fuelsum += (m_lastfuel + m_lastpitfuel - car->priv.fuel);
			m_fuelperlap = (m_fuelsum/(car->race.laps - 1));
			// This is here for adding strategy decisions, otherwise it could be moved to pitRefuel
			// for efficiency.
			updateFuelStrategy(car, s);
		}
		m_lastfuel = car->priv.fuel;
		m_lastpitfuel = 0.0;
		m_fuelchecked = true;
	} else if (id > 5) {
		m_fuelchecked = false;
	}
}


void SimpleStrategy2::setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index)
{
	// Load and set parameters.
	float fuel = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_FUELPERLAP, (char*) NULL, t->length*MAX_FUEL_PER_METER);
	m_expectedfuelperlap = fuel;
	// Pittime is pittime without refuel.
	m_pittime = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_PITTIME, (char*) NULL, 25.0f);
	m_bestlap = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_BESTLAP, (char*) NULL, 87.0f);
	m_worstlap = GfParmGetNum(*carParmHandle, BT_SECT_PRIV, BT_ATT_WORSTLAP, (char*) NULL, 87.0f);
	float maxfuel = GfParmGetNum(*carParmHandle, SECT_CAR, PRM_TANK, (char*) NULL, 100.0f);

	// Fuel for the whole race.
	float fuelforrace = (s->_totLaps + 1.0f)*fuel;
	// Estimate minimum number of pit stops, -1 because the tank can be filled at the start.
	int pitstopmin = int(ceil(fuelforrace/maxfuel) - 1.0f);
	// Compute race times for min to min + 9 pit stops.
	int i;
	float mintime = FLT_MAX;
	int beststops = pitstopmin;
	m_lastfuel = maxfuel;
	for (i = 0; i < 10; i++) {
		float stintfuel = fuelforrace/(pitstopmin + i + 1);
		float fillratio = stintfuel/maxfuel;
		float avglapest = m_bestlap + (m_worstlap - m_bestlap)*fillratio;
		float racetime = (pitstopmin + i)*(m_pittime + stintfuel/8.0f) + s->_totLaps*avglapest;
		if (mintime > racetime) {
			mintime = racetime;
			beststops = i + pitstopmin;
			m_lastfuel = stintfuel;
			m_fuelperstint = stintfuel;
		}
	}

	m_remainingstops = beststops;
	// Add fuel dependent on index to avoid fuel stop in the same lap.
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*) NULL, m_lastfuel + index*m_expectedfuelperlap);
}


void SimpleStrategy2::updateFuelStrategy(tCarElt* car, tSituation *s)
{
	// Required additional fuel for the rest of the race. +1 because the computation happens right after
	// crossing the start line.
	float requiredfuel = ((car->_remainingLaps + 1) - ceil(car->_fuel/m_fuelperlap))*m_fuelperlap;
	if (requiredfuel < 0.0f) {
		// We have enough fuel to end the race, no further stop required.
		return;
	}

	// Estimate minimum number of minimum remaining pit stops.
	int pitstopmin = int(ceil(requiredfuel/car->_tank));
	if (pitstopmin < 1) {
		// Should never come here becuase of the above test, leave it anyway.
		return;
	}

	// Compute race times for min to min + 8 pit stops.
	int i;
	float mintime = FLT_MAX;
	int beststops = pitstopmin;
	for (i = 0; i < 9; i++) {
		float stintfuel = requiredfuel/(pitstopmin + i);
		float fillratio = stintfuel/car->_tank;
		float avglapest = m_bestlap + (m_worstlap - m_bestlap)*fillratio;
		float racetime = (pitstopmin + i)*(m_pittime + stintfuel/8.0f) + car->_remainingLaps*avglapest;
		if (mintime > racetime) {
			mintime = racetime;
			beststops = i + pitstopmin;
			m_fuelperstint = stintfuel;
		}
	}

	m_remainingstops = beststops;
}


SimpleStrategy2::~SimpleStrategy2()
{
	// Nothing so far.
}


float SimpleStrategy2::pitRefuel(tCarElt* car, tSituation *s)
{
	float fuel;
	if (m_remainingstops > 1) {
		fuel = MIN(m_fuelperstint, car->_tank - car->_fuel);
		m_remainingstops--;
	} else {
		float cmpfuel = (m_fuelperlap == 0.0f) ? m_expectedfuelperlap : m_fuelperlap;
		fuel = MAX(MIN((car->_remainingLaps + 1.0f)*cmpfuel - car->_fuel,
			       car->_tank - car->_fuel),
				   0.0f);
	}

	m_lastpitfuel = fuel;
	return fuel;
}

