/***************************************************************************

    file                 : strategy.h
    created              : Wed Sep 22 15:31:51 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: strategy.h,v 1.3.2.1 2008/11/09 17:50:19 berniw Exp $

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
	Pit strategy for drivers. It defines an abstract base class, such that one can easily plug in
	different strategies.
*/

#ifndef _STRATEGY_H_
#define _STRATEGY_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "driver.h"

class AbstractStrategy {
	public:

		// Need this empty constructor... do not remove.
		virtual ~AbstractStrategy() {}
		// Set Initial fuel at race start.
		virtual void setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index) = 0;
		// Update internal data at every timestep.
		virtual void update(tCarElt* car, tSituation *s) = 0;
		// Do we need a pit stop? Can be called less frequently.
		virtual bool needPitstop(tCarElt* car, tSituation *s) = 0;
		// How much to refuel at pit stop.
		virtual float pitRefuel(tCarElt* car, tSituation *s) = 0;
		// How much repair at pit stop.
		virtual int pitRepair(tCarElt* car, tSituation *s) = 0;
		// Pit Free?
		virtual bool isPitFree(tCarElt* car) = 0;
};


class SimpleStrategy : public AbstractStrategy {
	public:
		SimpleStrategy();
		~SimpleStrategy();

		void setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index);
		void update(tCarElt* car, tSituation *s);
		bool needPitstop(tCarElt* car, tSituation *s);
		float pitRefuel(tCarElt* car, tSituation *s);
		int pitRepair(tCarElt* car, tSituation *s);
		bool isPitFree(tCarElt* car);

	protected:
		bool m_fuelchecked;				// Fuel statistics updated.
		float m_fuelperlap;				// The maximum amount of fuel we needed for a lap.
		float m_lastpitfuel;			// Amount refueled, special case when we refuel.
		float m_lastfuel;				// the fuel available when we cross the start lane.
		float m_expectedfuelperlap;		// Expected fuel per lap (may be very inaccurate).
		float m_fuelsum;				// all the fuel used.

		static const float MAX_FUEL_PER_METER;	// [kg/m] fuel consumtion.
		static const int PIT_DAMMAGE;			// If damage > we request a pit stop.
};


class SimpleStrategy2 : public SimpleStrategy {
	public:
		~SimpleStrategy2();
		void setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s, int index);
		float pitRefuel(tCarElt* car, tSituation *s);
		void update(tCarElt* car, tSituation *s);
		

	protected:
		int m_remainingstops;
		float m_fuelperstint;
		float m_pittime;				// Expected additional time for pit stop.
		float m_bestlap;				// Best possible lap, empty tank and alone.
		float m_worstlap;				// Worst possible lap, full tank and alone.

		virtual void updateFuelStrategy(tCarElt* car, tSituation *s);
};


#endif // _STRATEGY_H_


