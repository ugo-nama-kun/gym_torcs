/***************************************************************************

    file        : racemain.cpp
    created     : Sat Nov 16 12:13:31 CET 2002
    copyright   : (C) 2002 by Eric Espi�
    email       : eric.espie@torcs.org
    version     : $Id: racemain.cpp,v 1.13 2005/08/17 20:48:39 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/** @file

    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: racemain.cpp,v 1.13 2005/08/17 20:48:39 berniw Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <raceman.h>
#include <robot.h>
#include <racescreens.h>
#include <exitmenu.h>

#include "raceengine.h"
#include "raceinit.h"
#include "racegl.h"
#include "raceresults.h"
#include "racestate.h"
#include "racemanmenu.h"

#include "racemain.h"

static char buf[1024];
static char path[1024];
static char path2[1024];


/***************************************************************/
/* ABANDON RACE HOOK */

static void *AbandonRaceHookHandle = 0;

static void
AbandonRaceHookActivate(void * /* vforce */)
{
	// Shutdown current event.
	ReEventShutdown();

	/* Return to race menu */
	ReInfo->_reState = RE_STATE_CONFIG;

	GfuiScreenActivate(ReInfo->_reGameScreen);
}

static void *
AbandonRaceHookInit(void)
{
	if (AbandonRaceHookHandle) {
		return AbandonRaceHookHandle;
	}

	AbandonRaceHookHandle = GfuiHookCreate(0, AbandonRaceHookActivate);

	return AbandonRaceHookHandle;
}

static void *AbortRaceHookHandle = 0;

static void
AbortRaceHookActivate(void * /* dummy */)
{
	GfuiScreenActivate(ReInfo->_reGameScreen);

	ReInfo->_reSimItf.shutdown();
	if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL) {
		ReInfo->_reGraphicItf.shutdowncars();
	}
	ReInfo->_reGraphicItf.shutdowntrack();
	ReRaceCleanDrivers();

	FREEZ(ReInfo->_reCarInfo);
	/* Return to race menu */
	ReInfo->_reState = RE_STATE_CONFIG;
}

static void *
AbortRaceHookInit(void)
{
	if (AbortRaceHookHandle) {
		return AbortRaceHookHandle;
	}

	AbortRaceHookHandle = GfuiHookCreate(0, AbortRaceHookActivate);

	return AbortRaceHookHandle;
}

int
ReRaceEventInit(void)
{
	void *params = ReInfo->params;

	if (getTextOnly()==false)
		RmLoadingScreenStart(ReInfo->_reName, "data/img/splash-qrloading.png");
	ReInitTrack();
	if (getTextOnly()==false)
	{
		RmLoadingScreenSetText("Loading Track 3D Description...");
		ReInfo->_reGraphicItf.inittrack(ReInfo->track);
	}
	// GIUSE - VISION HERE!!
	else if (getVision())
	{
//		RmLoadingScreenSetText("Loading Track 3D Description...");
		ReInfo->_reGraphicItf.inittrack(ReInfo->track);
	}
	ReEventInitResults();

	if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1) {
		ReNewTrackMenu();
		return RM_ASYNC | RM_NEXT_STEP;
	}
	return RM_SYNC | RM_NEXT_STEP;
}


int
RePreRace(void)
{
	tdble dist;
	char *raceName;
	char *raceType;
	void *params = ReInfo->params;
	void *results = ReInfo->results;

	raceName = ReInfo->_reRaceName = ReGetCurrentRaceName();
	if (!raceName) {
		return RM_QUIT;
	}

	dist = GfParmGetNum(params, raceName, RM_ATTR_DISTANCE, NULL, 0);
	if (dist < 0.001) {
		ReInfo->s->_totLaps = (int)GfParmGetNum(params, raceName, RM_ATTR_LAPS, NULL, 30);
	} else {
		ReInfo->s->_totLaps = ((int)(dist / ReInfo->track->length)) + 1;
	}
	ReInfo->s->_maxDammage = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DMG, NULL, 10000);

	raceType = GfParmGetStr(params, raceName, RM_ATTR_TYPE, RM_VAL_RACE);
	if (!strcmp(raceType, RM_VAL_RACE)) {
		ReInfo->s->_raceType = RM_TYPE_RACE;
	} else if (!strcmp(raceType, RM_VAL_QUALIF)) {
		ReInfo->s->_raceType = RM_TYPE_QUALIF;
	} else if (!strcmp(raceType, RM_VAL_PRACTICE)) {
		ReInfo->s->_raceType = RM_TYPE_PRACTICE;
	}

	ReInfo->s->_raceState = 0;

	/* Cleanup results */
	sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, raceName);
	GfParmListClean(results, path);

	return RM_SYNC | RM_NEXT_STEP;
}

