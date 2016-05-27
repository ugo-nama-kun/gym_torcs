/***************************************************************************

    file        : controlconfig.cpp
    created     : Wed Mar 12 21:20:34 CET 2003
    copyright   : (C) 2003 by Eric Espi�                        
    email       : eric.espie@torcs.org   
    version     : $Id: controlconfig.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $                                  

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
    @version	$Id: controlconfig.cpp,v 1.5.2.1 2008/11/09 17:50:21 berniw Exp $
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
#include "joystickconfig.h"

static void 	*scrHandle = NULL;
static void	*prevHandle = NULL;
static void	*PrefHdle = NULL;

static int	MouseCalButton;
static int	JoyCalButton;

static tCtrlMouseInfo	mouseInfo;
static char	CurrentSection[256];

static tCmdInfo Cmd[] = {
    {HM_ATT_GEAR_R,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_GEAR_N,     {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_UP_SHFT,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_DN_SHFT,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_LIGHT1_CMD, {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_ASR_CMD,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_LEFTSTEER,  {1,  GFCTRL_TYPE_MOUSE_AXIS},   0, HM_ATT_LEFTSTEER_MIN,  0, HM_ATT_LEFTSTEER_MAX,  0, HM_ATT_LEFTSTEER_POW,  1.0, 1},
    {HM_ATT_RIGHTSTEER, {2,  GFCTRL_TYPE_MOUSE_AXIS},   0, HM_ATT_RIGHTSTEER_MIN, 0, HM_ATT_RIGHTSTEER_MAX, 0, HM_ATT_RIGHTSTEER_POW, 1.0, 1},
    {HM_ATT_THROTTLE,   {1,  GFCTRL_TYPE_MOUSE_BUT},    0, HM_ATT_THROTTLE_MIN,   0, HM_ATT_THROTTLE_MAX,   0, HM_ATT_THROTTLE_POW,   1.0, 1},
    {HM_ATT_BRAKE,      {2,  GFCTRL_TYPE_MOUSE_BUT},    0, HM_ATT_BRAKE_MIN,      0, HM_ATT_BRAKE_MAX,      0, HM_ATT_BRAKE_POW,      1.0, 1},
    {HM_ATT_CLUTCH,     {3,  GFCTRL_TYPE_MOUSE_BUT},    0, HM_ATT_CLUTCH_MIN,     0, HM_ATT_CLUTCH_MAX,     0, HM_ATT_CLUTCH_POW,     1.0, 1},
    {HM_ATT_ABS_CMD,    {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1},
    {HM_ATT_SPDLIM_CMD, {-1, GFCTRL_TYPE_NOT_AFFECTED}, 0, 0, 0, 0, 0, 0, 0, 1}
};

static int maxCmd = sizeof(Cmd) / sizeof(Cmd[0]);

static jsJoystick	*js[NUM_JOY] = {NULL};
static float		ax[MAX_AXES * NUM_JOY] = {0};
static float 		axCenter[MAX_AXES * NUM_JOY];
static int		rawb[NUM_JOY] = {0};

static float SteerSensVal;
static float DeadZoneVal;

static char buf[1024];

static int SteerSensEditId;
static int DeadZoneEditId;

static int ReloadValues = 1;

static void
onSteerSensChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(scrHandle, SteerSensEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	sprintf(buf, "%f", fv);
	SteerSensVal = fv;
	GfuiEditboxSetString(scrHandle, SteerSensEditId, buf);
    } else {
	GfuiEditboxSetString(scrHandle, SteerSensEditId, "");
    }
    
}

static void
onDeadZoneChange(void * /* dummy */)
{
    char	*val;
    float	fv;

    val = GfuiEditboxGetString(scrHandle, DeadZoneEditId);
    if (sscanf(val, "%f", &fv) == 1) {
	sprintf(buf, "%f", fv);
	DeadZoneVal = fv;
	GfuiEditboxSetString(scrHandle, DeadZoneEditId, buf);
    } else {
	GfuiEditboxSetString(scrHandle, SteerSensEditId, "");
    }
    
}

static void
onSave(void * /* dummy */)
{
    int		i;
    char	*str;

    GfParmSetNum(PrefHdle, CurrentSection, HM_ATT_STEER_SENS, NULL, SteerSensVal);
    GfParmSetNum(PrefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);

    for (i = 0; i < maxCmd; i++) {
	str = GfctrlGetNameByRef(Cmd[i].ref.type, Cmd[i].ref.index);
	if (str) {
	    GfParmSetStr(PrefHdle, CurrentSection, Cmd[i].name, str);
	} else {
	    GfParmSetStr(PrefHdle, CurrentSection, Cmd[i].name, "");
	}
	if (Cmd[i].minName) {
	    GfParmSetNum(PrefHdle, CurrentSection, Cmd[i].minName, NULL, Cmd[i].min);
	}
	if (Cmd[i].maxName) {
	    GfParmSetNum(PrefHdle, CurrentSection, Cmd[i].maxName, NULL, Cmd[i].max);
	}
	if (Cmd[i].powName) {
	    GfParmSetNum(PrefHdle, CurrentSection, Cmd[i].powName, NULL, Cmd[i].pow);
	}
    }

    GfParmWriteFile(NULL, PrefHdle, "preferences");
    GfuiScreenActivate(prevHandle);
}

