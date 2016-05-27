/***************************************************************************

    file                 : human.cpp
    created              : Sat Mar 18 23:16:38 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: human.cpp,v 1.45.2.4 2008/11/09 20:14:35 berniw Exp $

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: human.cpp,v 1.45.2.4 2008/11/09 20:14:35 berniw Exp $
*/


#ifdef _WIN32
#include <windows.h>
#define isnan _isnan
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <js.h>

#include <tgfclient.h>

#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#include <playerpref.h>
#include "pref.h"
#include "human.h"

#define DRWD 0
#define DFWD 1
#define D4WD 2

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void drive_mt(int index, tCarElt* car, tSituation *s);
static void drive_at(int index, tCarElt* car, tSituation *s);
static void newrace(int index, tCarElt* car, tSituation *s);
static int  pitcmd(int index, tCarElt* car, tSituation *s);

int joyPresent = 0;

static char	sstring[1024];
static char	buf[1024];

static tTrack	*curTrack;
//static void	*DrvInfo;

static float color[] = {0.0, 0.0, 1.0, 1.0};

static tCtrlJoyInfo	*joyInfo = NULL;
static tCtrlMouseInfo	*mouseInfo = NULL;
static int		masterPlayer = -1;

tHumanContext *HCtx[10] = {0};

static int speedLimiter	= 0;
static tdble Vtarget;


typedef struct
{
	int state;
	int edgeDn;
	int edgeUp;
} tKeyInfo;

static tKeyInfo keyInfo[256];
static tKeyInfo skeyInfo[256];

static int currentKey[256];
static int currentSKey[256];

static tdble lastKeyUpdate = -10.0;

static int	firstTime = 0;

#ifdef _WIN32
/* should be present in mswindows */
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
    return TRUE;
}
#endif

static void
shutdown(int index)
{
	//static int	firstTime = 1;
	int		idx = index - 1;

	free (HCtx[idx]);

	if (firstTime) {
		//GfParmReleaseHandle(DrvInfo);
		GfParmReleaseHandle(PrefHdle);
		GfctrlJoyRelease(joyInfo);
		GfctrlMouseRelease(mouseInfo);
		GfuiKeyEventRegisterCurrent(NULL);
		GfuiSKeyEventRegisterCurrent(NULL);
		firstTime = 0;
	}
}



/*
 * Function
 *	InitFuncPt
 *
 * Description
 *	Robot functions initialisation
 *
 * Parameters
 *	pt	pointer on functions structure
 *
 * Return
 *	0
 *
 * Remarks
 *
 */
static int
InitFuncPt(int index, void *pt)
{
	tRobotItf *itf = (tRobotItf *)pt;
	int idx = index - 1;

	if (masterPlayer == -1) {
		masterPlayer = index;
	}

	if (firstTime < 1) {
		firstTime = 1;
		//DrvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
		joyInfo = GfctrlJoyInit();
		if (joyInfo) {
			joyPresent = 1;
		}

		mouseInfo = GfctrlMouseInit();
	}


	/* Allocate a new context for that player */
	HCtx[idx] = (tHumanContext *) calloc (1, sizeof (tHumanContext));

	HCtx[idx]->ABS = 1.0;
	HCtx[idx]->AntiSlip = 1.0;

	itf->rbNewTrack = initTrack;	/* give the robot the track view called */
	/* for every track change or new race */
	itf->rbNewRace  = newrace;

	HmReadPrefs(index);

	if (HCtx[idx]->Transmission == 0) {
		itf->rbDrive    = drive_at;
	} else {
		itf->rbDrive    = drive_mt;		/* drive during race */
	}
	itf->rbShutdown = shutdown;
	itf->rbPitCmd   = pitcmd;
	itf->index      = index;

	return 0;
}

/*
 * Function
 *	human
 *
 * Description
 *	DLL entry point (general to all types of modules)
 *
 * Parameters
 *	modInfo	administrative info on module
 *
 * Return
 *	0
 *
 * Remarks
 *
 */

#define MAXNAMELEN 100

static char names[10][MAXNAMELEN];

extern "C" int
human(tModInfo *modInfo)
{
	int i;
	char *driver;

	memset(modInfo, 0, 10*sizeof(tModInfo));

	sprintf(buf, "%sdrivers/human/human.xml", GetLocalDir());
	void *DrvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);

	if (DrvInfo != NULL) {
		for (i = 0; i < 10; i++) {
			sprintf(sstring, "Robots/index/%d", i+1);
			driver = GfParmGetStr(DrvInfo, sstring, "name", "");
			if (strlen(driver) == 0) {
				break;
			}
			strncpy(names[i], driver, MAXNAMELEN);
			modInfo->name    = names[i];	/* name of the module (short) */
			modInfo->desc    = strdup("Joystick controlable driver");	/* description of the module (can be long) */
			modInfo->fctInit = InitFuncPt;	/* init function */
			modInfo->gfId    = ROB_IDENT;	/* supported framework version */
			modInfo->index   = i+1;
			modInfo++;
		}
		// Just release in case we got it.
		GfParmReleaseHandle(DrvInfo);
	}

	return 0;
}


