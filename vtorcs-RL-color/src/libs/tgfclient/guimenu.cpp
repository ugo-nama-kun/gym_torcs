/***************************************************************************
                          menu.cpp -- menu management                            
                             -------------------                                         
    created              : Fri Aug 13 22:23:19 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: guimenu.cpp,v 1.2.2.1 2008/11/09 17:50:22 berniw Exp $                                  
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
    		GUI menu management.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: guimenu.cpp,v 1.2.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	gui
*/


#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <GL/glut.h>

#include <tgfclient.h>
#include "gui.h"
#include "guimenu.h"

void
gfMenuInit(void)
{
}

/** Add the default menu keyboard callback to a screen.
    The keys are:
    <br><tt>Up Arrow .... </tt>Select Previous Entry
    <br><tt>Down Arrow .. </tt>Select Next Entry
    <br><tt>F1 .......... </tt>Help
    <br><tt>Tab ......... </tt>Select Next Entry
    <br><tt>Enter ....... </tt>Perform Action
    <br><tt>Escape ...... </tt>Quit the menu
    @ingroup	gui
    @param	scr	Screen Id
 */
void
GfuiMenuDefaultKeysAdd(void *scr)
{
    GfuiAddKey(scr, 9, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddKey(scr, 13, "Perform Action", (void*)2, gfuiMouseAction, NULL);
    GfuiAddSKey(scr, GLUT_KEY_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, GLUT_KEY_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, GLUT_KEY_PAGE_UP, "Select Previous Entry", NULL, gfuiSelectPrev, NULL);
    GfuiAddSKey(scr, GLUT_KEY_PAGE_DOWN, "Select Next Entry", NULL, gfuiSelectNext, NULL);
    GfuiAddSKey(scr, GLUT_KEY_F1, "Help", scr, GfuiHelpScreen, NULL);
    GfuiAddSKey(scr, GLUT_KEY_F12, "Screen-Shot", NULL, GfuiScreenShot, NULL);
    
}

/** Create a new menu screen.
    Set the title of the menu
    Add the default keyboard callbacks to the menu.
    @ingroup	gui
    @param	title	title of the screen
    @return	Handle of the menu
 */
void *
GfuiMenuScreenCreate(char *title)
{
    void	*scr;

    scr = GfuiScreenCreate();
    GfuiTitleCreate(scr, title, strlen(title));

    GfuiMenuDefaultKeysAdd(scr);

    return scr;
}

static void
dispInfo(void *cbinfo)
{
    GfuiVisibilitySet(((tMnuCallbackInfo*)cbinfo)->screen, ((tMnuCallbackInfo*)cbinfo)->labelId, 1);
}
static void
remInfo(void *cbinfo)
{
    GfuiVisibilitySet(((tMnuCallbackInfo*)cbinfo)->screen, ((tMnuCallbackInfo*)cbinfo)->labelId, 0);
}


/** Add a button to a menu screen.
    @ingroup	gui
    @param	scr		Screen (menu) handle
    @param	text		Text of the button
    @param	tip		Text of the tip displayed when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button Id
 */
int
GfuiMenuButtonCreate(void *scr, char *text, char *tip, void *userdata, tfuiCallback onpush)
{
    tMnuCallbackInfo	*cbinfo;
    int			xpos, ypos;
    int			nbItems = ((tGfuiScreen*)scr)->nbItems++;
    int			bId;

    if (nbItems < 11) {
	xpos = 320;
	ypos = 380 - 30 * nbItems;
    } else {
	if (nbItems > 22) {
	    GfTrace("Too many items in that menu !!!\n");
	    return -1;
	}
	xpos = 380;
	ypos = 380 - 30 * (nbItems - 11);
    }

    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
			   text,
			   GFUI_FONT_LARGE,
			   xpos, ypos, GFUI_BTNSZ, GFUI_ALIGN_HC_VB, 0,
			   userdata, onpush,
			   (void*)cbinfo, dispInfo,
			   remInfo);

    return bId;
}

/** Add the "Back" or "Quit" button at the bottom of the menu screen.
    @ingroup	gui
    @param	scr	Screen or Menu handle
    @param	text	Text of the button
    @param	tip	Text to display when the button is focused
    @param	userdata	Parameter of the Push function
    @param	onpush		Callback when the button is pushed
    @return	Button Id
 */
int
GfuiMenuBackQuitButtonCreate(void *scr, char *text, char *tip, void *userdata, tfuiCallback onpush)
{
    tMnuCallbackInfo	*cbinfo;
    int			xpos, ypos;
    int			bId;
    
    xpos = 320;
    ypos = 40;

    cbinfo = (tMnuCallbackInfo*)calloc(1, sizeof(tMnuCallbackInfo));
    cbinfo->screen = scr;
    cbinfo->labelId = GfuiTipCreate(scr, tip, strlen(tip));

    GfuiVisibilitySet(scr, cbinfo->labelId, 0);
    
    bId = GfuiButtonCreate(scr,
			text,
			GFUI_FONT_LARGE,
			xpos, ypos, GFUI_BTNSZ, GFUI_ALIGN_HC_VB, 0,
			userdata, onpush,
			(void*)cbinfo, dispInfo,
			remInfo);

    GfuiAddKey(scr, (unsigned char)27, tip, userdata, onpush, NULL);

    return bId;
}