/* return state mode */
static int
reRaceRealStart(void)
{
	int i, j;
	int sw, sh, vw, vh;
	tRobotItf *robot;
	tReCarInfo *carInfo;
	char *dllname;
	char key[256];
	int foundHuman;
	void *params = ReInfo->params;
	void *results = ReInfo->results;
	tSituation *s = ReInfo->s;

	if (getTextOnly()==false)
		RmLoadingScreenSetText("Loading Simulation Engine...");
	dllname = GfParmGetStr(ReInfo->_reParam, "Modules", "simu", "");
	sprintf(key, "%smodules/simu/%s.%s", GetLibDir (), dllname, DLLEXT);
	if (GfModLoad(0, key, &ReRaceModList)) return RM_QUIT;
	ReRaceModList->modInfo->fctInit(ReRaceModList->modInfo->index, &ReInfo->_reSimItf);

	if (ReInitCars()) {
		return RM_QUIT;
	}

	/* Blind mode or not */
	// GIUSE - VISION HERE!! - or visual based ;)
	if (getTextOnly()==false)
	{
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
		ReInfo->_reGameScreen = ReScreenInit();

		foundHuman = 0;
		for (i = 0; i < s->_ncars; i++) {
			if (s->cars[i]->_driverType == RM_DRV_HUMAN) {
				foundHuman = 1;
				break;
			}
		}
		if (!foundHuman) {
			if (!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_INVISIBLE)) {
				ReInfo->_displayMode = RM_DISP_MODE_NONE;
				ReInfo->_reGameScreen = ReResScreenInit();
			}
		}
	}
	// GIUSE - VISION HERE!!
	else if (getVision())
	{
		ReInfo->_displayMode = RM_DISP_MODE_NORMAL;
		ReInfo->_reGameScreen = ReScreenInit();

//		foundHuman = 0;
//		for (i = 0; i < s->_ncars; i++) {
//			if (s->cars[i]->_driverType == RM_DRV_HUMAN) {
//				foundHuman = 1;
//				break;
//			}
//		}
//		if (!foundHuman) {
//			if (!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPMODE, RM_VAL_VISIBLE), RM_VAL_INVISIBLE)) {
				ReInfo->_displayMode = RM_DISP_MODE_NONE;
				ReInfo->_reGameScreen = ReResScreenInit();
//			}
//		}
  }	else
	{
		ReInfo->_displayMode = RM_DISP_MODE_NONE;
  }

	if (!(ReInfo->s->_raceType == RM_TYPE_QUALIF) ||
	((int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1) == 1))
	{
		if (getTextOnly()==false)
			RmLoadingScreenStart(ReInfo->_reName, "data/img/splash-qrloading.png");
	}

	for (i = 0; i < s->_ncars; i++) {
		sprintf(buf, "Initializing Driver %s...", s->cars[i]->_name);
		RmLoadingScreenSetText(buf);
		robot = s->cars[i]->robot;
		robot->rbNewRace(robot->index, s->cars[i], s);
	}
	carInfo = ReInfo->_reCarInfo;

	ReInfo->_reSimItf.update(s, RCM_MAX_DT_SIMU, -1);
	for (i = 0; i < s->_ncars; i++) {
		carInfo[i].prevTrkPos = s->cars[i]->_trkPos;
	}

	if (getTextOnly()==false)
		RmLoadingScreenSetText("Running Prestart...");
	for (i = 0; i < s->_ncars; i++) {
		memset(&(s->cars[i]->ctrl), 0, sizeof(tCarCtrl));
		s->cars[i]->ctrl.brakeCmd = 1.0;
	}

	for (j = 0; j < ((int)(1.0 / RCM_MAX_DT_SIMU)); j++) {
		ReInfo->_reSimItf.update(s, RCM_MAX_DT_SIMU, -1);
	}

	if (ReInfo->_displayMode != RM_DISP_MODE_NORMAL) {
		if (ReInfo->s->_raceType == RM_TYPE_QUALIF) {
			ReUpdateQualifCurRes(s->cars[0]);
		} else if (getTextOnly()==false) {
			sprintf(buf, "%s on %s", s->cars[0]->_name, ReInfo->track->name);
			ReResScreenSetTitle(buf);
		}
	}

	if (getTextOnly()==false)
		RmLoadingScreenSetText("Ready.");

	ReInfo->_reTimeMult = 1.0;