static void
updateButtonText(void)
{
    int		i;
    char	*str;
    int		displayMouseCal = GFUI_INVISIBLE;
    int		displayJoyCal = GFUI_INVISIBLE;

    for (i = 0; i < maxCmd; i++) {
	str = GfctrlGetNameByRef(Cmd[i].ref.type, Cmd[i].ref.index);
	if (str) {
	    GfuiButtonSetText (scrHandle, Cmd[i].Id, str);
	} else {
	    GfuiButtonSetText (scrHandle, Cmd[i].Id, "---");
	}
	if (Cmd[i].ref.type == GFCTRL_TYPE_MOUSE_AXIS) {
	    displayMouseCal = GFUI_VISIBLE;
	} else if (Cmd[i].ref.type == GFCTRL_TYPE_JOY_AXIS) {
	    displayJoyCal = GFUI_VISIBLE;
	}
    }

    sprintf(buf, "%f", SteerSensVal);
    GfuiEditboxSetString(scrHandle, SteerSensEditId, buf);

    sprintf(buf, "%f", DeadZoneVal);
    GfuiEditboxSetString(scrHandle, DeadZoneEditId, buf);

    GfuiVisibilitySet(scrHandle, MouseCalButton, displayMouseCal);
    GfuiVisibilitySet(scrHandle, JoyCalButton, displayJoyCal);
}

static void
onFocusLost(void * /* dummy */)
{
    updateButtonText();
}

static int CurrentCmd;

static int InputWaited = 0;

static int
onKeyAction(unsigned char key, int /* modifier */, int state)
{
    char *name;

    if (!InputWaited || (state == GFUI_KEY_UP)) {
	return 0;
    }
    if (key == 27) {
	/* escape */
	Cmd[CurrentCmd].ref.index = -1;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_NOT_AFFECTED;
	GfParmSetStr(PrefHdle, CurrentSection, Cmd[CurrentCmd].name, "");
    } else {
	name = GfctrlGetNameByRef(GFCTRL_TYPE_KEYBOARD, (int)key);
	Cmd[CurrentCmd].ref.index = (int)key;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_KEYBOARD;
	GfParmSetStr(PrefHdle, CurrentSection, Cmd[CurrentCmd].name, name);
    }

    glutIdleFunc(GfuiIdle);
    InputWaited = 0;
    updateButtonText();
    return 1;
}

static int
onSKeyAction(int key, int /* modifier */, int state)
{
    char *name;

    if (!InputWaited || (state == GFUI_KEY_UP)) {
	return 0;
    }
    name = GfctrlGetNameByRef(GFCTRL_TYPE_SKEYBOARD, key);
    Cmd[CurrentCmd].ref.index = key;
    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_SKEYBOARD;
    GfParmSetStr(PrefHdle, CurrentSection, Cmd[CurrentCmd].name, name);

    glutIdleFunc(GfuiIdle);
    InputWaited = 0;
    updateButtonText();
    return 1;
}

static int
getMovedAxis(void)
{
    int		i;
    int		Index = -1;
    float	maxDiff = 0.3;

    for (i = 0; i < MAX_AXES * NUM_JOY; i++) {
	if (maxDiff < fabs(ax[i] - axCenter[i])) {
	    maxDiff = fabs(ax[i] - axCenter[i]);
	    Index = i;
	}
    }
    return Index;
}

