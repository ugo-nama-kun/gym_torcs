/***************************************************************************

    file        : racemanmenu.cpp
    created     : Fri Jan  3 22:24:41 CET 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs Exp $                                  

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
    @version	$Id: racemanmenu.cpp,v 1.5 2004/08/11 17:44:06 torcs Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <raceman.h>
#include <racescreens.h>
#include <driverconfig.h>

#include "raceengine.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"

#include "racemanmenu.h"

static float red[4]     = {1.0, 0.0, 0.0, 1.0};

static void		*racemanMenuHdle = NULL;
static void		*newTrackMenuHdle = NULL;
static tRmTrackSelect	ts;
static tRmDrvSelect	ds;
static tRmRaceParam	rp;
static tRmFileSelect    fs;

static char		path[1024];
static char		buf[1024];


static void reConfigRunState(void);

static void
reConfigBack(void)
{
    void	*params = ReInfo->params;

    /* Go back one step in the conf */
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 
		 GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1) - 2);

    reConfigRunState();
}


/***************************************************************/
/* Callback hooks used only to run the automaton on activation */
static void	*configHookHandle = 0;

static void
configHookActivate(void * /* dummy */)
{
    reConfigRunState();
}

static void *
reConfigHookInit(void)
{
    if (configHookHandle) {
	return configHookHandle;
    }

    configHookHandle = GfuiHookCreate(0, configHookActivate);

    return configHookHandle;
}

/***************************************************************/
/* Config Back Hook */

static void	*ConfigBackHookHandle = 0;

static void
ConfigBackHookActivate(void * /* dummy */)
{
    reConfigBack();
}

static void *
reConfigBackHookInit(void)
{
    if (ConfigBackHookHandle) {
	return ConfigBackHookHandle;
    }

    ConfigBackHookHandle = GfuiHookCreate(0, ConfigBackHookActivate);

    return ConfigBackHookHandle;
}

static void
reConfigRunState(void)
{
    int		i;
    int		curConf;
    char	*conf;
    int		numOpt;
    char	*opt;
    void	*params = ReInfo->params;

    curConf = (int)GfParmGetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
    if (curConf > GfParmGetEltNb(params, RM_SECT_CONF)) {
	GfOut("End of configuration\n");
	GfParmWriteFile(NULL, ReInfo->params, ReInfo->_reName);
	goto menuback;
    }
    
    sprintf(path, "%s/%d", RM_SECT_CONF, curConf);
    conf = GfParmGetStr(params, path, RM_ATTR_TYPE, 0);
    if (!conf) {
	GfOut("no %s here %s\n", RM_ATTR_TYPE, path);
	goto menuback;
    }

    GfOut("Configuration step %s\n", conf);
    if (!strcmp(conf, RM_VAL_TRACKSEL)) {
	/* Track Select Menu */
	ts.nextScreen = reConfigHookInit();
	if (curConf == 1) {
	    ts.prevScreen = racemanMenuHdle;
	} else {
	    ts.prevScreen = reConfigBackHookInit();
	}
	ts.param = ReInfo->params;
	ts.trackItf = ReInfo->_reTrackItf;
	RmTrackSelect(&ts);

    } else if (!strcmp(conf, RM_VAL_DRVSEL)) {
	/* Drivers select menu */
	ds.nextScreen = reConfigHookInit();
	if (curConf == 1) {
	    ds.prevScreen = racemanMenuHdle;
	} else {
	    ds.prevScreen = reConfigBackHookInit();
	}
	ds.param = ReInfo->params;
	RmDriversSelect(&ds);

    } else if (!strcmp(conf, RM_VAL_RACECONF)) {
	/* Race Options menu */
	rp.nextScreen = reConfigHookInit();
	if (curConf == 1) {
	    rp.prevScreen = racemanMenuHdle;
	} else {
	    rp.prevScreen = reConfigBackHookInit();
	}
	rp.param = ReInfo->params;
	rp.title = GfParmGetStr(params, path, RM_ATTR_RACE, "Race");
	/* Select options to configure */
	rp.confMask = 0;
	sprintf(path, "%s/%d/%s", RM_SECT_CONF, curConf, RM_SECT_OPTIONS);
	numOpt = GfParmGetEltNb(params, path);
	for (i = 1; i < numOpt + 1; i++) {
	    sprintf(path, "%s/%d/%s/%d", RM_SECT_CONF, curConf, RM_SECT_OPTIONS, i);
	    opt = GfParmGetStr(params, path, RM_ATTR_TYPE, "");
	    if (!strcmp(opt, RM_VAL_CONFRACELEN)) {
		/* Configure race length */
		rp.confMask |= RM_CONF_RACE_LEN;
	    } else {
		if (!strcmp(opt, RM_VAL_CONFDISPMODE)) {
		    /* Configure display mode */
		    rp.confMask |= RM_CONF_DISP_MODE;
		}
	    }
	}
	RmRaceParamMenu(&rp);
    }

    curConf++;
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, curConf);

    return;

    /* Back to the race menu */
 menuback:
    GfuiScreenActivate(racemanMenuHdle);
    return;
}

