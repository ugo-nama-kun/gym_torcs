/***************************************************************************

    file        : miscscreens.cpp
    created     : Sun Dec  8 13:01:47 CET 2002
    copyright   : (C) 2002 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: miscscreens.cpp,v 1.2.2.1 2008/05/31 15:52:38 berniw Exp $                                  

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
    @version	$Id: miscscreens.cpp,v 1.2.2.1 2008/05/31 15:52:38 berniw Exp $
*/

#include <stdio.h>
#include <tgfclient.h>
#include <robot.h>

#include <racescreens.h>

static void *twoStateHdle = 0;
static void *triStateHdle = 0;
static void *fourStateHdle = 0;


void *
RmTwoStateScreen(
	char *title,
	char *label1, char *tip1, void *screen1,
	char *label2, char *tip2, void *screen2)
{
	if (twoStateHdle) {
		GfuiScreenRelease(twoStateHdle);
	}
	
	twoStateHdle = GfuiMenuScreenCreate(title);
	GfuiScreenAddBgImg(twoStateHdle, "data/img/splash-quit.png");
	GfuiMenuButtonCreate(twoStateHdle, label1, tip1, screen1, GfuiScreenActivate);
	GfuiMenuButtonCreate(twoStateHdle, label2, tip2, screen2, GfuiScreenActivate);
	GfuiAddKey(twoStateHdle, 27, tip2, screen2, GfuiScreenActivate, NULL);
	GfuiScreenActivate(twoStateHdle);

	return twoStateHdle;
}


void *
RmTriStateScreen(
	char *title,
	char *label1, char *tip1, void *screen1,
	char *label2, char *tip2, void *screen2,
	char *label3, char *tip3, void *screen3)
{
	if (triStateHdle) {
		GfuiScreenRelease(triStateHdle);
	}
	
	triStateHdle = GfuiMenuScreenCreate(title);
	GfuiScreenAddBgImg(triStateHdle, "data/img/splash-quit.png");
	GfuiMenuButtonCreate(triStateHdle, label1, tip1, screen1, GfuiScreenActivate);
	GfuiMenuButtonCreate(triStateHdle, label2, tip2, screen2, GfuiScreenActivate);
	GfuiMenuButtonCreate(triStateHdle, label3, tip3, screen3, GfuiScreenActivate);
	GfuiAddKey(triStateHdle, 27, tip3, screen3, GfuiScreenActivate, NULL);
	GfuiScreenActivate(triStateHdle);
	
	return triStateHdle;
}

void *
RmFourStateScreen(
	char *title,
	char *label1, char *tip1, void *screen1,
	char *label2, char *tip2, void *screen2,
	char *label3, char *tip3, void *screen3,
	char *label4, char *tip4, void *screen4)
{
	if (fourStateHdle) {
		GfuiScreenRelease(fourStateHdle);
	}
	
	fourStateHdle = GfuiMenuScreenCreate(title);
	GfuiScreenAddBgImg(fourStateHdle, "data/img/splash-quit.png");
	GfuiMenuButtonCreate(fourStateHdle, label1, tip1, screen1, GfuiScreenActivate);
	GfuiMenuButtonCreate(fourStateHdle, label2, tip2, screen2, GfuiScreenActivate);
	GfuiMenuButtonCreate(fourStateHdle, label3, tip3, screen3, GfuiScreenActivate);
	GfuiMenuButtonCreate(fourStateHdle, label4, tip4, screen4, GfuiScreenActivate);
	GfuiAddKey(fourStateHdle, 27, tip4, screen4, GfuiScreenActivate, NULL);
	GfuiScreenActivate(fourStateHdle);
	
	return fourStateHdle;
}



/*********************************************************
 * Start screen
 */

#define MAX_LINES 20

typedef struct 
{
    void	*startScr;
    void	*abortScr;
    tRmInfo	*info;
    int		start;
} tStartRaceCall;

static tStartRaceCall	nextStartRace, prevStartRace;
static void		*rmScrHdle = 0;

static void rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start);

static void
rmChgStartScreen(void *vpsrc)
{
	void		*prevScr = rmScrHdle;
	tStartRaceCall 	*psrc = (tStartRaceCall*)vpsrc;
	
	rmDisplayStartRace(psrc->info, psrc->startScr, psrc->abortScr, psrc->start);
	GfuiScreenRelease(prevScr);
}