//	ReInfo->_reLastTime = -1.0;
//	ReInfo->s->currentTime = -2.0;
// GIUSE - let's start immediately, to save time for evolution
	ReInfo->_reLastTime = 0.0;
	ReInfo->s->currentTime = 0.0;
	ReInfo->s->deltaTime = RCM_MAX_DT_SIMU;

	ReInfo->s->_raceState = RM_RACE_STARTING;

	if (getTextOnly()==false)
	{
		GfScrGetSize(&sw, &sh, &vw, &vh);
		ReInfo->_reGraphicItf.initview((sw-vw)/2, (sh-vh)/2, vw, vh, GR_VIEW_STD, ReInfo->_reGameScreen);

		if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL) {
			/* RmLoadingScreenSetText("Loading Cars 3D Objects..."); */
			ReInfo->_reGraphicItf.initcars(s);
		}

		GfuiScreenActivate(ReInfo->_reGameScreen);
	}
  // GIUSE - VISION HERE!!
  else if (getVision())
	{
		GfScrGetSize(&sw, &sh, &vw, &vh);
		ReInfo->_reGraphicItf.initview((sw-vw)/2, (sh-vh)/2, vw, vh, GR_VIEW_STD, ReInfo->_reGameScreen);

//		if (ReInfo->_displayMode == RM_DISP_MODE_NORMAL) {
//			/* RmLoadingScreenSetText("Loading Cars 3D Objects..."); */
			ReInfo->_reGraphicItf.initcars(s);
//		}

//		GfuiScreenActivate(ReInfo->_reGameScreen);
	}

	return RM_SYNC | RM_NEXT_STEP;
}

/***************************************************************/
/* START RACE HOOK */

static void	*StartRaceHookHandle = 0;

static void
StartRaceHookActivate(void * /* dummy */)
{
	reRaceRealStart();
}

static void *
StartRaceHookInit(void)
{
	if (StartRaceHookHandle) {
		return StartRaceHookHandle;
	}

	StartRaceHookHandle = GfuiHookCreate(0, StartRaceHookActivate);

	return StartRaceHookHandle;
}