static void
reConfigureMenu(void * /* dummy */)
{
    void *params = ReInfo->params;

    /* Reset configuration automaton */
    GfParmSetNum(params, RM_SECT_CONF, RM_ATTR_CUR_CONF, NULL, 1);
    reConfigRunState();
}

static void
reSelectLoadFile(char *filename)
{
    sprintf(buf, "%sresults/%s/%s", GetLocalDir(), ReInfo->_reFilename, filename);
    GfOut("Loading Saved File %s...\n", buf);
    ReInfo->results = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
    ReInfo->_reRaceName = ReInfo->_reName;
    RmShowStandings(ReInfo->_reGameScreen, ReInfo);
}

static void
reLoadMenu(void *prevHandle)
{
    char *str;
    void *params = ReInfo->params;

    fs.prevScreen = prevHandle;
    fs.select = reSelectLoadFile;

    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
    if (str) {
	fs.title = str;
    }
    sprintf(buf, "%sresults/%s", GetLocalDir(), ReInfo->_reFilename);
    fs.path = buf;

    RmFileSelect((void*)&fs);
}

int
ReRacemanMenu(void)
{
    char	*str;
    void	*params = ReInfo->params;

    if (getTextOnly())
    	ReStartNewRace(NULL);
    else
    {
		if (racemanMenuHdle) {
		GfuiScreenRelease(racemanMenuHdle);
		}
		racemanMenuHdle = GfuiScreenCreateEx(NULL,
						 NULL, (tfuiCallback)NULL,
						 NULL, (tfuiCallback)NULL,
						 1);

		str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
		if (str) {
		GfuiScreenAddBgImg(racemanMenuHdle, str);
		}

		GfuiMenuDefaultKeysAdd(racemanMenuHdle);

		str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, 0);
		if (str) {
		GfuiTitleCreate(racemanMenuHdle, str, strlen(str));
		}


		GfuiMenuButtonCreate(racemanMenuHdle,
				 "New Race", "Start a New Race",
				 NULL, ReStartNewRace);

		GfuiMenuButtonCreate(racemanMenuHdle,
				 "Configure Race", "Configure The Race",
				 NULL, reConfigureMenu);

	/*     GfuiMenuButtonCreate(racemanMenuHdle, */
	/* 			 "Configure Players", "Players configuration menu", */
	/* 			 TorcsDriverMenuInit(racemanMenuHdle), GfuiScreenActivate); */

		if (GfParmGetEltNb(params, RM_SECT_TRACKS) > 1) {
		GfuiMenuButtonCreate(racemanMenuHdle,
					 "Load", "Load a Previously Saved Game",
					 racemanMenuHdle, reLoadMenu);
		}

		GfuiMenuBackQuitButtonCreate(racemanMenuHdle,
					 "Back to Main", "Return to previous Menu",
					 ReInfo->_reMenuScreen, GfuiScreenActivate);

		GfuiScreenActivate(racemanMenuHdle);
    }
    return RM_ASYNC | RM_NEXT_STEP;
}

static void
reStateManage(void * /* dummy */)
{
    ReStateManage();
}

int
ReNewTrackMenu(void)
{
    char	*str;
    void	*params = ReInfo->params;
    void	*results = ReInfo->results;

    if (newTrackMenuHdle) {
	GfuiScreenRelease(newTrackMenuHdle);
    }
    newTrackMenuHdle = GfuiScreenCreateEx(NULL, 
					  NULL, (tfuiCallback)NULL, 
					  NULL, (tfuiCallback)NULL, 
					  1);

    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_BGIMG, 0);
    if (str) {
	GfuiScreenAddBgImg(newTrackMenuHdle, str);
    }
    str = GfParmGetStr(params, RM_SECT_HEADER, RM_ATTR_NAME, "");
    GfuiTitleCreate(newTrackMenuHdle, str, strlen(str));

    GfuiMenuDefaultKeysAdd(newTrackMenuHdle);

    sprintf(buf, "Race Day #%d/%d on %s",
	    (int)GfParmGetNum(results, RE_SECT_CURRENT, RE_ATTR_CUR_TRACK, NULL, 1),
	    GfParmGetEltNb(params, RM_SECT_TRACKS),
	    ReInfo->track->name);

    GfuiLabelCreateEx(newTrackMenuHdle,
		      buf,
		      red,
		      GFUI_FONT_MEDIUM_C,
		      320, 420,
		      GFUI_ALIGN_HC_VB, 50);

    GfuiMenuButtonCreate(newTrackMenuHdle,
			 "Start Event", "Start The Current Race",
			 NULL, reStateManage);

    
    GfuiMenuButtonCreate(newTrackMenuHdle, 
			 "Abandon", "Abandon The Race",
			 ReInfo->_reMenuScreen, GfuiScreenActivate);

    GfuiAddKey(newTrackMenuHdle, 27,  "Abandon", ReInfo->_reMenuScreen, GfuiScreenActivate, NULL);

    GfuiScreenActivate(newTrackMenuHdle);

    return RM_ASYNC | RM_NEXT_STEP;
}
