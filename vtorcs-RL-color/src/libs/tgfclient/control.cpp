/***************************************************************************

    file        : control.cpp
    created     : Thu Mar  6 22:01:33 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: control.cpp,v 1.3.2.1 2008/11/09 17:50:22 berniw Exp $                                  

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
    		Human control (joystick, mouse and keyboard).
    @author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
    @version	$Id: control.cpp,v 1.3.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	ctrl
*/

#include <stdlib.h>
#include <stdio.h>
#include <js.h>

#include <tgfclient.h>


static char *GfJoyBtn[] = {
"BTN1-0","BTN2-0","BTN3-0","BTN4-0","BTN5-0","BTN6-0","BTN7-0","BTN8-0","BTN9-0","BTN10-0","BTN11-0","BTN12-0","BTN13-0","BTN14-0","BTN15-0","BTN16-0",
"BTN17-0","BTN18-0","BTN19-0","BTN20-0","BTN21-0","BTN22-0","BTN23-0","BTN24-0","BTN25-0","BTN26-0","BTN27-0","BTN28-0","BTN29-0","BTN30-0","BTN31-0","BTN32-0",
"BTN1-1","BTN2-1","BTN3-1","BTN4-1","BTN5-1","BTN6-1","BTN7-1","BTN8-1","BTN9-1","BTN10-1","BTN11-1","BTN12-1","BTN13-1","BTN14-1","BTN15-1","BTN16-1",
"BTN17-1","BTN18-1","BTN19-1","BTN20-1","BTN21-1","BTN22-1","BTN23-1","BTN24-1","BTN25-1","BTN26-1","BTN27-1","BTN28-1","BTN29-1","BTN30-1","BTN31-1","BTN32-1",
"BTN1-2","BTN2-2","BTN3-2","BTN4-2","BTN5-2","BTN6-2","BTN7-2","BTN8-2","BTN9-2","BTN10-2","BTN11-2","BTN12-2","BTN13-2","BTN14-2","BTN15-2","BTN16-2",
"BTN17-2","BTN18-2","BTN19-2","BTN20-2","BTN21-2","BTN22-2","BTN23-2","BTN24-2","BTN25-2","BTN26-2","BTN27-2","BTN28-2","BTN29-2","BTN30-2","BTN31-2","BTN32-2",
"BTN1-3","BTN2-3","BTN3-3","BTN4-3","BTN5-3","BTN6-3","BTN7-3","BTN8-3","BTN9-3","BTN10-3","BTN11-3","BTN12-3","BTN13-3","BTN14-3","BTN15-3","BTN16-3",
"BTN17-3","BTN18-3","BTN19-3","BTN20-3","BTN21-3","BTN22-3","BTN23-3","BTN24-3","BTN25-3","BTN26-3","BTN27-3","BTN28-3","BTN29-3","BTN30-3","BTN31-3","BTN32-3",
"BTN1-4","BTN2-4","BTN3-4","BTN4-4","BTN5-4","BTN6-4","BTN7-4","BTN8-4","BTN9-4","BTN10-4","BTN11-4","BTN12-4","BTN13-4","BTN14-4","BTN15-4","BTN16-4",
"BTN17-4","BTN18-4","BTN19-4","BTN20-4","BTN21-4","BTN22-4","BTN23-4","BTN24-4","BTN25-4","BTN26-4","BTN27-4","BTN28-4","BTN29-4","BTN30-4","BTN31-4","BTN32-4",
"BTN1-5","BTN2-5","BTN3-5","BTN4-5","BTN5-5","BTN6-5","BTN7-5","BTN8-5","BTN9-5","BTN10-5","BTN11-5","BTN12-5","BTN13-5","BTN14-5","BTN15-5","BTN16-5",
"BTN17-5","BTN18-5","BTN19-5","BTN20-5","BTN21-5","BTN22-5","BTN23-5","BTN24-5","BTN25-5","BTN26-5","BTN27-5","BTN28-5","BTN29-5","BTN30-5","BTN31-5","BTN32-5",
"BTN1-6","BTN2-6","BTN3-6","BTN4-6","BTN5-6","BTN6-6","BTN7-6","BTN8-6","BTN9-6","BTN10-6","BTN11-6","BTN12-6","BTN13-6","BTN14-6","BTN15-6","BTN16-6",
"BTN17-6","BTN18-6","BTN19-6","BTN20-6","BTN21-6","BTN22-6","BTN23-6","BTN24-6","BTN25-6","BTN26-6","BTN27-6","BTN28-6","BTN29-6","BTN30-6","BTN31-6","BTN32-6",
"BTN1-7","BTN2-7","BTN3-7","BTN4-7","BTN5-7","BTN6-7","BTN7-7","BTN8-7","BTN9-7","BTN10-7","BTN11-7","BTN12-7","BTN13-7","BTN14-7","BTN15-7","BTN16-7",
"BTN17-7","BTN18-7","BTN19-7","BTN20-7","BTN21-7","BTN22-7","BTN23-7","BTN24-7","BTN25-7","BTN26-7","BTN27-7","BTN28-7","BTN29-7","BTN30-7","BTN31-7","BTN32-7"
};