/*
 * Function
 *
 *
 * Description
 *	search under drivers/human/tracks/<trackname>/car-<model>-<index>.xml
 *		     drivers/human/car-<model>-<index>.xml
 *		     drivers/human/tracks/<trackname>/car-<model>.xml
 *		     drivers/human/car-<model>.xml
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
static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	char *carname;
	char *s1, *s2;
	char trackname[256];
	tdble fuel;
	int idx = index - 1;

	curTrack = track;
	s1 = strrchr(track->filename, '/') + 1;
	s2 = strchr(s1, '.');
	strncpy(trackname, s1, s2-s1);
	trackname[s2-s1] = 0;
	sprintf(sstring, "Robots/index/%d", index);

	sprintf(buf, "%sdrivers/human/human.xml", GetLocalDir());
	void *DrvInfo = GfParmReadFile(buf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	carname = "";
	if (DrvInfo != NULL) {
		carname = GfParmGetStr(DrvInfo, sstring, "car name", "");
	}

	sprintf(sstring, "%sdrivers/human/tracks/%s/car-%s-%d.xml", GetLocalDir(), trackname, carname, index);
	*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
	if (*carParmHandle != NULL) {
		GfOut("Player: %s Loaded\n", sstring);
	} else {
		sprintf(sstring, "%sdrivers/human/tracks/%s/car-%s.xml", GetLocalDir(), trackname, carname);
		*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
		if (*carParmHandle != NULL) {
			GfOut("Player: %s Loaded\n", sstring);
		} else {
			sprintf(sstring, "%sdrivers/human/car-%s-%d.xml", GetLocalDir(), carname, index);
			*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
			if (*carParmHandle != NULL) {
				GfOut("Player: %s Loaded\n", sstring);
			} else {
				sprintf(sstring, "%sdrivers/human/car-%s.xml", GetLocalDir(), carname);
				*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
				if (*carParmHandle != NULL) {
					GfOut("Player: %s Loaded\n", sstring);
				} else {
					sprintf(sstring, "%sdrivers/human/car.xml", GetLocalDir ());
					*carParmHandle = GfParmReadFile(sstring, GFPARM_RMODE_REREAD);
					if (*carParmHandle != NULL) {
						GfOut("Player: %s Loaded\n", sstring);
					}
				}
			}
		}
	}

	if (curTrack->pits.type != TR_PIT_NONE) {
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		HCtx[idx]->NbPitStopProg = (int)GfParmGetNum(PrefHdle, sstring, HM_ATT_NBPITS, (char*)NULL, 0);
		GfOut("Player: index %d , Pits stops %d\n", index, HCtx[idx]->NbPitStopProg);
	} else {
		HCtx[idx]->NbPitStopProg = 0;
	}
	fuel = 0.0008 * curTrack->length * (s->_totLaps + 1) / (1.0 + ((tdble)HCtx[idx]->NbPitStopProg)) + 20.0;
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, fuel);
	Vtarget = curTrack->pits.speedLimit;
	if (DrvInfo != NULL) {
		GfParmReleaseHandle(DrvInfo);
	}
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
 */

void newrace(int index, tCarElt* car, tSituation *s)
{
	int i;
	int idx = index - 1;

	for (i = 0; i < MAX_GEARS; i++) {
		if (car->_gearRatio[i] != 0) {
			HCtx[idx]->shiftThld[i] = car->_enginerpmRedLine * car->_wheelRadius(2) * 0.85 / car->_gearRatio[i];
			GfOut("Gear %d: Spd %f\n", i, HCtx[idx]->shiftThld[i] * 3.6);
		} else {
	    	HCtx[idx]->shiftThld[i] = 10000.0;
		}
	}

	if (HCtx[idx]->MouseControlUsed) {
		GfctrlMouseCenter();
	}

	memset(keyInfo, 0, sizeof(keyInfo));
	memset(skeyInfo, 0, sizeof(skeyInfo));

	memset(currentKey, 0, sizeof(currentKey));
	memset(currentSKey, 0, sizeof(currentSKey));


#ifndef WIN32
#ifdef TELEMETRY
	if (s->_raceType == RM_TYPE_PRACTICE) {
		RtTelemInit(-10, 10);
		RtTelemNewChannel("Dist", &HCtx[idx]->distToStart, 0, 0);
		RtTelemNewChannel("Ax", &car->_accel_x, 0, 0);
		RtTelemNewChannel("Ay", &car->_accel_y, 0, 0);
		RtTelemNewChannel("Steer", &car->ctrl->steer, 0, 0);
		RtTelemNewChannel("Throttle", &car->ctrl->accelCmd, 0, 0);
		RtTelemNewChannel("Brake", &car->ctrl->brakeCmd, 0, 0);
		RtTelemNewChannel("Gear", &HCtx[idx]->Gear, 0, 0);
		RtTelemNewChannel("Speed", &car->_speed_x, 0, 0);
	}
#endif
#endif

	char *traintype =GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
		HCtx[idx]->drivetrain = DRWD;
	} else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
		HCtx[idx]->drivetrain = DFWD;
	} else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
		HCtx[idx]->drivetrain = D4WD;
	} 

	tControlCmd	*cmd = HCtx[idx]->CmdControl;
	if (cmd[CMD_CLUTCH].type != GFCTRL_TYPE_JOY_AXIS && 
			cmd[CMD_CLUTCH].type != GFCTRL_TYPE_MOUSE_AXIS)
		HCtx[idx]->autoClutch = 1;
	else
		HCtx[idx]->autoClutch = 0;
}

