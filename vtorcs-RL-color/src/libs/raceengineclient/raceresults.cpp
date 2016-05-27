/***************************************************************************

    file        : raceresults.cpp
    created     : Thu Jan  2 12:43:10 CET 2003
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: raceresults.cpp,v 1.9.2.1 2008/05/31 15:55:36 berniw Exp $                                  

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
    @version	$Id: raceresults.cpp,v 1.9.2.1 2008/05/31 15:55:36 berniw Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tgfclient.h>
#include <robot.h>
#include <raceman.h>
#include <racescreens.h>
#include <robottools.h>

#include "racemain.h"
#include "racegl.h"
#include "raceinit.h"
#include "raceengine.h"

#include "raceresults.h"

static char buf[1024];
static char path[1024];
static char path2[1024];

typedef struct
{
	char	*carName;
	char	*modName;
	int		drvIdx;
	int		points;
} tReStandings;

void
ReInitResults(void)
{
	struct tm	*stm;
	time_t	t;
	void	*results;
	
	t = time(NULL);
	stm = localtime(&t);
	sprintf(buf, "%sresults/%s/results-%4d-%02d-%02d-%02d-%02d.xml",
		GetLocalDir(),
		ReInfo->_reFilename,
		stm->tm_year+1900,
		stm->tm_mon+1,
		stm->tm_mday,
		stm->tm_hour,
		stm->tm_min);
	
	ReInfo->results = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	results = ReInfo->results;
	GfParmSetNum(results, RE_SECT_HEADER, RE_ATTR_DATE, NULL, (tdble)t);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_RACE, NULL, 1);
	GfParmSetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_DRIVER, NULL, 1);	
}

void
ReEventInitResults(void)
{
	int		nCars;
	int		i;
	void	*results = ReInfo->results;
	void	*params = ReInfo->params;
	
	nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS);
	for (i = 1; i < nCars + 1; i++) {
		sprintf(path, "%s/%s/%d", ReInfo->track->name, RM_SECT_DRIVERS, i);
		sprintf(path2, "%s/%d", RM_SECT_DRIVERS, i);
		GfParmSetStr(results, path, RE_ATTR_DLL_NAME, GfParmGetStr(params, path2, RM_ATTR_MODULE, ""));
		GfParmSetNum(results, path, RE_ATTR_INDEX, NULL, GfParmGetNum(params, path2, RM_ATTR_IDX, (char*)NULL, 0));
    }
}

void
ReUpdateStandings(void)
{
	int			maxDrv;
	int			curDrv;
	int			runDrv;
	char		*carName;
	char		*modName;
	int			drvIdx;
	int			points;
	int			i, j;
	int			found;
	tReStandings	*standings = 0;
	void		*results = ReInfo->results;
	char		str1[1024], str2[1024];
	
	sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, RE_SECT_RANK);
	
	runDrv = GfParmGetEltNb(results, path);
	curDrv = GfParmGetEltNb(results, RE_SECT_STANDINGS);
	maxDrv = curDrv + runDrv;
	
	standings = (tReStandings *)calloc(maxDrv, sizeof(tReStandings));
	
	/* Read the current standings */
	for (i = 0; i < curDrv; i++) {
		sprintf(path2, "%s/%d", RE_SECT_STANDINGS, i + 1);
		standings[i].carName = strdup(GfParmGetStr(results, path2, RE_ATTR_NAME, 0));
		standings[i].modName = strdup(GfParmGetStr(results, path2, RE_ATTR_MODULE, 0));
		standings[i].drvIdx  = (int)GfParmGetNum(results, path2, RE_ATTR_IDX, NULL, 0);
		standings[i].points  = (int)GfParmGetNum(results, path2, RE_ATTR_POINTS, NULL, 0);
	}

	GfParmListClean(results, RE_SECT_STANDINGS);
	
	for (i = 0; i < runDrv; i++) {
		/* Search the driver in the standings */
		found = 0;
		sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, RE_SECT_RANK, i + 1);
		carName = GfParmGetStr(results, path, RE_ATTR_NAME, 0);
		for (j = 0; j < curDrv; j++) {
			if (!strcmp(carName, standings[j].carName)) {
				found = 1;
				break;
			}
		}

		if (!found) {
			/* Add the new driver */
			curDrv++;
			standings[j].carName = strdup(carName);
			standings[j].modName = strdup(GfParmGetStr(results, path, RE_ATTR_MODULE, 0));
			standings[j].drvIdx  = (int)GfParmGetNum(results, path, RE_ATTR_IDX, NULL, 0);
			standings[j].points  = (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0);
		} else {
			/* Add the new points */
			standings[j].points += (int)GfParmGetNum(results, path, RE_ATTR_POINTS, NULL, 0);
		}
		/* bubble sort... */
		while (j > 0) {
			if (standings[j - 1].points >= standings[j].points) {
				break;
			}
			/* Swap with preceeding */
			carName = standings[j].carName;
			modName = standings[j].modName;
			drvIdx  = standings[j].drvIdx;
			points  = standings[j].points;
		
			standings[j].carName = standings[j - 1].carName;
			standings[j].modName = standings[j - 1].modName;
			standings[j].drvIdx  = standings[j - 1].drvIdx;
			standings[j].points  = standings[j - 1].points;
		
			standings[j - 1].carName = carName;
			standings[j - 1].modName = modName;
			standings[j - 1].drvIdx  = drvIdx;
			standings[j - 1].points  = points;
		
			j--;
		}
	}
	
	/* Store the standing back */
	for (i = 0; i < curDrv; i++) {
		sprintf(path, "%s/%d", RE_SECT_STANDINGS, i + 1);
		GfParmSetStr(results, path, RE_ATTR_NAME, standings[i].carName);
		free(standings[i].carName);
		GfParmSetStr(results, path, RE_ATTR_MODULE, standings[i].modName);
		free(standings[i].modName);
		GfParmSetNum(results, path, RE_ATTR_IDX, NULL, standings[i].drvIdx);
		GfParmSetNum(results, path, RE_ATTR_POINTS, NULL, standings[i].points);
	}
	free(standings);
	
	sprintf(str1, "%sconfig/params.dtd", GetDataDir());
	sprintf(str2, "<?xml-stylesheet type=\"text/xsl\" href=\"file:///%sconfig/style.xsl\"?>", GetDataDir());
	
	GfParmSetDTD (results, str1, str2);
	GfParmWriteFile(0, results, "Results");
}