static void
Idle(void)
{
    int		mask;
    int		b, i;
    int		index;
    char	*str;
    int		axis;

    GfctrlMouseGetCurrent(&mouseInfo);

    /* Check for a mouse button pressed */
    for (i = 0; i < 3; i++) {
	if (mouseInfo.edgedn[i]) {
	    glutIdleFunc(GfuiIdle);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_BUT, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_BUT;
	    GfuiButtonSetText (scrHandle, Cmd[CurrentCmd].Id, str);
	    glutPostRedisplay();
	    return;
	}
    }

    /* Check for a mouse axis moved */
    for (i = 0; i < 4; i++) {
	if (mouseInfo.ax[i] > 20.0) {
	    glutIdleFunc(GfuiIdle);
	    InputWaited = 0;
	    str = GfctrlGetNameByRef(GFCTRL_TYPE_MOUSE_AXIS, i);
	    Cmd[CurrentCmd].ref.index = i;
	    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_MOUSE_AXIS;
	    GfuiButtonSetText (scrHandle, Cmd[CurrentCmd].Id, str);
	    glutPostRedisplay();
	    return;
	}
    }

    /* Check for a Joystick button pressed */
    for (index = 0; index < NUM_JOY; index++) {
	if (js[index]) {
	    js[index]->read(&b, &ax[index * MAX_AXES]);

	    /* Joystick buttons */
	    for (i = 0, mask = 1; i < 32; i++, mask *= 2) {
		if (((b & mask) != 0) && ((rawb[index] & mask) == 0)) {
		    /* Button i fired */
		    glutIdleFunc(GfuiIdle);
		    InputWaited = 0;
		    str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_BUT, i + 32 * index);
		    Cmd[CurrentCmd].ref.index = i + 32 * index;
		    Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_BUT;
		    GfuiButtonSetText (scrHandle, Cmd[CurrentCmd].Id, str);
		    glutPostRedisplay();
		    rawb[index] = b;
		    return;
		}
	    }
	    rawb[index] = b;
	}
    }

    /* detect joystick movement */
    axis = getMovedAxis();
    if (axis != -1) {
	glutIdleFunc(GfuiIdle);
	InputWaited = 0;
	Cmd[CurrentCmd].ref.type = GFCTRL_TYPE_JOY_AXIS;
	Cmd[CurrentCmd].ref.index = axis;
	str = GfctrlGetNameByRef(GFCTRL_TYPE_JOY_AXIS, axis);
	GfuiButtonSetText (scrHandle, Cmd[CurrentCmd].Id, str);
	glutPostRedisplay();
	return;
    }
}

static void
onPush(void *vi)
{
    int		index;    
    long	i = (long)vi;
    
    CurrentCmd = i;
    GfuiButtonSetText (scrHandle, Cmd[i].Id, "");
    Cmd[i].ref.index = -1;
    Cmd[i].ref.type = GFCTRL_TYPE_NOT_AFFECTED;
    GfParmSetStr(PrefHdle, CurrentSection, Cmd[i].name, "");
    if (Cmd[CurrentCmd].keyboardPossible) {
	InputWaited = 1;
    }
    glutIdleFunc(Idle);
    GfctrlMouseInitCenter();
    memset(&mouseInfo, 0, sizeof(mouseInfo));
    GfctrlMouseGetCurrent(&mouseInfo);

    for (index = 0; index < NUM_JOY; index++) {
	if (js[index]) {
	    js[index]->read(&rawb[index], &ax[index * MAX_AXES]); /* initial value */
	}
    }
    memcpy(axCenter, ax, sizeof(axCenter));
}

static void
onActivate(void * /* dummy */)
{
    int		cmd;
    char	*prm;
    tCtrlRef	*ref;

    if (ReloadValues) {
	sprintf(buf, "%s%s", GetLocalDir(), HM_PREF_FILE);
	PrefHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	/* Mouse Settings */
	for (cmd = 0; cmd < maxCmd; cmd++) {
	    prm = GfctrlGetNameByRef(Cmd[cmd].ref.type, Cmd[cmd].ref.index);
	    if (!prm) {
		prm = "---";
	    }
	    prm = GfParmGetStr(PrefHdle, HM_SECT_MOUSEPREF, Cmd[cmd].name, prm);
	    prm = GfParmGetStr(PrefHdle, CurrentSection, Cmd[cmd].name, prm);
	    ref = GfctrlGetRefByName(prm);
	    Cmd[cmd].ref.type = ref->type;
	    Cmd[cmd].ref.index = ref->index;
	    if (Cmd[cmd].minName) {
		Cmd[cmd].min = GfParmGetNum(PrefHdle, HM_SECT_MOUSEPREF, Cmd[cmd].minName, NULL, Cmd[cmd].min);
		Cmd[cmd].min = GfParmGetNum(PrefHdle, CurrentSection, Cmd[cmd].minName, NULL, Cmd[cmd].min);
	    }
	    if (Cmd[cmd].maxName) {
		Cmd[cmd].max = GfParmGetNum(PrefHdle, HM_SECT_MOUSEPREF, Cmd[cmd].maxName, NULL, Cmd[cmd].max);
		Cmd[cmd].max = GfParmGetNum(PrefHdle, CurrentSection, Cmd[cmd].maxName, NULL, Cmd[cmd].max);
	    }
	    if (Cmd[cmd].powName) {
		Cmd[cmd].pow = GfParmGetNum(PrefHdle, HM_SECT_MOUSEPREF, Cmd[cmd].powName, NULL, Cmd[cmd].pow);
		Cmd[cmd].pow = GfParmGetNum(PrefHdle, CurrentSection, Cmd[cmd].powName, NULL, Cmd[cmd].pow);
	    }
	}

	SteerSensVal = GfParmGetNum(PrefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_SENS, NULL, 0);
	SteerSensVal = GfParmGetNum(PrefHdle, CurrentSection, HM_ATT_STEER_SENS, NULL, SteerSensVal);
	DeadZoneVal = GfParmGetNum(PrefHdle, HM_SECT_MOUSEPREF, HM_ATT_STEER_DEAD, NULL, 0);
	DeadZoneVal = GfParmGetNum(PrefHdle, CurrentSection, HM_ATT_STEER_DEAD, NULL, DeadZoneVal);
    }
    
    updateButtonText();
}

