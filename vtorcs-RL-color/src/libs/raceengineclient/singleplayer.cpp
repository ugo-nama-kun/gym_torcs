/***************************************************************************

    file        : singleplayer.cpp
    created     : Sat Nov 16 09:36:29 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: singleplayer.cpp,v 1.4 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: singleplayer.cpp,v 1.4 2004/04/05 18:25:00 olethros Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <raceman.h>

#include "raceengine.h"
#include "racemain.h"
#include "raceinit.h"
#include "racestate.h"

static void *singlePlayerHandle = NULL;

/* Called when the menu is activated */
static void
singlePlayerMenuActivate(void * /* dummy */)
{
    /* Race engine init */
    ReInit();
    ReInfo->_reMenuScreen = singlePlayerHandle;
}

/* Exit from Race engine */
static void
singlePLayerShutdown(void *precMenu)
{
    GfuiScreenActivate(precMenu);
    ReShutdown();
}


/* Initialize the single player menu */
void *
ReSinglePlayerInit(void *precMenu)
{
	if (getTextOnly())
	{
		ReInit();
		ReAddRacemanListButton(NULL);
		//reSelectRaceman();
		return NULL;
	}
	else
	{

		if (singlePlayerHandle)
			return singlePlayerHandle;

		singlePlayerHandle = GfuiScreenCreateEx((float*) NULL, NULL,
				singlePlayerMenuActivate, NULL, (tfuiCallback) NULL, 1);

		GfuiTitleCreate(singlePlayerHandle, "SELECT RACE", 0);

		GfuiScreenAddBgImg(singlePlayerHandle,
				"data/img/splash-single-player.png");

		/* Display the raceman button selection */
		ReAddRacemanListButton(singlePlayerHandle);

		GfuiMenuDefaultKeysAdd(singlePlayerHandle);

		ReStateInit(singlePlayerHandle);

		GfuiMenuBackQuitButtonCreate(singlePlayerHandle, "Back",
				"Back to Main", precMenu, singlePLayerShutdown);

		return singlePlayerHandle;
	}
}
