/***************************************************************************

    file        : mouseconfig.cpp
    created     : Thu Mar 13 21:27:03 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: mouseconfig.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $                                  

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
    @version	$Id: mouseconfig.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $
*/

#include <stdio.h>
#include <stdlib.h>

#include <tgfclient.h>
#include <track.h>
#include <robot.h>
#include <playerpref.h>
#include <js.h>

#include "controlconfig.h"
#include "mouseconfig.h"

static void 	*scrHandle2 = NULL;

static tCtrlMouseInfo	mouseInfo;

#define CMD_OFFSET	6

/*
 * Mouse calibration
 */
static int	InstId;
static int	CalState;
static int	scrw, scrh;

static tCmdInfo *Cmd;
static int maxCmd;

static char *Instructions[] = {
    "Move Mouse for maximum left steer then press a button",
    "Move Mouse for maximum right steer then press a button",
    "Move Mouse for full throttle then press a button",
    "Move Mouse for full brake then press a button",
    "Calibration terminated",
    "Calibration failed"
};

static void Idle2(void);

static int
GetNextAxis(void)
{
    int i;

    for (i = CalState; i < 4; i++) {
	if (Cmd[CMD_OFFSET + i].ref.type == GFCTRL_TYPE_MOUSE_AXIS) {
	    return i;
	}
    }
    return i;
}


static void
MouseCalAutomaton(void)
{
    float	axv;

    switch (CalState) {
    case 0:
    case 1:
	GfctrlMouseGetCurrent(&mouseInfo);
	axv = mouseInfo.ax[Cmd[CMD_OFFSET + CalState].ref.index];
	if (fabs(axv) < 0.01) {
	    return;		/* ignore no move input */
	}
	Cmd[CMD_OFFSET + CalState].max = axv;
	Cmd[CMD_OFFSET + CalState].pow = 1.0 / axv;
	break;

    case 2:
    case 3:
	GfctrlMouseGetCurrent(&mouseInfo);
	axv = mouseInfo.ax[Cmd[CMD_OFFSET + CalState].ref.index];
	    if (fabs(axv) < 0.01) {
	    return;		/* ignore no move input */
	}
	Cmd[CMD_OFFSET + CalState].max = axv;
	Cmd[CMD_OFFSET + CalState].pow = 1.0 / axv;
	break;
	
    }

    CalState++;
    CalState = GetNextAxis();
    GfuiLabelSetText(scrHandle2, InstId, Instructions[CalState]);
    if (CalState < 4) {
	glutIdleFunc(Idle2);
    } else {
	glutIdleFunc(GfuiIdle);
    }
}

static void
Idle2(void)
{
    int	i;

    GfctrlMouseGetCurrent(&mouseInfo);

    /* Check for a mouse button pressed */
    for (i = 0; i < 3; i++) {
	if (mouseInfo.edgedn[i]) {
	    MouseCalAutomaton();
	    return;
	}
    }
}

static void
IdleMouseInit(void)
{
    /* Get the center mouse position  */
    memset(&mouseInfo, 0, sizeof(mouseInfo));
    GfctrlMouseGetCurrent(&mouseInfo);
    GfctrlMouseInitCenter();
    glutIdleFunc(Idle2);
}

static void
onActivate2(void * /* dummy */)
{
    int dummy;

    GfScrGetSize(&scrw, &scrh, &dummy, &dummy);
    CalState = 0;
    GetNextAxis();
    GfuiLabelSetText(scrHandle2, InstId, Instructions[CalState]);
    if (CalState < 4) {
	glutIdleFunc(IdleMouseInit);
	GfctrlMouseCenter();
    }
}

void *
MouseCalMenuInit(void *prevMenu, tCmdInfo *cmd, int maxcmd)
{
    int x, y, dy;
    
    Cmd = cmd;
    maxCmd = maxcmd;
    
    if (scrHandle2) {
	return scrHandle2;
    }

    scrHandle2 = GfuiScreenCreateEx(NULL, NULL, onActivate2, NULL, NULL, 1);
    GfuiTitleCreate(scrHandle2, "Mouse Calibration", 0);
    GfuiMenuDefaultKeysAdd(scrHandle2);

    GfuiScreenAddBgImg(scrHandle2, "data/img/splash-mousecal.png");

    x = 128;
    y = 300;
    dy = 50;

    InstId = GfuiLabelCreate(scrHandle2, "", GFUI_FONT_MEDIUM, 320, 80, GFUI_ALIGN_HC_VB, 60);

    GfuiButtonCreate(scrHandle2, "Back", GFUI_FONT_LARGE, 160, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     prevMenu, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);
    
    GfuiButtonCreate(scrHandle2, "Reset", GFUI_FONT_LARGE, 480, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     NULL, onActivate2, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    return scrHandle2;
}
