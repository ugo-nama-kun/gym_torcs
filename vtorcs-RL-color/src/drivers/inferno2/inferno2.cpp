/***************************************************************************

    file                 : inferno.cpp
    created              : Tue Mar  7 22:09:50 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: inferno2.cpp,v 1.11.2.2 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/



#ifdef _WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <tgf.h>

#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>

#include "common.h"

static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s);
static void drive(int index, tCarElt* car, tSituation *s);
static void newrace(int index, tCarElt* car, tSituation *s);

tTrack		*DmTrack;
static char	ParamNames[256];

#ifdef _WIN32
/* should be present in mswindows */
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
	return TRUE;
}
#endif

static int
pitCmd(int index, tCarElt *car, tSituation *s)
{
	int 	idx = index - 1;
	int		remainLaps = s->_totLaps - car->_laps - car->_lapsBehindLeader + 1;
	tdble	remainDist = remainLaps * DmTrack->length;
	tdble	fuel;
	
	PitState[idx] = PIT_STATE_PIT_EXIT;
	fuel = ConsFactor[idx] * (remainLaps + 1);
	
	if (fuel > car->_tank) fuel = car->_tank;
	
	fuel -=  car->_fuel;
	
	if (fuel < 0) fuel = 0;
	
	car->_pitFuel = fuel;
	remainDist = remainLaps * DmTrack->length;

	if (remainDist > 100) {
		car->_pitRepair = (int)(car->_dammage);
	} else if (remainDist > 60) {
		car->_pitRepair = (int)(car->_dammage / 1.5);
	} else {
		car->_pitRepair = (int)(car->_dammage / 2.0);
	}
	
	return ROB_PIT_IM;
}

/* #define RELAXATION(target, prev, gain) {(target) = (prev) + gain * ((target) - (prev)) * s->deltaTime; (prev) = (target);} */

static void
shutdn(int index)
{
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
	
	itf->rbNewTrack = initTrack;			/* give the robot the track view called */
	/* for every track change or new race */
	itf->rbNewRace  = newrace;
	itf->rbDrive    = drive;			/* drive during race */
	itf->index      = index;
	itf->rbPitCmd   = pitCmd;
	itf->rbShutdown = shutdn;
	return 0;
}


static const char* botname[10] = {"InfHist 1", "InfHist 2", "InfHist 3", "InfHist 4", "InfHist 5",
							"InfHist 6", "InfHist 7", "InfHist 8", "InfHist 9", "InfHist 10"};
static const char* botdesc[10] = {"For Laurence", "For Laurence", "For Laurence", "For Laurence", "For Laurence",
							"For Laurence", "For Laurence", "For Laurence", "For Laurence", "For Laurence"};


/*
 * Function
 *	inferno
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
extern "C" int
inferno2(tModInfo *modInfo)
{
	int		i;
	//char	buf[256];
	
	for (i = 0; i < 10; i++) {
		//sprintf(buf, "InfHist %d", i + 1);
		modInfo[i].name    = strdup(botname[i]);	/* name of the module (short) */
		modInfo[i].desc    = strdup(botdesc[i]);	/* description of the module (can be long) */
		modInfo[i].fctInit = InitFuncPt;			/* init function */
		modInfo[i].gfId    = ROB_IDENT;				/* supported framework version */
		modInfo[i].index   = i + 1;
	}
	return 0;
}


tdble	Tright[10];
tdble	MaxSpeed[10];
tdble	hold[10] = {0};
tdble	shiftThld[10][MAX_GEARS+1];

static tdble PGain[10]     = {0.015f};
static tdble AGain[10]     = {0.008f};
static tdble PnGain[10]    = {0.02f};
static tdble Advance[10]   = {3.5f};
static tdble Advance2[10]  = {10.0f};
static tdble AdvStep[10]   = {1.0f};
static tdble VGain[10]     = {0.0005f};
static tdble preDy[10]     = {0.0f};
static tdble spdtgt[10]    = {250.0f};
static tdble spdtgt2[10]   = {2.0f};
static tdble steerMult[10] = {2.0f};
static tdble Offset[10]    = {0.0f};
static tdble Trightprev[10];
tdble DynOffset[10] = {0.0};
int   PitState[10]  = {0};

tdble O1[10] = {60.0};
tdble O2[10] = {60.0};
tdble O3[10] = {0.0};
tdble O4[10] = {0.0};
tdble O5[10] = {20.0};
tdble OP[10] = {15.0};
tdble OA[10] = {0.0};
tdble OW[10] = {2.0};
tdble VM;
tdble VM1[10] = {15.0};
tdble VM2[10] = {0.0};
tdble VM3[10] = {25.0};