void
ReStoreRaceResults(char *race)
{
	int		i;
	int		nCars;
	tCarElt	*car;
	tSituation 	*s = ReInfo->s;
	char	*carName;
	void	*carparam;
	void	*results = ReInfo->results;
	void	*params = ReInfo->params;
	
	/* Store the number of laps of the race */
	switch (ReInfo->s->_raceType) {
		case RM_TYPE_RACE:
			car = s->cars[0];
			if (car->_laps > s->_totLaps) car->_laps = s->_totLaps + 1;

			sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race);
			GfParmListClean(results, path);
			GfParmSetNum(results, path, RE_ATTR_LAPS, NULL, car->_laps - 1);
			
			for (i = 0; i < s->_ncars; i++) {
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
				car = s->cars[i];
				if (car->_laps > s->_totLaps) car->_laps = s->_totLaps + 1;
			
				GfParmSetStr(results, path, RE_ATTR_NAME, car->_name);
			
				sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
				carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
				carName = GfParmGetName(carparam);
			
				GfParmSetStr(results, path, RE_ATTR_CAR, carName);
				GfParmSetNum(results, path, RE_ATTR_INDEX, NULL, car->index);
			
				GfParmSetNum(results, path, RE_ATTR_LAPS, NULL, car->_laps - 1);
				GfParmSetNum(results, path, RE_ATTR_TIME, NULL, car->_curTime);
				GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
				GfParmSetNum(results, path, RE_ATTR_TOP_SPEED, NULL, car->_topSpeed);
				GfParmSetNum(results, path, RE_ATTR_DAMMAGES, NULL, car->_dammage);
				GfParmSetNum(results, path, RE_ATTR_NB_PIT_STOPS, NULL, car->_nbPitStops);
			
				GfParmSetStr(results, path, RE_ATTR_MODULE, car->_modName);
				GfParmSetNum(results, path, RE_ATTR_IDX, NULL, car->_driverIndex);
			
				sprintf(path2, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
				GfParmSetNum(results, path, RE_ATTR_POINTS, NULL,
						(int)GfParmGetNum(params, path2, RE_ATTR_POINTS, NULL, 0));

				GfParmReleaseHandle(carparam);
			}
			break;
			
		case RM_TYPE_PRACTICE:
			car = s->cars[0];
			sprintf(path, "%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race);
			GfParmSetStr(results, path, RM_ATTR_DRVNAME, car->_name);
			break;
			
		case RM_TYPE_QUALIF:
			car = s->cars[0];
			sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
			nCars = GfParmGetEltNb(results, path);
			for (i = nCars; i > 0; i--) {
				sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i);
				float opponentBestLapTime = GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0);
			
				if (
					(car->_bestLapTime != 0.0) && 
					((car->_bestLapTime < opponentBestLapTime) || (opponentBestLapTime == 0.0))
				) {
					/* shift */
					sprintf(path2, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
					GfParmSetStr(results, path2, RE_ATTR_NAME, GfParmGetStr(results, path, RE_ATTR_NAME, ""));
					GfParmSetStr(results, path2, RE_ATTR_CAR, GfParmGetStr(results, path, RE_ATTR_CAR, ""));
					GfParmSetNum(results, path2, RE_ATTR_BEST_LAP_TIME, NULL, GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0));
					GfParmSetStr(results, path2, RE_ATTR_MODULE, GfParmGetStr(results, path, RM_ATTR_MODULE, ""));
					GfParmSetNum(results, path2, RE_ATTR_IDX, NULL, GfParmGetNum(results, path, RM_ATTR_IDX, NULL, 0));
					sprintf(path, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
					GfParmSetNum(results, path2, RE_ATTR_POINTS, NULL,
								(int)GfParmGetNum(params, path, RE_ATTR_POINTS, NULL, 0));
				} else {
					break;
				}
			}
			/* insert after */
			sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i + 1);
			GfParmSetStr(results, path, RE_ATTR_NAME, car->_name);
			
			sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
			carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
			carName = GfParmGetName(carparam);
			
			GfParmSetStr(results, path, RE_ATTR_CAR, carName);
			GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
			GfParmSetStr(results, path, RE_ATTR_MODULE, car->_modName);
			GfParmSetNum(results, path, RE_ATTR_IDX, NULL, car->_driverIndex);
			sprintf(path2, "%s/%s/%d", race, RM_SECT_POINTS, i + 1);
			GfParmSetNum(results, path, RE_ATTR_POINTS, NULL,
						(int)GfParmGetNum(params, path2, RE_ATTR_POINTS, NULL, 0));
		
			GfParmReleaseHandle(carparam);
			break;
	}
}