static void
DevCalibrate(void *menu)
{
    ReloadValues = 0;
    GfuiScreenActivate(menu);
}


void *
TorcsControlMenuInit(void *prevMenu, int idx)
{
    int		x, y, x2, dy, i;
    int		index;

    ReloadValues = 1;
    sprintf(CurrentSection, "%s/%d", HM_SECT_DRVPREF, idx);

    prevHandle = prevMenu;
    sprintf(buf, "%s%s", GetLocalDir(), HM_PREF_FILE);
    PrefHdle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

    if (scrHandle) {
	return scrHandle;
    }

    for (index = 0; index < NUM_JOY; index++) {
	if (js[index] == NULL) {
	    js[index] = new jsJoystick(index);
	}
    
	if (js[index]->notWorking()) {
	    /* don't configure the joystick */
	    js[index] = NULL;
	}
    }
    
    scrHandle = GfuiScreenCreateEx((float*)NULL, NULL, onActivate, NULL, (tfuiCallback)NULL, 1);
    GfuiTitleCreate(scrHandle, "Control Configuration", 0);

    GfuiScreenAddBgImg(scrHandle, "data/img/splash-mouseconf.png");

    GfuiMenuDefaultKeysAdd(scrHandle);

    x = 10;
    x2 = 210;
    y = 340;
    dy = 30;

    for (i = 0; i < maxCmd; i++) {
	GfuiLabelCreate(scrHandle, Cmd[i].name, GFUI_FONT_MEDIUM, x, y, GFUI_ALIGN_HL_VB, 0);
	Cmd[i].Id = GfuiButtonStateCreate (scrHandle, "MOUSE_MIDDLE_BUTTON", GFUI_FONT_MEDIUM_C, x+x2, y, 0, GFUI_ALIGN_HC_VB, GFUI_MOUSE_DOWN, 
					   (void*)i, onPush, NULL, (tfuiCallback)NULL, onFocusLost);
	y -= dy;
	if (i == (maxCmd / 2 - 1)) {
	    x = 320;
	    y = 340;
	    x2 = 220;
	}
    }

    GfuiLabelCreate(scrHandle, "Steer Sensibility", GFUI_FONT_MEDIUM, 30, 90, GFUI_ALIGN_HL_VB, 0);
    SteerSensEditId = GfuiEditboxCreate(scrHandle, "", GFUI_FONT_MEDIUM_C,
					200, 90, 80, 8, NULL, (tfuiCallback)NULL, onSteerSensChange);

    GfuiLabelCreate(scrHandle, "Steer Dead Zone", GFUI_FONT_MEDIUM, 340, 90, GFUI_ALIGN_HL_VB, 0);
    DeadZoneEditId = GfuiEditboxCreate(scrHandle, "", GFUI_FONT_MEDIUM_C,
					510, 90, 80, 8, NULL, (tfuiCallback)NULL, onDeadZoneChange);


    GfuiAddKey(scrHandle, 13, "Save", NULL, onSave, NULL);
    GfuiButtonCreate(scrHandle, "Save", GFUI_FONT_LARGE, 160, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     NULL, onSave, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    MouseCalButton = GfuiButtonCreate(scrHandle, "Calibrate", GFUI_FONT_LARGE, 320, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
				      MouseCalMenuInit(scrHandle, Cmd, maxCmd), DevCalibrate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    JoyCalButton = GfuiButtonCreate(scrHandle, "Calibrate", GFUI_FONT_LARGE, 320, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
				    JoyCalMenuInit(scrHandle, Cmd, maxCmd), DevCalibrate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);

    GfuiAddKey(scrHandle, 27, "Cancel", prevMenu, GfuiScreenActivate, NULL);
    GfuiButtonCreate(scrHandle, "Cancel", GFUI_FONT_LARGE, 480, 40, 150, GFUI_ALIGN_HC_VB, GFUI_MOUSE_UP,
		     prevMenu, GfuiScreenActivate, NULL, (tfuiCallback)NULL, (tfuiCallback)NULL);


    GfuiKeyEventRegister(scrHandle, onKeyAction);
    GfuiSKeyEventRegister(scrHandle, onSKeyAction);

    return scrHandle;
}