tdble OffsetApproach[10] = {0.0};
tdble OffsetFinal[10]    = {0.0};
tdble OffsetExit[10]     = {0.0};
tdble LgfsFinal[10];
tdble ConsFactor[10]     = {0.0007f};

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

#define SIMU_PRMS	"Simulation Parameters"
#define PGAIN		"PGain"
#define AGAIN		"AGain"
#define PNGAIN		"PnGain"
#define ADVANCE		"Advance"
#define ADVANCE2	"Advance2"
#define ADVSTEP		"AdvStep"
#define VGAIN		"VGain"
#define PREDY		"preDy"
#define SPDTGT		"spdtgt"
#define SPDTGT2		"spdtgt2"
#define STEERMULT	"steerMult"
#define OFFSET		"offset"
#define OFFSETAPPROACH	"offsetApproach"
#define OFFSETFINAL	"offsetFinal"
#define OFFSETEXIT	"offsetExit"
#define PITOFFSET1	"len before pit entry"
#define PITOFFSET2	"len before pit start"
#define PITOFFSETP	"len around pit stop"
#define PITOFFSETA	"len to align around pit stop"
#define PITOFFSET3	"len after pit end"
#define PITOFFSET4	"len after pit exit"
#define PITOFFSET5	"len to speed down for pitting"
#define VMAX1		"VMax1"
#define VMAX2		"VMax2"
#define VMAX3		"VMax3"

tdble Gmax;


