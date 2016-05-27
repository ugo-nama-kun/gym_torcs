/***************************************************************************

    file                 : exitmenu.cpp
    created              : Sat Mar 18 23:42:12 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: exitmenu.cpp,v 1.4 2006/10/05 21:25:55 berniw Exp $

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
#include "exitmenu.h"
#include "mainmenu.h"

static void 
endofprog(void * /* dummy */)
{
    STOP_ACTIVE_PROFILES();
    PRINT_PROFILE();
/*     glutSetKeyRepeat(GLUT_KEY_REPEAT_ON); */
    GfScrShutdown();
    exit(0);
}

static void *exitmenuHandle = NULL;
static void *exitMainMenuHandle = NULL;


void * exitMenuInit(void *menu, void *menuHandle)
{
    if (menuHandle) {
		GfuiScreenRelease(menuHandle);
    }
    
    menuHandle = GfuiMenuScreenCreate("Quit ?");
    GfuiScreenAddBgImg(menuHandle, "data/img/splash-quit.png");

    GfuiMenuButtonCreate(menuHandle,
		      "No, Back to Game",
		      "Return to TORCS",
		      menu,
		      GfuiScreenActivate);
    
    GfuiMenuButtonCreate(menuHandle,
		      "Yes, Let's Quit",
		      "Exit of TORCS",
		      NULL,
		      endofprog);
    return menuHandle;
}

/*
 * Function
 *	TorcsExitMenuInit
 *
 * Description
 *	init the exit menus
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
void * TorcsExitMenuInit(void *menu)
{
	exitmenuHandle = exitMenuInit(menu, exitmenuHandle);
	return exitmenuHandle;
}


void * TorcsMainExitMenuInit(void *mainMenu)
{
	exitMainMenuHandle = exitMenuInit(mainMenu, exitMainMenuHandle);
	return exitMainMenuHandle;
}