static void
updateKeys(void)
{
	int i;
	int key;
	int idx;
	tControlCmd *cmd;

	for (idx = 0; idx < 10; idx++) {
		if (HCtx[idx]) {
			cmd = HCtx[idx]->CmdControl;
			for (i = 0; i < nbCmdControl; i++) {
				if (cmd[i].type == GFCTRL_TYPE_KEYBOARD) {
					key = cmd[i].val;
					if (currentKey[key] == GFUI_KEY_DOWN) {
						if (keyInfo[key].state == GFUI_KEY_UP) {
							keyInfo[key].edgeDn = 1;
						} else {
							keyInfo[key].edgeDn = 0;
						}
					} else {
						if (keyInfo[key].state == GFUI_KEY_DOWN) {
							keyInfo[key].edgeUp = 1;
						} else {
							keyInfo[key].edgeUp = 0;
						}
					}
					keyInfo[key].state = currentKey[key];
				}

				if (cmd[i].type == GFCTRL_TYPE_SKEYBOARD) {
					key = cmd[i].val;
					if (currentSKey[key] == GFUI_KEY_DOWN) {
						if (skeyInfo[key].state == GFUI_KEY_UP) {
							skeyInfo[key].edgeDn = 1;
						} else {
							skeyInfo[key].edgeDn = 0;
						}
					} else {
						if (skeyInfo[key].state == GFUI_KEY_DOWN) {
							skeyInfo[key].edgeUp = 1;
						} else {
							skeyInfo[key].edgeUp = 0;
						}
					}
					skeyInfo[key].state = currentSKey[key];
				}
			}
		}
    }
}


static int
onKeyAction(unsigned char key, int modifier, int state)
{
	currentKey[key] = state;

	return 0;
}

static int
onSKeyAction(int key, int modifier, int state)
{
	currentSKey[key] = state;

	return 0;
}