void
ReUpdateQualifCurRes(tCarElt *car)
{
	int		i;
	int		nCars;
	int		printed;
	int		maxLines;
	void	*carparam;
	char	*carName;
	char	*race = ReInfo->_reRaceName;
	void	*results = ReInfo->results;
	
	ReResEraseScreen();
	maxLines = ReResGetLines();
	
	sprintf(buf, "%s on %s - Lap %d", car->_name, ReInfo->track->name, car->_laps);
	ReResScreenSetTitle(buf);
	
	sprintf(buf, "cars/%s/%s.xml", car->_carName, car->_carName);
	carparam = GfParmReadFile(buf, GFPARM_RMODE_STD);
	carName = GfParmGetName(carparam);
	
	printed = 0;
	sprintf(path, "%s/%s/%s/%s", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK);
	nCars = GfParmGetEltNb(results, path);
	nCars = MIN(nCars + 1, maxLines);
	for (i = 1; i < nCars; i++) {
		sprintf(path, "%s/%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, race, RE_SECT_RANK, i);
		if (!printed) {
			if ((car->_bestLapTime != 0.0) && (car->_bestLapTime < GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0))) {
				sprintf(buf, "%d - %s - %s (%s)", i, GfTime2Str(car->_bestLapTime, 0), car->_name, carName);
				ReResScreenSetText(buf, i - 1, 1);
				printed = 1;
			}
		}
		sprintf(buf, "%d - %s - %s (%s)", i + printed, GfTime2Str(GfParmGetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, 0), 0),
		GfParmGetStr(results, path, RE_ATTR_NAME, ""), GfParmGetStr(results, path, RE_ATTR_CAR, ""));
		ReResScreenSetText(buf, i - 1 + printed, 0);
	}

	if (!printed) {
		sprintf(buf, "%d - %s - %s (%s)", i, GfTime2Str(car->_bestLapTime, 0), car->_name, carName);
		ReResScreenSetText(buf, i - 1, 1);
	}

	GfParmReleaseHandle(carparam);
	ReInfo->_refreshDisplay = 1;
}

void
ReSavePracticeLap(tCarElt *car)
{
    void	*results = ReInfo->results;
    tReCarInfo	*info = &(ReInfo->_reCarInfo[car->index]);

    sprintf(path, "%s/%s/%s/%d", ReInfo->track->name, RE_SECT_RESULTS, ReInfo->_reRaceName, car->_laps - 1);
    GfParmSetNum(results, path, RE_ATTR_TIME, NULL, car->_lastLapTime);
    GfParmSetNum(results, path, RE_ATTR_BEST_LAP_TIME, NULL, car->_bestLapTime);
    GfParmSetNum(results, path, RE_ATTR_TOP_SPEED, NULL, info->topSpd);
    GfParmSetNum(results, path, RE_ATTR_BOT_SPEED, NULL, info->botSpd);
    GfParmSetNum(results, path, RE_ATTR_DAMMAGES, NULL, car->_dammage);
    
}

int
ReDisplayResults(void)
{
    void	*params = ReInfo->params;

    if ((!strcmp(GfParmGetStr(params, ReInfo->_reRaceName, RM_ATTR_DISPRES, RM_VAL_YES), RM_VAL_YES)) ||
	(ReInfo->_displayMode == RM_DISP_MODE_NORMAL)) {
	RmShowResults(ReInfo->_reGameScreen, ReInfo);
    } else {
	ReResShowCont();
    }

    return RM_ASYNC | RM_NEXT_STEP;
}


void
ReDisplayStandings(void)
{
    RmShowStandings(ReInfo->_reGameScreen, ReInfo);
}