/* return state mode */
int
ReRaceStart(void)
{
	int i;
	int nCars;
	int maxCars;
	char *prevRaceName;
	char *gridType;
	char *raceName = ReInfo->_reRaceName;
	void *params = ReInfo->params;
	void *results = ReInfo->results;


	FREEZ(ReInfo->_reCarInfo);
	ReInfo->_reCarInfo = (tReCarInfo*)calloc(GfParmGetEltNb(params, RM_SECT_DRIVERS), sizeof(tReCarInfo));

	/* Drivers starting order */
	GfParmListClean(params, RM_SECT_DRIVERS_RACING);
	if (ReInfo->s->_raceType == RM_TYPE_QUALIF) {
		i = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
		if (getTextOnly()==false)
		{
			if (i == 1) {
			RmLoadingScreenStart(ReInfo->_reName, "data/img/splash-qrloading.png");
			RmLoadingScreenSetText("Preparing Starting Grid...");
			} else {
				RmShutdownLoadingScreen();
			}
		}

		sprintf(path, "%s/%d", RM_SECT_DRIVERS, i);
		sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, 1);
		GfParmSetStr(params, path2, RM_ATTR_MODULE, GfParmGetStr(params, path, RM_ATTR_MODULE, ""));
		GfParmSetNum(params, path2, RM_ATTR_IDX, NULL, GfParmGetNum(params, path, RM_ATTR_IDX, NULL, 0));
	} else {
		if (getTextOnly()==false)
		{
			RmLoadingScreenStart(ReInfo->_reName, "data/img/splash-qrloading.png");
			RmLoadingScreenSetText("Preparing Starting Grid...");
		}

		gridType = GfParmGetStr(params, raceName, RM_ATTR_START_ORDER, RM_VAL_DRV_LIST_ORDER);
		if (!strcmp(gridType, RM_VAL_LAST_RACE_ORDER)) {
			/* Starting grid in the arrival of the previous race */
			nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
			maxCars = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DRV, NULL, 100);
			nCars = MIN(nCars, maxCars);
			prevRaceName = ReGetPrevRaceName();
			if (!prevRaceName) {
				return RM_QUIT;
			}
			for (i = 1; i < nCars + 1; i++) {
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, i);
				sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE, GfParmGetStr(results, path, RE_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL, GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
			}
		} else if (!strcmp(gridType, RM_VAL_LAST_RACE_RORDER)) {
			/* Starting grid in the reversed arrival order of the previous race */
			nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
			maxCars = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DRV, NULL, 100);
			nCars = MIN(nCars, maxCars);
			prevRaceName = ReGetPrevRaceName();
			if (!prevRaceName) {
				return RM_QUIT;
			}
			for (i = 1; i < nCars + 1; i++) {
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, prevRaceName, RE_SECT_RANK, nCars - i + 1);
				sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE, GfParmGetStr(results, path, RE_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL, GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0));
			}
		} else {
			/* Starting grid in the drivers list order */
			nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
			maxCars = (int)GfParmGetNum(params, raceName, RM_ATTR_MAX_DRV, NULL, 100);
			nCars = MIN(nCars, maxCars);
			for (i = 1; i < nCars + 1; i++) {
				sprintf(path, "%s/%d", RM_SECT_DRIVERS, i);
				sprintf(path2, "%s/%d", RM_SECT_DRIVERS_RACING, i);
				GfParmSetStr(params, path2, RM_ATTR_MODULE, GfParmGetStr(params, path, RM_ATTR_MODULE, ""));
				GfParmSetNum(params, path2, RM_ATTR_IDX, NULL, GfParmGetNum(params, path, RM_ATTR_IDX, NULL, 0));
			}
		}
	}

	if (!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_SPLASH_MENU, RM_VAL_NO), RM_VAL_YES)) {
		if (getTextOnly()==false)
		{
			RmShutdownLoadingScreen();
			RmDisplayStartRace(ReInfo, StartRaceHookInit(), AbandonRaceHookInit());
		}
    // GIUSE - VISION HERE!!
		if (getVision())
		{
//			RmShutdownLoadingScreen();
			RmDisplayStartRace(ReInfo, StartRaceHookInit(), AbandonRaceHookInit());
		}
		return RM_ASYNC | RM_NEXT_STEP;
	}

	return reRaceRealStart();
}

/***************************************************************/
/* BACK TO RACE HOOK */

static void	*BackToRaceHookHandle = 0;

static void
BackToRaceHookActivate(void * /* dummy */)
{
	ReInfo->_reState = RE_STATE_RACE;
	GfuiScreenActivate(ReInfo->_reGameScreen);
}

static void *
BackToRaceHookInit(void)
{
	if (BackToRaceHookHandle) {
		return BackToRaceHookHandle;
	}

	BackToRaceHookHandle = GfuiHookCreate(0, BackToRaceHookActivate);

	return BackToRaceHookHandle;
}

/***************************************************************/
/* RESTART RACE HOOK */

static void	*RestartRaceHookHandle = 0;

static void
RestartRaceHookActivate(void * /* dummy */)
{
	ReRaceCleanup();
	ReInfo->_reState = RE_STATE_PRE_RACE;
	GfuiScreenActivate(ReInfo->_reGameScreen);
}

static void *
RestartRaceHookInit(void)
{
	if (RestartRaceHookHandle) {
		return RestartRaceHookHandle;
	}

	RestartRaceHookHandle = GfuiHookCreate(0, RestartRaceHookActivate);

	return RestartRaceHookHandle;
}

/***************************************************************/
/* QUIT HOOK */

static void	*QuitHookHandle = 0;
static void	*StopScrHandle = 0;

static void
QuitHookActivate(void * /* dummy */)
{
	if (StopScrHandle) {
		GfuiScreenActivate(TorcsExitMenuInit(StopScrHandle));
	}
}