static void common_drive(int index, tCarElt* car, tSituation *s)
{
	tdble slip;
	tdble ax0;
	tdble brake;
	tdble clutch;
	tdble throttle;
	tdble leftSteer;
	tdble rightSteer;
	int scrw, scrh, dummy;
	int idx = index - 1;
	tControlCmd	*cmd = HCtx[idx]->CmdControl;

	static int firstTime = 1;

	if (firstTime) {
		if (HCtx[idx]->MouseControlUsed) {
	    	GfuiMouseShow();
	    	GfctrlMouseInitCenter();
		}
		GfuiKeyEventRegisterCurrent(onKeyAction);
		GfuiSKeyEventRegisterCurrent(onSKeyAction);
		firstTime = 0;
    }


	HCtx[idx]->distToStart = RtGetDistFromStart(car);

	HCtx[idx]->Gear = (tdble)car->_gear;	/* telemetry */

	GfScrGetSize(&scrw, &scrh, &dummy, &dummy);

	memset(&(car->ctrl), 0, sizeof(tCarCtrl));

	car->_lightCmd = HCtx[idx]->lightCmd;

	if (car->_laps != HCtx[idx]->LastPitStopLap) {
		car->_raceCmd = RM_CMD_PIT_ASKED;
	}

	if (lastKeyUpdate != s->currentTime) {
		/* Update the controls only once for all the players */
		updateKeys();

		if (joyPresent) {
			GfctrlJoyGetCurrent(joyInfo);
		}

		GfctrlMouseGetCurrent(mouseInfo);
		lastKeyUpdate = s->currentTime;
	}

	if (((cmd[CMD_ABS].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_ABS].val]) ||
	((cmd[CMD_ABS].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_ABS].val].edgeUp) ||
	((cmd[CMD_ABS].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_ABS].val].edgeUp))
	{
		HCtx[idx]->ParamAbs = 1 - HCtx[idx]->ParamAbs;
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ABS, Yn[1 - HCtx[idx]->ParamAbs]);
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	if (((cmd[CMD_ASR].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_ASR].val]) ||
	((cmd[CMD_ASR].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_ASR].val].edgeUp) ||
	((cmd[CMD_ASR].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_ASR].val].edgeUp))
	{
		HCtx[idx]->ParamAsr = 1 - HCtx[idx]->ParamAsr;
		sprintf(sstring, "%s/%s/%d", HM_SECT_PREF, HM_LIST_DRV, index);
		GfParmSetStr(PrefHdle, sstring, HM_ATT_ASR, Yn[1 - HCtx[idx]->ParamAsr]);
		GfParmWriteFile(NULL, PrefHdle, "Human");
	}

	sprintf(car->_msgCmd[0], "%s %s", (HCtx[idx]->ParamAbs ? "ABS" : ""), (HCtx[idx]->ParamAsr ? "ASR" : ""));
	memcpy(car->_msgColorCmd, color, sizeof(car->_msgColorCmd));

	if (((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_JOY_BUT) && (joyInfo->levelup[cmd[CMD_SPDLIM].val] == 1)) ||
	((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_KEYBOARD) && (keyInfo[cmd[CMD_SPDLIM].val].state == GFUI_KEY_DOWN)) ||
	((cmd[CMD_SPDLIM].type == GFCTRL_TYPE_SKEYBOARD) && (skeyInfo[cmd[CMD_SPDLIM].val].state == GFUI_KEY_DOWN)))
	{
		speedLimiter = 1;
		sprintf(car->_msgCmd[1], "Speed Limiter On");
	} else {
		speedLimiter = 0;
		sprintf(car->_msgCmd[1], "Speed Limiter Off");
	}


	if (((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_LIGHT1].val]) ||
	((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_LIGHT1].val].edgeUp) ||
	((cmd[CMD_LIGHT1].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_LIGHT1].val].edgeUp))
	{
		if (HCtx[idx]->lightCmd & RM_LIGHT_HEAD1) {
			HCtx[idx]->lightCmd &= ~(RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2);
		} else {
			HCtx[idx]->lightCmd |= RM_LIGHT_HEAD1 | RM_LIGHT_HEAD2;
		}
	}

	switch (cmd[CMD_LEFTSTEER].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			ax0 = joyInfo->ax[cmd[CMD_LEFTSTEER].val];
			if (ax0 > cmd[CMD_LEFTSTEER].max) {
				ax0 = cmd[CMD_LEFTSTEER].max;
			} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
				ax0 = cmd[CMD_LEFTSTEER].min;
			}
			leftSteer = -SIGN(ax0) * cmd[CMD_LEFTSTEER].pow * pow(fabs(ax0), cmd[CMD_LEFTSTEER].sens) / (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x);
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_LEFTSTEER].val] - cmd[CMD_LEFTSTEER].deadZone;
			if (ax0 > cmd[CMD_LEFTSTEER].max) {
				ax0 = cmd[CMD_LEFTSTEER].max;
			} else if (ax0 < cmd[CMD_LEFTSTEER].min) {
				ax0 = cmd[CMD_LEFTSTEER].min;
			}
			ax0 = ax0 * cmd[CMD_LEFTSTEER].pow;
			leftSteer = pow(fabs(ax0), cmd[CMD_LEFTSTEER].sens) / (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x / 10.0);
			break;
		case GFCTRL_TYPE_KEYBOARD:
		case GFCTRL_TYPE_SKEYBOARD:
		case GFCTRL_TYPE_JOY_BUT:
			if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
				ax0 = keyInfo[cmd[CMD_LEFTSTEER].val].state;
			} else if (cmd[CMD_LEFTSTEER].type == GFCTRL_TYPE_SKEYBOARD) {
				ax0 = skeyInfo[cmd[CMD_LEFTSTEER].val].state;
			} else {
				ax0 = joyInfo->levelup[cmd[CMD_LEFTSTEER].val];
			}
			if (ax0 == 0) {
				HCtx[idx]->prevLeftSteer = leftSteer = 0;
			} else {
				ax0 = 2 * ax0 - 1;
				leftSteer = HCtx[idx]->prevLeftSteer + ax0 * cmd[CMD_LEFTSTEER].sens * s->deltaTime / (1.0 + cmd[CMD_LEFTSTEER].spdSens * car->_speed_x / 10.0);
				if (leftSteer > 1.0) leftSteer = 1.0;
				if (leftSteer < 0.0) leftSteer = 0.0;
				HCtx[idx]->prevLeftSteer = leftSteer;
			}
			break;
		default:
			leftSteer = 0;
			break;
	}

	switch (cmd[CMD_RIGHTSTEER].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			ax0 = joyInfo->ax[cmd[CMD_RIGHTSTEER].val];
			if (ax0 > cmd[CMD_RIGHTSTEER].max) {
				ax0 = cmd[CMD_RIGHTSTEER].max;
			} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
				ax0 = cmd[CMD_RIGHTSTEER].min;
			}
			rightSteer = -SIGN(ax0) * cmd[CMD_RIGHTSTEER].pow * pow(fabs(ax0), cmd[CMD_RIGHTSTEER].sens) / (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x);
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_RIGHTSTEER].val] - cmd[CMD_RIGHTSTEER].deadZone;
			if (ax0 > cmd[CMD_RIGHTSTEER].max) {
				ax0 = cmd[CMD_RIGHTSTEER].max;
			} else if (ax0 < cmd[CMD_RIGHTSTEER].min) {
				ax0 = cmd[CMD_RIGHTSTEER].min;
			}
			ax0 = ax0 * cmd[CMD_RIGHTSTEER].pow;
			rightSteer = - pow(fabs(ax0), cmd[CMD_RIGHTSTEER].sens) / (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x / 10.0);
			break;
		case GFCTRL_TYPE_KEYBOARD:
		case GFCTRL_TYPE_SKEYBOARD:
		case GFCTRL_TYPE_JOY_BUT:
			if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_KEYBOARD) {
				ax0 = keyInfo[cmd[CMD_RIGHTSTEER].val].state;
			} else  if (cmd[CMD_RIGHTSTEER].type == GFCTRL_TYPE_SKEYBOARD) {
				ax0 = skeyInfo[cmd[CMD_RIGHTSTEER].val].state;
			} else {
				ax0 = joyInfo->levelup[cmd[CMD_RIGHTSTEER].val];
			}
			if (ax0 == 0) {
				HCtx[idx]->prevRightSteer = rightSteer = 0;
			} else {
				ax0 = 2 * ax0 - 1;
				rightSteer = HCtx[idx]->prevRightSteer - ax0 * cmd[CMD_RIGHTSTEER].sens * s->deltaTime/ (1.0 + cmd[CMD_RIGHTSTEER].spdSens * car->_speed_x / 10.0);
				if (rightSteer > 0.0) rightSteer = 0.0;
				if (rightSteer < -1.0) rightSteer = -1.0;
				HCtx[idx]->prevRightSteer = rightSteer;
			}
			break;
		default:
			rightSteer = 0;
			break;
	}

	car->_steerCmd = leftSteer + rightSteer;


	switch (cmd[CMD_BRAKE].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			brake = joyInfo->ax[cmd[CMD_BRAKE].val];
			if (brake > cmd[CMD_BRAKE].max) {
				brake = cmd[CMD_BRAKE].max;
			} else if (brake < cmd[CMD_BRAKE].min) {
				brake = cmd[CMD_BRAKE].min;
			}
			car->_brakeCmd = fabs(cmd[CMD_BRAKE].pow *
						pow(fabs((brake - cmd[CMD_BRAKE].minVal) /
							(cmd[CMD_BRAKE].max - cmd[CMD_BRAKE].min)),
						cmd[CMD_BRAKE].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_BRAKE].val] - cmd[CMD_BRAKE].deadZone;
			if (ax0 > cmd[CMD_BRAKE].max) {
				ax0 = cmd[CMD_BRAKE].max;
			} else if (ax0 < cmd[CMD_BRAKE].min) {
				ax0 = cmd[CMD_BRAKE].min;
			}
			ax0 = ax0 * cmd[CMD_BRAKE].pow;
			car->_brakeCmd =  pow(fabs(ax0), cmd[CMD_BRAKE].sens) / (1.0 + cmd[CMD_BRAKE].spdSens * car->_speed_x / 10.0);
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_brakeCmd = joyInfo->levelup[cmd[CMD_BRAKE].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_brakeCmd = mouseInfo->button[cmd[CMD_BRAKE].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_brakeCmd = keyInfo[cmd[CMD_BRAKE].val].state;
			break;
		case GFCTRL_TYPE_SKEYBOARD:
			car->_brakeCmd = skeyInfo[cmd[CMD_BRAKE].val].state;
			break;
		default:
			car->_brakeCmd = 0;
			break;
	}

	switch (cmd[CMD_CLUTCH].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			clutch = joyInfo->ax[cmd[CMD_CLUTCH].val];
			if (clutch > cmd[CMD_CLUTCH].max) {
				clutch = cmd[CMD_CLUTCH].max;
			} else if (clutch < cmd[CMD_CLUTCH].min) {
				clutch = cmd[CMD_CLUTCH].min;
			}
			car->_clutchCmd = fabs(cmd[CMD_CLUTCH].pow *
						pow(fabs((clutch - cmd[CMD_CLUTCH].minVal) /
							(cmd[CMD_CLUTCH].max - cmd[CMD_CLUTCH].min)),
						cmd[CMD_CLUTCH].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_CLUTCH].val] - cmd[CMD_CLUTCH].deadZone;
			if (ax0 > cmd[CMD_CLUTCH].max) {
				ax0 = cmd[CMD_CLUTCH].max;
			} else if (ax0 < cmd[CMD_CLUTCH].min) {
				ax0 = cmd[CMD_CLUTCH].min;
			}
			ax0 = ax0 * cmd[CMD_CLUTCH].pow;
			car->_clutchCmd =  pow(fabs(ax0), cmd[CMD_CLUTCH].sens) / (1.0 + cmd[CMD_CLUTCH].spdSens * car->_speed_x / 10.0);
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_clutchCmd = joyInfo->levelup[cmd[CMD_CLUTCH].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_clutchCmd = mouseInfo->button[cmd[CMD_CLUTCH].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_clutchCmd = keyInfo[cmd[CMD_CLUTCH].val].state;
			break;
		case GFCTRL_TYPE_SKEYBOARD:
			car->_clutchCmd = skeyInfo[cmd[CMD_CLUTCH].val].state;
			break;
		default:
			car->_clutchCmd = 0;
			break;
	}

	// if player's used the clutch manually then we dispense with autoClutch
	if (car->_clutchCmd != 0.0f)
		HCtx[idx]->autoClutch = 0;

	switch (cmd[CMD_THROTTLE].type) {
		case GFCTRL_TYPE_JOY_AXIS:
			throttle = joyInfo->ax[cmd[CMD_THROTTLE].val];
			if (throttle > cmd[CMD_THROTTLE].max) {
				throttle = cmd[CMD_THROTTLE].max;
			} else if (throttle < cmd[CMD_THROTTLE].min) {
				throttle = cmd[CMD_THROTTLE].min;
			}
			car->_accelCmd = fabs(cmd[CMD_THROTTLE].pow *
						pow(fabs((throttle - cmd[CMD_THROTTLE].minVal) /
								(cmd[CMD_THROTTLE].max - cmd[CMD_THROTTLE].min)),
							cmd[CMD_THROTTLE].sens));
			break;
		case GFCTRL_TYPE_MOUSE_AXIS:
			ax0 = mouseInfo->ax[cmd[CMD_THROTTLE].val] - cmd[CMD_THROTTLE].deadZone;
			if (ax0 > cmd[CMD_THROTTLE].max) {
				ax0 = cmd[CMD_THROTTLE].max;
			} else if (ax0 < cmd[CMD_THROTTLE].min) {
				ax0 = cmd[CMD_THROTTLE].min;
			}
			ax0 = ax0 * cmd[CMD_THROTTLE].pow;
			car->_accelCmd =  pow(fabs(ax0), cmd[CMD_THROTTLE].sens) / (1.0 + cmd[CMD_THROTTLE].spdSens * car->_speed_x / 10.0);
			if (isnan (car->_accelCmd)) {
				car->_accelCmd = 0;
			}
			/* printf("  axO:%f  accelCmd:%f\n", ax0, car->_accelCmd); */
			break;
		case GFCTRL_TYPE_JOY_BUT:
			car->_accelCmd = joyInfo->levelup[cmd[CMD_THROTTLE].val];
			break;
		case GFCTRL_TYPE_MOUSE_BUT:
			car->_accelCmd = mouseInfo->button[cmd[CMD_THROTTLE].val];
			break;
		case GFCTRL_TYPE_KEYBOARD:
			car->_accelCmd = keyInfo[cmd[CMD_THROTTLE].val].state;
			break;
		case GFCTRL_TYPE_SKEYBOARD:
			car->_accelCmd = skeyInfo[cmd[CMD_THROTTLE].val].state;
			break;
		default:
			car->_accelCmd = 0;
			break;
	}

	if (s->currentTime > 1.0) {
		// thanks Christos for the following: gradual accel/brake changes for on/off controls.
		const tdble inc_rate = 0.2f;
		
		if (cmd[CMD_BRAKE].type == GFCTRL_TYPE_JOY_BUT ||
		    cmd[CMD_BRAKE].type == GFCTRL_TYPE_MOUSE_BUT ||
		    cmd[CMD_BRAKE].type == GFCTRL_TYPE_KEYBOARD ||
		    cmd[CMD_BRAKE].type == GFCTRL_TYPE_SKEYBOARD)
		{
			tdble d_brake = car->_brakeCmd - HCtx[idx]->pbrake;
			if (fabs(d_brake) > inc_rate && car->_brakeCmd > HCtx[idx]->pbrake) {
				car->_brakeCmd = MIN(car->_brakeCmd, HCtx[idx]->pbrake + inc_rate*d_brake/fabs(d_brake));
			}
			HCtx[idx]->pbrake = car->_brakeCmd;
		}

		if (cmd[CMD_THROTTLE].type == GFCTRL_TYPE_JOY_BUT ||
			cmd[CMD_THROTTLE].type == GFCTRL_TYPE_MOUSE_BUT ||
			cmd[CMD_THROTTLE].type == GFCTRL_TYPE_KEYBOARD ||
			cmd[CMD_THROTTLE].type == GFCTRL_TYPE_SKEYBOARD)
		{
			tdble d_accel = car->_accelCmd - HCtx[idx]->paccel;
			if (fabs(d_accel) > inc_rate && car->_accelCmd > HCtx[idx]->paccel) {
				car->_accelCmd = MIN(car->_accelCmd, HCtx[idx]->paccel + inc_rate*d_accel/fabs(d_accel));
			}
			HCtx[idx]->paccel = car->_accelCmd;
		}
	}

	if (HCtx[idx]->AutoReverseEngaged) {
		/* swap brake and throttle */
		brake = car->_brakeCmd;
		car->_brakeCmd = car->_accelCmd;
		car->_accelCmd = brake;
	}

	if (HCtx[idx]->ParamAbs) 
	{
		if (fabs(car->_speed_x) > 10.0)
		{
			tdble rearskid = MAX(0.0, MAX(car->_skid[2], car->_skid[3]) - MAX(car->_skid[0], car->_skid[1]));
			int i;

			tdble skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
			NORM_PI_PI(skidAng);

			if (car->_speed_x > 5 && fabs(skidAng) > 0.2)
				car->_brakeCmd = MIN(car->_brakeCmd, 0.10 + 0.70 * cos(skidAng));

			if (fabs(car->_steerCmd) > 0.1)
			{
				tdble decel = ((fabs(car->_steerCmd)-0.1) * (1.0 + fabs(car->_steerCmd)) * 0.6);
				car->_brakeCmd = MIN(car->_brakeCmd, MAX(0.35, 1.0 - decel));
			}

			const tdble abs_slip = 2.5;
			const tdble abs_range = 5.0;

			slip = 0;
			for (i = 0; i < 4; i++) {
				slip += car->_wheelSpinVel(i) * car->_wheelRadius(i);
			}
			slip = car->_speed_x - slip/4.0f;

			if (slip > abs_slip)
				car->_brakeCmd = car->_brakeCmd - MIN(car->_brakeCmd*0.8, (slip - abs_slip) / abs_range);
		}
	}


	if (HCtx[idx]->ParamAsr) 
	{
    	tdble trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
		tdble angle = trackangle - car->_yaw;
		NORM_PI_PI(angle);

		tdble maxaccel = 0.0;
		if (car->_trkPos.seg->type == TR_STR)
			maxaccel = MIN(car->_accelCmd, 0.2);
		else if (car->_trkPos.seg->type == TR_LFT && angle < 0.0)
			maxaccel = MIN(car->_accelCmd, MIN(0.6, -angle));
		else if (car->_trkPos.seg->type == TR_RGT && angle > 0.0)
			maxaccel = MIN(car->_accelCmd, MIN(0.6, angle));

		tdble origaccel = car->_accelCmd;
		tdble skidAng = atan2(car->_speed_Y, car->_speed_X) - car->_yaw;
		NORM_PI_PI(skidAng);

		if (car->_speed_x > 5 && fabs(skidAng) > 0.2)
		{
			car->_accelCmd = MIN(car->_accelCmd, 0.15 + 0.70 * cos(skidAng));
			car->_accelCmd = MAX(car->_accelCmd, maxaccel);
		}

		if (fabs(car->_steerCmd) > 0.1)
		{
			tdble decel = ((fabs(car->_steerCmd)-0.1) * (1.0 + fabs(car->_steerCmd)) * 0.8);
			car->_accelCmd = MIN(car->_accelCmd, MAX(0.35, 1.0 - decel));
		}

		tdble drivespeed = 0.0;
		switch (HCtx[idx]->drivetrain)
		{
			case D4WD:
				drivespeed = ((car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
				              car->_wheelRadius(FRNT_LFT) +
				              (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
				              car->_wheelRadius(REAR_LFT)) / 4.0; 
				break;
			case DFWD:
				drivespeed = (car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
				              car->_wheelRadius(FRNT_LFT) / 2.0;
				break;
			default:
				drivespeed = (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
				              car->_wheelRadius(REAR_LFT) / 2.0;
				break;
		}

		tdble slip = drivespeed - fabs(car->_speed_x);
		if (slip > 2.0)
			car->_accelCmd = MIN(car->_accelCmd, origaccel - MIN(origaccel-0.1, ((slip - 2.0)/10.0)));
	}

	if (speedLimiter) {
		tdble Dv;
		if (Vtarget != 0) {
			Dv = Vtarget - car->_speed_x;
			if (Dv > 0.0) {
				car->_accelCmd = MIN(car->_accelCmd, fabs(Dv/6.0));
			} else {
				car->_brakeCmd = MAX(car->_brakeCmd, fabs(Dv/5.0));
				car->_accelCmd = 0;
			}
		}
	}


#ifndef WIN32
#ifdef TELEMETRY
	if ((car->_laps > 1) && (car->_laps < 5)) {
		if (HCtx[idx]->lap == 1) {
			RtTelemStartMonitoring("Player");
		}
		RtTelemUpdate(car->_curLapTime);
	}
	if (car->_laps == 5) {
		if (HCtx[idx]->lap == 4) {
			RtTelemShutdown();
		}
	}
#endif
#endif

	HCtx[idx]->lap = car->_laps;
}


static tdble getAutoClutch(int idx, int gear, int newgear, tCarElt *car)
{
	if (newgear != 0 && newgear < car->_gearNb) {
		if (newgear != gear) {
			HCtx[idx]->clutchtime = 0.332f - ((tdble) newgear / 65.0f);
		}

		if (HCtx[idx]->clutchtime > 0.0f)
			HCtx[idx]->clutchtime -= RCM_MAX_DT_ROBOTS;
		return 2.0f * HCtx[idx]->clutchtime;
	}
	
	return 0.0f;
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
static void drive_mt(int index, tCarElt* car, tSituation *s)
{
	int i;
	int idx = index - 1;
	tControlCmd	*cmd = HCtx[idx]->CmdControl;

	common_drive(index, car, s);
	car->_gearCmd = car->_gear;
	/* manual shift sequential */
	if (((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_UP_SHFT].val]) ||
		((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgeup[cmd[CMD_UP_SHFT].val]) ||
		((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_UP_SHFT].val].edgeUp) ||
		((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_UP_SHFT].val].edgeUp))
	{
		car->_gearCmd++;
	}

	if (((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_DN_SHFT].val]) ||
		((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgeup[cmd[CMD_DN_SHFT].val]) ||
		((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_DN_SHFT].val].edgeUp) ||
		((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_DN_SHFT].val].edgeUp))
	{
		if (HCtx[idx]->SeqShftAllowNeutral || (car->_gearCmd > 1)) {
			car->_gearCmd--;
		}
	}

	/* manual shift direct */
	if (HCtx[idx]->RelButNeutral) {
		for (i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
			if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgedn[cmd[i].val]) ||
				((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgedn[cmd[i].val]) ||
				((cmd[i].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[i].val].edgeDn) ||
				((cmd[i].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[i].val].edgeDn))
			{
				car->_gearCmd = 0;
			}
		}
	}

	for (i = CMD_GEAR_R; i <= CMD_GEAR_6; i++) {
		if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[i].val]) ||
			((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgeup[cmd[i].val]) ||
			((cmd[i].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[i].val].edgeUp) ||
			((cmd[i].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[i].val].edgeUp))
		{
			car->_gearCmd = i - CMD_GEAR_N;
		}
	}

	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
		car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);

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
static void drive_at(int index, tCarElt* car, tSituation *s)
{
	int gear, i;
	int idx = index - 1;
	tControlCmd	*cmd = HCtx[idx]->CmdControl;

	common_drive(index, car, s);

	/* shift */
	gear = car->_gear;

	if (gear > 0) {
		/* return to auto-shift */
		HCtx[idx]->manual = 0;
	}
	gear += car->_gearOffset;
	car->_gearCmd = car->_gear;

    if (!HCtx[idx]->AutoReverse) {
		/* manual shift */
		if (((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_UP_SHFT].val]) ||
			((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_UP_SHFT].val].edgeUp) ||
			((cmd[CMD_UP_SHFT].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_UP_SHFT].val].edgeUp))
		{
			car->_gearCmd++;
			HCtx[idx]->manual = 1;
		}

		if (((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[CMD_DN_SHFT].val]) ||
			((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[CMD_DN_SHFT].val].edgeUp) ||
			((cmd[CMD_DN_SHFT].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[CMD_DN_SHFT].val].edgeUp))
		{
			car->_gearCmd--;
			HCtx[idx]->manual = 1;
		}

		/* manual shift direct */
		if (HCtx[idx]->RelButNeutral) {
			for (i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
				if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgedn[cmd[i].val]) ||
					((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgedn[cmd[i].val]) ||
					((cmd[i].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[i].val].edgeDn) ||
					((cmd[i].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[i].val].edgeDn))
				{
					car->_gearCmd = 0;
					/* return to auto-shift */
					HCtx[idx]->manual = 0;
				}
			}
		}

		for (i = CMD_GEAR_R; i < CMD_GEAR_2; i++) {
			if (((cmd[i].type == GFCTRL_TYPE_JOY_BUT) && joyInfo->edgeup[cmd[i].val]) ||
				((cmd[i].type == GFCTRL_TYPE_MOUSE_BUT) && mouseInfo->edgeup[cmd[i].val]) ||
				((cmd[i].type == GFCTRL_TYPE_KEYBOARD) && keyInfo[cmd[i].val].edgeUp) ||
				((cmd[i].type == GFCTRL_TYPE_SKEYBOARD) && skeyInfo[cmd[i].val].edgeUp))
			{
				car->_gearCmd = i - CMD_GEAR_N;
				HCtx[idx]->manual = 1;
			}
		}
    }

	/* auto shift */
	if (!HCtx[idx]->manual && !HCtx[idx]->AutoReverseEngaged) {
		if (car->_speed_x > HCtx[idx]->shiftThld[gear]) {
			car->_gearCmd++;
		} else if ((car->_gearCmd > 1) && (car->_speed_x < (HCtx[idx]->shiftThld[gear-1] - 4.0))) {
			car->_gearCmd--;
		}
		if (car->_gearCmd <= 0) {
			car->_gearCmd++;
		}
	}

    if (HCtx[idx]->AutoReverse) {
		/* Automatic Reverse Gear Mode */
		if (!HCtx[idx]->AutoReverseEngaged) {
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x < 1.0)) {
				HCtx[idx]->AutoReverseEngaged = 1;
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		} else {
			/* currently in autoreverse mode */
			if ((car->_brakeCmd > car->_accelCmd) && (car->_speed_x > -1.0) && (car->_speed_x < 1.0)) {
				HCtx[idx]->AutoReverseEngaged = 0;
				car->_gearCmd = CMD_GEAR_1 - CMD_GEAR_N;
			} else {
				car->_gearCmd = CMD_GEAR_R - CMD_GEAR_N;
			}
		}
    }

	if (HCtx[idx]->autoClutch && car->_clutchCmd == 0.0f)
	    car->_clutchCmd = getAutoClutch(idx, car->_gear, car->_gearCmd, car);
}