static char *GfJoyAxis[] = {
    "AXIS0-0", "AXIS1-0", "AXIS2-0", "AXIS3-0", "AXIS4-0", "AXIS5-0", "AXIS6-0", "AXIS7-0", "AXIS8-0", "AXIS9-0", "AXIS10-0", "AXIS11-0",
    "AXIS0-1", "AXIS1-1", "AXIS2-1", "AXIS3-1", "AXIS4-1", "AXIS5-1", "AXIS6-1", "AXIS7-1", "AXIS8-1", "AXIS9-1", "AXIS10-1", "AXIS11-1",
    "AXIS0-2", "AXIS1-2", "AXIS2-2", "AXIS3-2", "AXIS4-2", "AXIS5-2", "AXIS6-2", "AXIS7-2", "AXIS8-2", "AXIS9-2", "AXIS10-2", "AXIS11-2",
    "AXIS0-3", "AXIS1-3", "AXIS2-3", "AXIS3-3", "AXIS4-3", "AXIS5-3", "AXIS6-3", "AXIS7-3", "AXIS8-3", "AXIS9-3", "AXIS10-3", "AXIS11-3",
    "AXIS0-4", "AXIS1-4", "AXIS2-4", "AXIS3-4", "AXIS4-4", "AXIS5-4", "AXIS6-4", "AXIS7-4", "AXIS8-4", "AXIS9-4", "AXIS10-4", "AXIS11-4",
    "AXIS0-5", "AXIS1-5", "AXIS2-5", "AXIS3-5", "AXIS4-5", "AXIS5-5", "AXIS6-5", "AXIS7-5", "AXIS8-5", "AXIS9-5", "AXIS10-5", "AXIS11-5",
    "AXIS0-6", "AXIS1-6", "AXIS2-6", "AXIS3-6", "AXIS4-6", "AXIS5-6", "AXIS6-6", "AXIS7-6", "AXIS8-6", "AXIS9-6", "AXIS10-6", "AXIS11-6",
    "AXIS0-7", "AXIS1-7", "AXIS2-7", "AXIS3-7", "AXIS4-7", "AXIS5-7", "AXIS6-7", "AXIS7-7", "AXIS8-7", "AXIS9-7", "AXIS10-7", "AXIS11-7"
};

static char *GfMouseBtn[] = {"MOUSE_LEFT_BTN", "MOUSE_MIDDLE_BTN", "MOUSE_RIGHT_BTN"}; /* glut order */

static char *GfMouseAxis[] = {"MOUSE_LEFT", "MOUSE_RIGHT", "MOUSE_UP", "MOUSE_DOWN"};

typedef struct
{
    char	*descr;
    int		val;
} tgfKeyBinding;

