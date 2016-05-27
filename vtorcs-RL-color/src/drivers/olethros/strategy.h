// -*- Mode: c++ -*-
/***************************************************************************

    file                 : strategy.h
    created              : Wed Sep 22 15:31:51 CET 2004
    copyright            : (C) 2004 Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: strategy.h,v 1.9.2.1 2008/11/09 17:50:20 berniw Exp $

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

// forward declarations
class Opponents;

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif

/** Some type of strategy */
class AbstractStrategy {
public:

	virtual ~AbstractStrategy() {};
	/// Set Initial fuel at race start.
	virtual void setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s) = 0;
	/// Update internal data at every timestep.
	virtual void update(tCarElt* car, tSituation *s) = 0;
	/// Do we need a pit stop? Can be called less frequently.
	virtual bool needPitstop(tCarElt* car, tSituation *s,
							 Opponents* opponents) = 0;
	/// How much to refuel at pit stop.
	virtual float pitRefuel(tCarElt* car, tSituation *s) = 0;
	/// How much repair at pit stop.
	virtual int pitRepair(tCarElt* car, tSituation *s) = 0;
	/// Speed factor can depend on fuel economy needs, overtaking.. etc.
	virtual float getSpeedFactor(tCarElt* car, tSituation* s, Opponents* opponents) = 0;
};

/// A naive strategy
class SimpleStrategy : public AbstractStrategy {
public:
	SimpleStrategy();
	~SimpleStrategy();

	virtual void setFuelAtRaceStart(tTrack* t, void **carParmHandle, tSituation *s);
	virtual void update(tCarElt* car, tSituation *s);
	virtual bool needPitstop(tCarElt* car, tSituation *s, Opponents* opponents);
	virtual float pitRefuel(tCarElt* car, tSituation *s);
	virtual int pitRepair(tCarElt* car, tSituation *s);
	virtual float getSpeedFactor(tCarElt* car, tSituation* s, Opponents* opponents);

protected:
	bool fuelchecked;				// Fuel statistics updated.
	float fuelperlap;				// The maximum amount of fuel we needed for a lap.
	float lastpitfuel;				// Amount refueled, special case when we refuel.
	float lastfuel;					// the fuel available when we cross the start lane.
	float expectedfuelperlap;		// Expected fuel per lap (may be very inaccurate).

	static const float MAX_FUEL_PER_METER;	// [kg/m] fuel consumtion.
	static const int PIT_DAMMAGE;			// If damage > we request a pit stop.
};



/// A strategy taking into account position and time to decide on pitstops.
class ManagedStrategy : public SimpleStrategy
{
public:
	ManagedStrategy();
	virtual ~ManagedStrategy();
	virtual bool needPitstop(tCarElt* car, tSituation *s, Opponents* opponents);
	virtual float getSpeedFactor(tCarElt* car, tSituation* s, Opponents* opponents);
protected:
	float speed_factor;
	virtual bool RepairDamage(tCarElt* car, Opponents* opponents);

};

#ifdef USE_OLETHROS_NAMESPACE
}
#endif


#endif // _STRATEGY_H_