static void
rmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr, int start)
{
	static char		path[1024];
	int			nCars;
	int			i;
	int			y;
	int			x, dx;
	int			rows, curRow;
	char		*img;
	char		*name;
	int			robotIdx;
	void		*robhdle;
	void		*carHdle;
	char		*carName;
	void		*params = info->params;
	char		*race = info->_reRaceName;
	
	rmScrHdle = GfuiScreenCreate();
	GfuiTitleCreate(rmScrHdle, race, strlen(race));	
	
	img = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_STARTIMG, 0);
	if (img) {
		GfuiScreenAddBgImg(rmScrHdle, img);
	}
	
	if (!strcmp(GfParmGetStr(params, race, RM_ATTR_DISP_START_GRID, RM_VAL_YES), RM_VAL_YES)) {
		GfuiLabelCreate(rmScrHdle, "Starting Grid", GFUI_FONT_MEDIUM_C, 320, 420, GFUI_ALIGN_HC_VB, 0);
		sprintf(path, "%s/%s", race, RM_SECT_STARTINGGRID);
		rows = (int)GfParmGetNum(params, path, RM_ATTR_ROWS, (char*)NULL, 2);
		
		dx = 0;
		x = 40;
		y = 400;
		curRow = 0;
		nCars = GfParmGetEltNb(params, RM_SECT_DRIVERS_RACING);
		
		for (i = start; i < MIN(start + MAX_LINES, nCars); i++) {
			/* Find starting driver's name */
			sprintf(path, "%s/%d", RM_SECT_DRIVERS_RACING, i + 1);
			name = GfParmGetStr(info->params, path, RM_ATTR_MODULE, "");
			robotIdx = (int)GfParmGetNum(info->params, path, RM_ATTR_IDX, NULL, 0);
			
			sprintf(path, "%sdrivers/%s/%s.xml", GetLocalDir(), name, name);
			robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
			if (!robhdle) {
				sprintf(path, "drivers/%s/%s.xml", name, name);
				robhdle = GfParmReadFile(path, GFPARM_RMODE_STD);
			}

			if (robhdle) {
				sprintf(path, "%s/%s/%d", ROB_SECT_ROBOTS, ROB_LIST_INDEX, robotIdx);
				name = GfParmGetStr(robhdle, path, ROB_ATTR_NAME, "<none>");
				carName = GfParmGetStr(robhdle, path, ROB_ATTR_CAR, "");
				
				sprintf(path, "cars/%s/%s.xml", carName, carName);
				carHdle = GfParmReadFile(path, GFPARM_RMODE_STD);
				carName = GfParmGetName(carHdle);
			
				sprintf(path, "%d - %s - (%s)", i + 1, name, carName);
				GfuiLabelCreate(rmScrHdle, path, GFUI_FONT_MEDIUM_C,
						x + curRow * dx, y, GFUI_ALIGN_HL_VB, 0);

				GfParmReleaseHandle(carHdle);
				GfParmReleaseHandle(robhdle);
			}
			curRow = (curRow + 1) % rows;
			y -= 15;
		}
		
		
		if (start > 0) {
			prevStartRace.startScr = startScr;
			prevStartRace.abortScr = abortScr;
			prevStartRace.info     = info;
			prevStartRace.start    = start - MAX_LINES;
			GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-up.png", "data/img/arrow-up.png",
						"data/img/arrow-up.png", "data/img/arrow-up-pushed.png",
						80, 40, GFUI_ALIGN_HL_VB, 1,
						(void*)&prevStartRace, rmChgStartScreen,
						NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
			GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_UP,   "Previous drivers", (void*)&prevStartRace, rmChgStartScreen, NULL);
		}
		
		if (i < nCars) {
			nextStartRace.startScr = startScr;
			nextStartRace.abortScr = abortScr;
			nextStartRace.info     = info;
			nextStartRace.start    = start + MAX_LINES;
			GfuiGrButtonCreate(rmScrHdle, "data/img/arrow-down.png", "data/img/arrow-down.png",
						"data/img/arrow-down.png", "data/img/arrow-down-pushed.png",
						540, 40, GFUI_ALIGN_HL_VB, 1,
						(void*)&nextStartRace, rmChgStartScreen,
						NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
			GfuiAddSKey(rmScrHdle, GLUT_KEY_PAGE_DOWN, "Next Drivers", (void*)&nextStartRace, rmChgStartScreen, NULL);
		}
	}
	
	GfuiButtonCreate(rmScrHdle,
				"Start",
				GFUI_FONT_LARGE,
				210,
				40,
				150,
				GFUI_ALIGN_HC_VB,
				0,
				startScr,
				GfuiScreenReplace,
				NULL,
				(tfuiCallback)NULL,
				(tfuiCallback)NULL);
	GfuiAddKey(rmScrHdle, (unsigned char)13, "Start",   startScr, GfuiScreenReplace, NULL);
	
	GfuiButtonCreate(rmScrHdle,
				"Abandon",
				GFUI_FONT_LARGE,
				430,
				40,
				150,
				GFUI_ALIGN_HC_VB,
				0,
				abortScr,
				GfuiScreenReplace,
				NULL,
				(tfuiCallback)NULL,
				(tfuiCallback)NULL);
	GfuiAddKey(rmScrHdle, (unsigned char)27, "Abandon", abortScr, GfuiScreenReplace, NULL);
	
	GfuiAddSKey(rmScrHdle, GLUT_KEY_F12, "Take a Screen Shot", NULL, GfuiScreenShot, NULL);
	
	GfuiScreenActivate(rmScrHdle);
}


void
RmDisplayStartRace(tRmInfo *info, void *startScr, void *abortScr)
{
    rmDisplayStartRace(info, startScr, abortScr, 0);
}