static tgfKeyBinding GfSKey[] = {
    {"F1",		GLUT_KEY_F1},
    {"F2",		GLUT_KEY_F2},
    {"F3",		GLUT_KEY_F3},
    {"F4",		GLUT_KEY_F4},
    {"F5",		GLUT_KEY_F5},
    {"F6",		GLUT_KEY_F6},
    {"F7",		GLUT_KEY_F7},
    {"F8",		GLUT_KEY_F8},
    {"F9",		GLUT_KEY_F9},
    {"F10",		GLUT_KEY_F10},
    {"F11",		GLUT_KEY_F11},
    {"F12",		GLUT_KEY_F12},
    {"Left Arrow",	GLUT_KEY_LEFT},
    {"Up Arrow",	GLUT_KEY_UP},
    {"Right Arrow",	GLUT_KEY_RIGHT},
    {"Down Arrow",	GLUT_KEY_DOWN},
    {"Page Up",		GLUT_KEY_PAGE_UP},
    {"Page Down",	GLUT_KEY_PAGE_DOWN},
    {"Home",		GLUT_KEY_HOME},
    {"End",		GLUT_KEY_END},
    {"Insert",		GLUT_KEY_INSERT}
};

static tgfKeyBinding GfKey[] = {
    {"backspace", 8},
    {"tab",       9},
    {"enter",     13},
    {"esc",       27},
    {"space",     ' '}
};

static int gfmaxJoyButton	= sizeof(GfJoyBtn)	/ sizeof(GfJoyBtn[0]);
static int gfmaxJoyAxis		= sizeof(GfJoyAxis)	/ sizeof(GfJoyAxis[0]);
static int gfmaxMouseButton	= sizeof(GfMouseBtn)	/ sizeof(GfMouseBtn[0]);
static int gfmaxMouseAxis	= sizeof(GfMouseAxis)	/ sizeof(GfMouseAxis[0]);
static int gfmaxSKey		= sizeof(GfSKey)	/ sizeof(GfSKey[0]);
static int gfmaxKey		= sizeof(GfKey)		/ sizeof(GfKey[0]);

static int gfctrlJoyPresent = GFCTRL_JOY_UNTESTED;
static jsJoystick *js[NUM_JOY] = {NULL};


/** Get a control reference by its name
    @ingroup	ctrl
    @param	name	name of the control
    @return	pointer on a static structure tCtrlRef
    @see	tCtrlRef
*/
tCtrlRef *
GfctrlGetRefByName(char *name)
{
    static tCtrlRef	ref;
    int 		i;

    if (!name || !strlen(name)) {
	ref.index = -1;
	ref.type = GFCTRL_TYPE_NOT_AFFECTED;
	return &ref;
    }
    for (i = 0; i < gfmaxJoyButton; i++) {
	if (strcmp(name, GfJoyBtn[i]) == 0) {
	    ref.index = i;
	    ref.type = GFCTRL_TYPE_JOY_BUT;
	    return &ref;
	}
    }
    for (i = 0; i < gfmaxJoyAxis; i++) {
	if (strcmp(name, GfJoyAxis[i]) == 0) {
	    ref.index = i;
	    ref.type = GFCTRL_TYPE_JOY_AXIS;
	    return &ref;
	}
    }
    for (i = 0; i < gfmaxMouseButton; i++) {
	if (strcmp(name, GfMouseBtn[i]) == 0) {
	    ref.index = i;
	    ref.type = GFCTRL_TYPE_MOUSE_BUT;
	    return &ref;
	}
    }
    for (i = 0; i < gfmaxMouseAxis; i++) {
	if (strcmp(name, GfMouseAxis[i]) == 0) {
	    ref.index = i;
	    ref.type = GFCTRL_TYPE_MOUSE_AXIS;
	    return &ref;
	}
    }
    for (i = 0; i < gfmaxSKey; i++) {
	if (strcmp(name, GfSKey[i].descr) == 0) {
	    ref.index = GfSKey[i].val;
	    ref.type = GFCTRL_TYPE_SKEYBOARD;
	    return &ref;
	}
    }
    for (i = 0; i < gfmaxKey; i++) {
	if (strcmp(name, GfKey[i].descr) == 0) {
	    ref.index = GfKey[i].val;
	    ref.type = GFCTRL_TYPE_KEYBOARD;
	    return &ref;
	}
    }
    ref.index = name[0];
    ref.type = GFCTRL_TYPE_KEYBOARD;
    return &ref;
}

