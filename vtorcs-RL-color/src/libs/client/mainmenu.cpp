/***************************************************************************

    file                 : mainmenu.cpp
    created              : Sat Mar 18 23:42:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: mainmenu.cpp,v 1.4.2.1 2008/08/16 23:59:54 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdio.h>
#include <tgfclient.h>
#include <singleplayer.h>
#include <driverconfig.h>

#include "mainmenu.h"
#include "exitmenu.h"
#include "optionmenu.h"


void *menuHandle = NULL;
tModList *RacemanModLoaded = (tModList*)NULL;

static void
TorcsMainMenuActivate(void * /* dummy */)
{
	if (RacemanModLoaded != NULL) {
		GfModUnloadList(&RacemanModLoaded);
	}
}

/*
 * Function
 *	TorcsMainMenuInit
 *
 * Description
 *	init the main menus
 *
 * Parameters
 *	none
 *
 * Return
 *	0 ok -1 nok
 *
 * Remarks
 *	
 */
int
TorcsMainMenuInit(void)
{
	if (getTextOnly())
		ReSinglePlayerInit(NULL);
	else
	{
	    menuHandle = GfuiScreenCreateEx((float*)NULL, 
					    NULL, TorcsMainMenuActivate, 
					    NULL, (tfuiCallback)NULL, 
					    1);

	    GfuiScreenAddBgImg(menuHandle, "data/img/splash-main.png");

	    GfuiTitleCreate(menuHandle, "TORCS", 0);

	    GfuiLabelCreate(menuHandle,
			    "The Open Racing Car Simulator",
			    GFUI_FONT_LARGE,
			    320,
			    420,
			    GFUI_ALIGN_HC_VB,
			    0);

	    GfuiMenuButtonCreate(menuHandle,
				 "Race", "Races Menu",
				 ReSinglePlayerInit(menuHandle), GfuiScreenActivate);

	    GfuiMenuButtonCreate(menuHandle,
				 "Configure Players", "Players configuration menu",
				 TorcsDriverMenuInit(menuHandle), GfuiScreenActivate);

	    GfuiMenuButtonCreate(menuHandle,
				 "Options", "Configure",
				 TorcsOptionOptionInit(menuHandle), GfuiScreenActivate);
	    
	    GfuiMenuDefaultKeysAdd(menuHandle);

	    GfuiMenuBackQuitButtonCreate(menuHandle,
					 "Quit", "Quit TORCS",
					 TorcsMainExitMenuInit(menuHandle), GfuiScreenActivate);
	}

    return 0;
}

/*
 * Function
 *	
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
int
TorcsMainMenuRun(void)
{
    GfuiScreenActivate(menuHandle);
    return 0;
}
