/***************************************************************************

    file                 : olethros.cpp
    copyright            : (C) 2002-2004 Bernhard Wymann 
                               2004 Christos Dimitrakakis
    email                : dimitrak@idiap.ch
    version              : $Id: olethros.cpp,v 1.10.2.2 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** 
	\file olethros.cpp
	
	\brief This is the main file for the Olethros robot.
	
	The Olethros robot is basically the same as berniw's BT robot with
	added learning capabilities.
*/

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include "driver.h"

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif

#define BUFSIZE 20
#define NBBOTS 10

static const char* botname[NBBOTS] = {
	"olethros 1", "olethros 2", "olethros 3", "olethros 4", "olethros 5",
	"olethros 6", "olethros 7", "olethros 8", "olethros 9", "olethros 10"
};

static const char* botdesc[NBBOTS] = {
	"olethros 1", "olethros 2", "olethros 3", "olethros 4", "olethros 5",
	"olethros 6", "olethros 7", "olethros 8", "olethros 9", "olethros 10"
};

static Driver *driver[NBBOTS];

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void newRace(int index, tCarElt* car, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static int pitcmd(int index, tCarElt* car, tSituation *s);
static void shutdown(int index);
static int InitFuncPt(int index, void *pt);
static void endRace(int index, tCarElt *car, tSituation *s);


// Module entry point.
extern "C" int olethros(tModInfo *modInfo)
{
	int i;

	// Clear all structures.
	memset(modInfo, 0, 10*sizeof(tModInfo));

	for (i = 0; i < NBBOTS; i++) {
		modInfo[i].name    = strdup(botname[i]);	// name of the module (short).
		modInfo[i].desc    = strdup(botdesc[i]);	// Description of the module (can be long).
		modInfo[i].fctInit = InitFuncPt;			// Init function.
		modInfo[i].gfId    = ROB_IDENT;				// Supported framework version.
		modInfo[i].index   = i;						// Indices from 0 to 9.
	}
	return 0;
}


// Module interface initialization.
static int InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;

	// Create robot instance for index.
	driver[index] = new Driver(index);
	itf->rbNewTrack = initTrack;	// Give the robot the track view called.
	itf->rbNewRace  = newRace;		// Start a new race.
	itf->rbDrive    = drive;		// Drive during race.
	itf->rbPitCmd   = pitcmd;		// Pit commands.
	itf->rbEndRace  = endRace;		// End of the current race.
	itf->rbShutdown = shutdown;		// Called before the module is unloaded.
	itf->index      = index;		// Index used if multiple interfaces.
	return 0;
}


// Called for every track change or new race.
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	driver[index]->initTrack(track, carHandle, carParmHandle, s);
}


// Start a new race.
static void newRace(int index, tCarElt* car, tSituation *s)
{
	driver[index]->newRace(car, s);
}


// Drive during race.
static void drive(int index, tCarElt* car, tSituation *s)
{
	driver[index]->drive(s);
}


// Pitstop callback.
static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	return driver[index]->pitCommand(s);
}


// End of the current race.
static void endRace(int index, tCarElt *car, tSituation *s)
{
	driver[index]->endRace(s);
}


// Called before the module is unloaded.
static void shutdown(int index)
{
	delete driver[index];
}

#ifdef USE_OLETHROS_NAMESPACE
}
#endif