/** Get a control name by its reference
    @ingroup	ctrl
    @param	type	type of control
    @param	index	reference index
    @return	pointer on a static structure tCtrlRef
*/
char *
GfctrlGetNameByRef(int type, int index)
{
    static char buf[4];
    int i;
    
    switch (type) {
    case GFCTRL_TYPE_NOT_AFFECTED:
	return NULL;
    case GFCTRL_TYPE_JOY_BUT:
	if (index < gfmaxJoyButton) {
	    return GfJoyBtn[index];
	} else {
	    return NULL;
	}
	break;
    case GFCTRL_TYPE_JOY_AXIS:
	if (index < gfmaxJoyAxis) {
	    return GfJoyAxis[index];
	} else {
	    return NULL;
	}
	break;
    case GFCTRL_TYPE_MOUSE_BUT:
	if (index < gfmaxMouseButton) {
	    return GfMouseBtn[index];
	} else {
	    return NULL;
	}
	break;
    case GFCTRL_TYPE_MOUSE_AXIS:
	if (index < gfmaxMouseAxis) {
	    return GfMouseAxis[index];
	} else {
	    return NULL;
	}
	break;
    case GFCTRL_TYPE_SKEYBOARD:
	for (i = 0; i < gfmaxSKey; i++) {
	    if (index == GfSKey[i].val) {
		return GfSKey[i].descr;
	    }
	}
	return NULL;
	break;
    case GFCTRL_TYPE_KEYBOARD:
	for (i = 0; i < gfmaxKey; i++) {
	    if (index == GfKey[i].val) {
		return GfKey[i].descr;
	    }
	}
	if (isprint(index)) {
	    sprintf(buf, "%c", index);
	    return buf;
	}
	return NULL;
	break;
    default:
	break;
    }
    return NULL;
}


static void
gfJoyFirstInit(void)
{
    int index;
    
    gfctrlJoyPresent = GFCTRL_JOY_NONE;

    for (index = 0; index < NUM_JOY; index++) {
	if (js[index] == NULL) {
	    js[index] = new jsJoystick(index);
	}
    
	if (js[index]->notWorking()) {
	    /* don't configure the joystick */
	    js[index] = NULL;
	} else {
	    gfctrlJoyPresent = GFCTRL_JOY_PRESENT;
	}
    }
}

/** Initialize the joystick control
    @ingroup	ctrl
    @return	pointer on a tCtrlJoyInfo structure
		<br>0 .. if no joystick present
    @note	call GfctrlJoyRelease to free the tCtrlJoyInfo structure
    @see	GfctrlJoyRelease
    @see	tCtrlJoyInfo
*/
tCtrlJoyInfo *
GfctrlJoyInit(void)
{
    tCtrlJoyInfo	*joyInfo = NULL;

    if (gfctrlJoyPresent == GFCTRL_JOY_UNTESTED) {
	gfJoyFirstInit();
    }

    joyInfo = (tCtrlJoyInfo *)calloc(1, sizeof(tCtrlJoyInfo));
    
    return joyInfo;
}

/** Release the tCtrlJoyInfo structure
    @ingroup	ctrl
    @param	joyInfo	joystick structure
    @return	none
*/
void
GfctrlJoyRelease(tCtrlJoyInfo *joyInfo)
{
    FREEZ(joyInfo);
}


/** Check if a joystick is present
    @ingroup	ctrl
    @return	GFCTRL_JOY_NONE	if no joystick
		<br>GFCTRL_JOY_PRESENT if a joystick is present
*/
int
GfctrlJoyIsPresent(void)
{
    if (gfctrlJoyPresent == GFCTRL_JOY_UNTESTED) {
	gfJoyFirstInit();
    }

    return gfctrlJoyPresent;
}