static void *
QuitHookInit(void)
{
	if (QuitHookHandle) {
		return QuitHookHandle;
	}

	QuitHookHandle = GfuiHookCreate(0, QuitHookActivate);

	return QuitHookHandle;
}

int
ReRaceStop(void)
{
	void	*params = ReInfo->params;

	if (RESTART!=1)
	{

		if (!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_ALLOW_RESTART, RM_VAL_NO), RM_VAL_NO)) {
			StopScrHandle = RmTriStateScreen("Race Stopped",
						"Abandon Race", "Abort current race", AbortRaceHookInit(),
						"Resume Race", "Return to Race", BackToRaceHookInit(),
						"Quit Game", "Quit the game", QuitHookInit());
		} else {
			StopScrHandle = RmFourStateScreen("Race Stopped",
						"Restart Race", "Restart the current race", RestartRaceHookInit(),
						"Abandon Race", "Abort current race", AbortRaceHookInit(),
						"Resume Race", "Return to Race", BackToRaceHookInit(),
						"Quit Game", "Quit the game", QuitHookInit());
		}
	}
	return RM_ASYNC | RM_NEXT_STEP;
}


int
ReRaceEnd(void)
{
	int curDrvIdx;
	void *params = ReInfo->params;
	void *results = ReInfo->results;

	if (getTextOnly()==false)
	{
		ReRaceCleanup();
		if (ReInfo->s->_raceType == RM_TYPE_QUALIF) {
			curDrvIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
			curDrvIdx++;
			if (curDrvIdx > GfParmGetEltNb(params, RM_SECT_DRIVERS)) {
				GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
				return ReDisplayResults();
			}
			GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, curDrvIdx);
			return RM_SYNC | RM_NEXT_RACE;
		}
		return ReDisplayResults();
	}
	// GIUSE - VISION HERE!!
	else if (getVision())
	{
		ReRaceCleanup();
		if (ReInfo->s->_raceType == RM_TYPE_QUALIF) {
			curDrvIdx = (int) 1 + GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
			if (curDrvIdx > GfParmGetEltNb(params, RM_SECT_DRIVERS)) {
				GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);
			}
			else
			{
			  GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, curDrvIdx);
			  return RM_SYNC | RM_NEXT_RACE;
			}
		}
		return RE_STATE_EXIT;
	}
	else
	{
		int i;
		tRobotItf *robot;
		int nCars;

		nCars = ReInfo->s->_ncars;
		for (i = 0; i < nCars; i++) {
			robot = ReInfo->s->cars[i]->robot;
			if (robot->rbShutdown) {
				robot->rbShutdown(robot->index);
			}
			//GfParmReleaseHandle(ReInfo->s->cars[i]->_paramsHandle);
			free(robot);
		}
		return RE_STATE_EXIT;
	}
}


int
RePostRace(void)
{
	int curRaceIdx;
	void *results = ReInfo->results;
	void *params = ReInfo->params;

	//ReUpdateStandings();

	curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	if (curRaceIdx < GfParmGetEltNb(params, RM_SECT_RACES)) {
		curRaceIdx++;
		GfOut("Race Nb %d\n", curRaceIdx);
		GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, curRaceIdx);
		ReUpdateStandings();
		return RM_SYNC | RM_NEXT_RACE;
	}

	ReUpdateStandings();
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	return RM_SYNC | RM_NEXT_STEP;
}


int
ReEventShutdown(void)
{
	int curTrkIdx;
	void *params = ReInfo->params;
	int nbTrk = GfParmGetEltNb(params, RM_SECT_TRACKS);
	int ret = 0;
	void *results = ReInfo->results;

	ReInfo->_reGraphicItf.shutdowntrack();

	int curRaceIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	curTrkIdx = (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1);

	if (curRaceIdx == 1) {
		if (curTrkIdx < nbTrk) {
			// Next track.
			curTrkIdx++;
		} else if (curTrkIdx >= nbTrk) {
			// Back to the beginning.
			curTrkIdx = 1;
		}
	}

	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, curTrkIdx);

	if (curTrkIdx != 1) {
		ret =  RM_NEXT_RACE;
	} else {
		ret =  RM_NEXT_STEP;
	}

	if (nbTrk != 1) {
		ReDisplayStandings();
		return RM_ASYNC | ret;
	}
	FREEZ(ReInfo->_reCarInfo);

	return RM_SYNC | ret;
}