static int pitcmd(int index, tCarElt* car, tSituation *s)
{
	tdble f1, f2;
	tdble ns;
	int idx = index - 1;

	HCtx[idx]->NbPitStops++;
	f1 = car->_tank - car->_fuel;
	if (HCtx[idx]->NbPitStopProg < HCtx[idx]->NbPitStops) {
		ns = 1.0;
	} else {
		ns = 1.0 + (HCtx[idx]->NbPitStopProg - HCtx[idx]->NbPitStops);
	}

	f2 = 0.00065 * (curTrack->length * car->_remainingLaps + car->_trkPos.seg->lgfromstart) / ns - car->_fuel;

	car->_pitFuel = MAX(MIN(f1, f2), 0);

	HCtx[idx]->LastPitStopLap = car->_laps;

	car->_pitRepair = (int)car->_dammage;

	int i;
	int key;
	tControlCmd *cmd;

	if (HCtx[idx]) {
		cmd = HCtx[idx]->CmdControl;
		for (i = 0; i < nbCmdControl; i++) {
			if (cmd[i].type == GFCTRL_TYPE_KEYBOARD || cmd[i].type == GFCTRL_TYPE_SKEYBOARD) {
				key = cmd[i].val;
				keyInfo[key].state = GFUI_KEY_UP;
				keyInfo[key].edgeDn = 0;
				keyInfo[key].edgeUp = 0;
				skeyInfo[key].state = GFUI_KEY_UP;
				skeyInfo[key].edgeDn = 0;
				skeyInfo[key].edgeUp = 0;
				currentKey[key] = GFUI_KEY_UP;
				currentSKey[key] = GFUI_KEY_UP;
			}
		}
	}

	return ROB_PIT_MENU; /* The player is able to modify the value by menu */
}