static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s)
{
	int 	idx = index - 1;
	void	*hdle;
	char	*str;
	char	buf[256];
	tdble	fuel;
	tdble	tmpMu;
			
			
	DmTrack = track;
	str = strrchr(track->filename, '/') + 1;
	sprintf(ParamNames, "drivers/inferno2/%d/tracksdata/car_%s", index, str);
	*carParmHandle = GfParmReadFile(ParamNames, GFPARM_RMODE_REREAD);
	
	if (*carParmHandle == NULL) {
		sprintf(ParamNames, "drivers/inferno2/%d/defaultcar.xml", index);
		*carParmHandle = GfParmReadFile(ParamNames, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
		GfOut("%s Loaded\n", ParamNames);
	} else {
		GfOut("%s Loaded\n", ParamNames);
	}
	
	ConsFactor[idx] = 0.0007 * DmTrack->length;
	fuel = ConsFactor[idx] * (s->_totLaps + 1);
			
	GfParmSetNum(*carParmHandle, SECT_CAR, PRM_FUEL, (char*)NULL, fuel);
		
	VM = track->pits.speedLimit;
		
	Gmax = GfParmGetNum(*carParmHandle, SECT_FRNTRGTWHEEL, PRM_MU, (char*)NULL, 1.0);
	tmpMu = GfParmGetNum(*carParmHandle, SECT_FRNTLFTWHEEL, PRM_MU, (char*)NULL, 1.0);
	Gmax = MIN(Gmax, tmpMu);
	tmpMu = GfParmGetNum(*carParmHandle, SECT_REARRGTWHEEL, PRM_MU, (char*)NULL, 1.0);
	Gmax = MIN(Gmax, tmpMu);
	tmpMu = GfParmGetNum(*carParmHandle, SECT_REARLFTWHEEL, PRM_MU, (char*)NULL, 1.0);
	Gmax = MIN(Gmax, tmpMu);
	/*     Gmax = Gmax * GfParmGetNum(*carParmHandle, SECT_CAR, PRM_MASS, (char*)NULL, 1000.0); */
		
	sprintf(buf, "drivers/inferno2/%d/tracksdata/%s", index, str);
	hdle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
	if (!hdle) {
		sprintf(buf, "drivers/inferno2/%d/default.xml", index);
		hdle = GfParmReadFile(buf, GFPARM_RMODE_REREAD);
	}
	
	if (hdle) {
		GfOut("%s Loaded\n", buf);
		
		PGain[idx]     = GfParmGetNum(hdle, SIMU_PRMS, PGAIN,     NULL, PGain[0]);
		AGain[idx]     = GfParmGetNum(hdle, SIMU_PRMS, AGAIN,     NULL, AGain[0]);
		PnGain[idx]    = GfParmGetNum(hdle, SIMU_PRMS, PNGAIN,    NULL, PnGain[0]);
		Advance[idx]   = GfParmGetNum(hdle, SIMU_PRMS, ADVANCE,   NULL, Advance[0]);
		Advance2[idx]  = GfParmGetNum(hdle, SIMU_PRMS, ADVANCE2,  NULL, Advance2[0]);
		AdvStep[idx]   = GfParmGetNum(hdle, SIMU_PRMS, ADVSTEP,   NULL, AdvStep[0]);
		VGain[idx]     = GfParmGetNum(hdle, SIMU_PRMS, VGAIN,     NULL, VGain[0]);
		preDy[idx]     = GfParmGetNum(hdle, SIMU_PRMS, PREDY,     NULL, preDy[0]);
		spdtgt[idx]    = GfParmGetNum(hdle, SIMU_PRMS, SPDTGT,    NULL, spdtgt[0]);
		spdtgt2[idx]   = GfParmGetNum(hdle, SIMU_PRMS, SPDTGT2,   NULL, spdtgt2[0]);
		steerMult[idx] = GfParmGetNum(hdle, SIMU_PRMS, STEERMULT, NULL, steerMult[0]);
		Offset[idx]    = GfParmGetNum(hdle, SIMU_PRMS, OFFSET,    NULL, Offset[0]);
		
		OffsetApproach[idx] = GfParmGetNum(hdle, SIMU_PRMS, OFFSETAPPROACH, NULL, OffsetApproach[0]);
		OffsetFinal[idx]    = GfParmGetNum(hdle, SIMU_PRMS, OFFSETFINAL,    NULL, OffsetFinal[0]);
		OffsetExit[idx]     = GfParmGetNum(hdle, SIMU_PRMS, OFFSETEXIT,     NULL, OffsetExit[0]);
		O1[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSET1,     NULL, O1[0]);
		O2[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSET2,     NULL, O2[0]);
		OP[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSETP,     NULL, OP[0]);
		OA[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSETA,     NULL, OA[0]);
		O3[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSET3,     NULL, O3[0]);
		O4[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSET4,     NULL, O4[0]);
		O5[idx]          = GfParmGetNum(hdle, SIMU_PRMS, PITOFFSET5,     NULL, O5[0]);
		VM1[idx]         = GfParmGetNum(hdle, SIMU_PRMS, VMAX1,          NULL, VM1[0]);
		VM2[idx]         = GfParmGetNum(hdle, SIMU_PRMS, VMAX2,          NULL, VM2[0]);
		VM3[idx]         = GfParmGetNum(hdle, SIMU_PRMS, VMAX3,          NULL, VM3[0]);
		GfParmReleaseHandle(hdle);
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
static int	curidx;
static tdble	Gear;
static tdble	TargetSpeed;
static tdble	InvBrkCmd;

void newrace(int index, tCarElt* car, tSituation *s)
{
    int 	idx = index - 1;
    
    Tright[idx] = Trightprev[idx] = car->_trkPos.toRight;
    hold[idx] = 8.0;
    curidx = 0;

    InitGears(car, idx);

    if ((car->_pit) && (OffsetFinal[idx] != 0.0)) {
	LgfsFinal[idx] = RtGetDistFromStart2(&car->_pit->pos);
    } else {
	PitState[idx] = PIT_STATE_NO;
    }

#ifndef WIN32
    if (s->_raceType == RM_TYPE_PRACTICE) {
	RtTelemInit(-10, 10);
	RtTelemNewChannel("Ax", &car->_accel_x, -30, 30);
	RtTelemNewChannel("Ay", &car->_accel_y, -30, 30);
	RtTelemNewChannel("Vaz", &car->_yaw_rate, -10, 10);
	RtTelemNewChannel("Steer", &car->_steerCmd, -1, 1);
	RtTelemNewChannel("Throttle", &car->_accelCmd, -1, 1);
	RtTelemNewChannel("Brake", &InvBrkCmd, -1, 1);
	RtTelemNewChannel("Gear", &Gear, -10, 10);
	RtTelemNewChannel("Speed", &car->_speed_x, -100, 100);
	RtTelemNewChannel("Target Speed", &TargetSpeed, -100, 100);
    }
#endif
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
static void drive(int index, tCarElt* car, tSituation *s)
{
    int 		idx = index - 1;
    tdble 		Dy, Dny;
    tdble 		Vy;
    tTrkLocPos		trkPos, trkPos2;
    tdble 		X, Y, x, y, CosA, SinA;
    tTrackSeg		*seg;
    tdble		Da, Db;
    tdble		tgtSpeed = -1.0;
    tdble		lgfs;
    tdble		vtgt1, vtgt2;
    tdble		curAdv, curAdvMax, Amax, Atmp, AdvMax;

    static int		lap[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    static tdble	lgfsprev[10] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    static tdble	adv[10];
    
    
    Gear = (tdble)car->_gear;
    
    memset(&car->ctrl, 0, sizeof(tCarCtrl));

    MaxSpeed[idx] = 10000.0;
    trkPos = car->_trkPos;
    X = car->_pos_X;
    Y = car->_pos_Y;
    seg = trkPos.seg;
    CosA = cos(car->_yaw);
    SinA = sin(car->_yaw);
    lgfs = GetDistToStart(car) + fabs(preDy[idx]);
    if (lgfs < DmTrack->seg->next->length) {
	curidx = 0;
	if (lgfsprev[idx] > lgfs) {
	    lgfsprev[idx] = 0;
	}
    }

    adv[idx] = Advance[idx] + 5.0 * sqrt(fabs(car->_speed_x));
    
    if (s->currentTime > hold[idx]) {
	    Tright[idx] = seg->width / 2.0 + Offset[idx] + DynOffset[idx];
    }

    
    vtgt1 = spdtgt[idx];
    vtgt2 = spdtgt2[idx];

    x = X + (CosA) * adv[idx];
    y = Y + (SinA) * adv[idx];
    RtTrackGlobal2Local(trkPos.seg, x, y, &trkPos2, TR_LPOS_MAIN);
    Dny = seg->width / 2.0 - trkPos2.toRight + Offset[idx] + DynOffset[idx];

    CollDet(car, idx, s, s->currentTime, Dny);
    
    RELAXATION(Tright[idx], Trightprev[idx], 2.0);

    /* proportionnal */
    Dy = Tright[idx] - trkPos.toRight;

    /* derivation */
    Vy = (Dy - preDy[idx]) / s->deltaTime;
    preDy[idx] = Dy;

    Da = RtTrackSideTgAngleL(&trkPos) - car->_yaw;
    NORM_PI_PI(Da);
    

    car->_steerCmd = PGain[idx] * Dy + VGain[idx] * Vy + PnGain[idx] * Dny + AGain[idx] * Da * Da;

    if (car->_speed_x < 0) {
	car->_steerCmd *= 1.5;
    } else if (car->_speed_x < 10) {
	car->_steerCmd *= 2.0;
    }

    /*
     * speed control
     */
    CosA = cos(car->_yaw + car->_steerCmd*2.0);
    SinA = sin(car->_yaw + car->_steerCmd*2.0);
    curAdv = Advance2[idx];
    AdvMax = fabs(car->_speed_x) * 5.0 + 1.0;
    Amax = 0;
    while (curAdv < AdvMax) {
	x = X + CosA * curAdv;
	y = Y + SinA * curAdv;
	RtTrackGlobal2Local(seg, x, y, &trkPos, TR_LPOS_MAIN);
	Atmp = fabs(trkPos.toRight - car->_trkPos.toRight) / curAdv;
	if (Amax < Atmp) {
	    Amax = Atmp;
	    curAdvMax = curAdv;
	}
	curAdv += AdvStep[idx];
    }

    Db = car->_yaw_rate;
    Amax = 1.0 - Amax;
    Amax = Amax * Amax;
    if (tgtSpeed < 0) {
	tgtSpeed = (vtgt1 * Amax  + vtgt2) *
	    (1.0 + tan(fabs(car->_trkPos.seg->angle[TR_XE] + car->_trkPos.seg->angle[TR_XS])));
	tgtSpeed -= (car->_dammage / s->_maxDammage) * 0.2;
	tgtSpeed = MIN(tgtSpeed, MaxSpeed[idx] / 1.15);
    }
    TargetSpeed = tgtSpeed * 1.15;
    SpeedStrategy(car, idx, TargetSpeed, s, Db);


#define AMARG 0.6
    if ((((Da > (PI/2.0-AMARG)) && (car->_trkPos.toRight < seg->width/3.0)) ||
	 ((Da < (AMARG-PI/2.0)) && (car->_trkPos.toRight > (seg->width - seg->width/3.0)))) && 
	(car->_gear < 2) && (car->_speed_x < 1.0)) {
	car->_steerCmd = -car->_steerCmd * 3.0;
	car->_gearCmd = -1;
    } else if ((fabs(Da) > (PI - (PI/4.0))) &&
	       ((car->_trkPos.toRight < 0) ||
		(car->_trkPos.toRight > seg->width))) {
	car->_steerCmd = -car->_steerCmd * 3.0;
    }
    if ((car->_speed_x < -0.5) && (car->_gear > 0)) {
	car->_brakeCmd = 1.0;
    }

    if ((PitState[idx] > PIT_STATE_DECEL) && (PitState[idx] < PIT_STATE_EXIT) && (car->_speed_x < 15.0)) {
	car->_steerCmd *= 5.0;
    }

#if 0
    if (car->_laps == 2) {
	if (s->_raceType == RM_TYPE_PRACTICE) {
	    if (lap[idx] == 1) {
		RtTelemStartMonitoring("Inferno2");
	    }
	    RtTelemUpdate(car->_curLapTime);
	}
    }
    if (car->_laps == 3) {
	if (s->_raceType == RM_TYPE_PRACTICE) {
	    if (lap[idx] == 2) {
		RtTelemShutdown();
	    }
	}
    }
#endif
    lap[idx] = car->_laps;

    InvBrkCmd = - car->_brakeCmd;
}