/** Get the joystick current values
    @ingroup	ctrl
    @param	joyInfo	joystick structure
    @return	<tt>0 ... </tt>Ok
		<br><tt>-1 .. </tt>Error
    @note	The tCtrlJoyInfo structure is updated with the new values
*/
int
GfctrlJoyGetCurrent(tCtrlJoyInfo *joyInfo)
{
    int			ind;
    int			i;
    int			b;
    unsigned int	mask;

    if (gfctrlJoyPresent == GFCTRL_JOY_PRESENT) {
    	for (ind = 0; ind < NUM_JOY; ind++) {
	    if (js[ind]) {
		js[ind]->read(&b, &(joyInfo->ax[MAX_AXES * ind]));

		/* Joystick buttons */
		for (i = 0, mask = 1; i < GFCTRL_JOY_MAXBUTTON; i++, mask *= 2) {
		    if (((b & mask) != 0) && ((joyInfo->oldb[ind] & mask) == 0)) {
			joyInfo->edgeup[i + GFCTRL_JOY_MAXBUTTON * ind] = 1;
		    } else {
			joyInfo->edgeup[i + GFCTRL_JOY_MAXBUTTON * ind] = 0;
		    }
		    if (((b & mask) == 0) && ((joyInfo->oldb[ind] & mask) != 0)) {
			joyInfo->edgedn[i + GFCTRL_JOY_MAXBUTTON * ind] = 1;
		    } else {
			joyInfo->edgedn[i + GFCTRL_JOY_MAXBUTTON * ind] = 0;
		    }
		    if ((b & mask) != 0) {
			joyInfo->levelup[i + GFCTRL_JOY_MAXBUTTON * ind] = 1;
		    } else {
			joyInfo->levelup[i + GFCTRL_JOY_MAXBUTTON * ind] = 0;
		    }
		}
		joyInfo->oldb[ind] = b;
	    }
	}
    } else {
	return -1;
    }

    return 0;
}




/** Initialize the mouse control
    @ingroup	ctrl
    @return	pointer on a tCtrlMouseInfo structure
		<br>0 .. if no mouse present
    @note	call GfctrlMouseRelease to free the tCtrlMouseInfo structure
    @see	GfctrlMouseRelease
*/
tCtrlMouseInfo *
GfctrlMouseInit(void)
{
    tCtrlMouseInfo	*mouseInfo = NULL;

    mouseInfo = (tCtrlMouseInfo *)calloc(1, sizeof(tCtrlMouseInfo));

    return mouseInfo;
}

/** Release the tCtrlMouseInfo structure
    @ingroup	ctrl
    @param	mouseInfo	mouse structure
    @return	none
*/
void
GfctrlMouseRelease(tCtrlMouseInfo *mouseInfo)
{
    FREEZ(mouseInfo);
}

static tMouseInfo refMouse;

/** Get the mouse current values
    @ingroup	ctrl
    @param	mouseInfo	mouse structure
    @return	<tt>0 ... </tt>Ok
		<br><tt>-1 .. </tt>Error
    @note	The tCtrlMouseInfo structure is updated with the new values
*/
int
GfctrlMouseGetCurrent(tCtrlMouseInfo *mouseInfo)
{
    float	mouseMove;
    tMouseInfo	*mouse;
    int		i;

    mouse = GfuiMouseInfo();

    mouseMove = (float)(refMouse.X - mouse->X);
    
    if (mouseMove < 0) {
	mouseInfo->ax[1] = -mouseMove;
	mouseInfo->ax[0] = 0;
    } else {
	mouseInfo->ax[0] = mouseMove;
	mouseInfo->ax[1] = 0;
    }
    mouseMove = (float)(refMouse.Y - mouse->Y);
    if (mouseMove < 0) {
	mouseInfo->ax[2] = -mouseMove;
	mouseInfo->ax[3] = 0;
    } else {
	mouseInfo->ax[3] = mouseMove;
	mouseInfo->ax[2] = 0;
    }
    for (i = 0; i < 3; i++) {
	if (mouseInfo->button[i] != mouse->button[i]) {
	    if (mouse->button[i]) {
		mouseInfo->edgedn[i] = 1;
		mouseInfo->edgeup[i] = 0;
	    } else {
		mouseInfo->edgeup[i] = 1;
		mouseInfo->edgedn[i] = 0;
	    }
	    mouseInfo->button[i] = mouse->button[i];
	} else {
	    mouseInfo->edgeup[i] = 0;
	    mouseInfo->edgedn[i] = 0;
	}
    }
    return 0;
}


/** Recentre the mouse on the screen.
    @ingroup	ctrl
    @return	none
*/
void
GfctrlMouseCenter(void)
{
    int sw, sh, vw, vh;

    GfScrGetSize(&sw, &sh, &vw, &vh);
    GfuiMouseSetPos(sw / 2, sh / 2);
}

/** Get the reference position.
    @ingroup	ctrl
    @return	none
*/
void
GfctrlMouseInitCenter(void)
{
    memcpy(&refMouse, GfuiMouseInfo(), sizeof(refMouse));
}
