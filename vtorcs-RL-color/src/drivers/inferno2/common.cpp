/***************************************************************************

    file                 : common.cpp
    created              : Sun Oct 24 14:16:27 CEST 1999
    copyright            : (C) 1999 by Eric Espie & Christophe Guionneau
    email                : torcs@free.fr
    version              : $Id: common.cpp,v 1.9.2.1 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

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

static tdble VI[10];

static tdble
Spline(tdble p0, tdble p1, tdble pos, tdble start, tdble end)
{
    tdble t2, t3;
    tdble h0, h1;
    tdble t;

    if (start < 0) {
	start += DmTrack->length;
    }
    if (start > DmTrack->length) {
	start -= DmTrack->length;
    }
    if (end < 0) {
	end += DmTrack->length;
    }
    if (end > DmTrack->length) {
	end -= DmTrack->length;
    }
    if (start > end) {
	end += DmTrack->length;
	if (pos < start) {
	    pos += DmTrack->length;
	}
    }
    t = (pos - start) / (end - start);
    
    t2 = t * t;
    t3 = t * t2;
    h1 = 3 * t2 - 2 * t3;
    h0 = 1 - h1;
    
    return h0 * p0 + h1 * p1;
}

void
InitGears(tCarElt* car, int idx)
{
    int i, j;
    tdble rpm;

    GfOut("Car %d\n", idx + 1);
    for (i = 0; i < MAX_GEARS; i++) {
	j = i + 1;
	if (j < MAX_GEARS) {
	    if ((car->_gearRatio[j] != 0) && (car->_gearRatio[i] != 0)) {
		rpm = car->_enginerpmRedLine;
		shiftThld[idx][i] = rpm * .95  * car->_wheelRadius(2) / car->_gearRatio[i];
		/* GfOut("   Gear %d: shift %d km/h\n", i, (int)(shiftThld[idx][i] * 3.6)); */
	    } else {
		shiftThld[idx][i] = 10000.0;
	    }
	} else {
	    shiftThld[idx][i] = 10000.0;
	}
    }
}

static tdble lastAccel[10] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
static tdble lastBrkCmd[10] = {0};
static tdble lastDv[10] = {0};

void
SpeedStrategy(tCarElt* car, int idx, tdble Vtarget, tSituation *s, tdble aspect)
{
    const tdble Dx  = 0.02f;
    const tdble Dxx = 0.01f;
    const tdble Dxb  = 0.05f;
    const tdble Dxxb = 0.01f;
    tdble	Dv;
    tdble	Dvv;
    tdble 	slip;
    int 	gear;

    gear = car->_gear;
    Dv = Vtarget - car->_speed_x;
    Dvv = Dv - lastDv[idx];
    lastDv[idx] = Dv;

    //RELAXATION(Vtarget, lastTarget[idx], 2.0);
    
    if (Dv > 0.0) {
	/* speed management */
	car->_accelCmd = MIN(Dv * Dx + Dvv * Dxx, 1.0);
	car->_accelCmd = 1.0;
	
	/* anti-slip */
	/* assume SPOOL differential and rwd */
	if (car->_speed_x > 0) {
	    slip = (car->_wheelRadius(3) * car->_wheelSpinVel(3) - car->_speed_x) / car->_speed_x;
	} else {
	    slip = 0;
	}
	if (gear == 1) {
	    car->_accelCmd = car->_accelCmd * exp(-fabs(car->_steerCmd) * 0.1) * exp(-fabs(aspect) * 5.0) + .1;
	} else if ((gear > 1) && (car->_speed_x < 40.0)) {
	    car->_accelCmd = car->_accelCmd * exp(-fabs(aspect) * 4.0) + 0.15;
	}
	
	
	if ((slip > 1.0) && (gear > 1)) {
	    car->_accelCmd *= 0.5;
	} else {
	    RELAXATION(car->_accelCmd, lastAccel[idx], 50.0);
	}
	car->_accelCmd = MIN(car->_accelCmd, fabs(Dv/6.0));
	//lastBrkCmd[idx] = 0.8;
    } else {
	tdble meanSpd = 0;
	int i;

	slip = 0;
	for (i = 0; i < 4; i++) {
	    meanSpd += car->_wheelSpinVel(i);
	}
	meanSpd /= 4.0;

	if (meanSpd > 15.0) {
	    for (i = 0; i < 4; i++) {
		if (((meanSpd - car->_wheelSpinVel(i)) / meanSpd) < -0.1) {
		    slip = 1.0;
		}
	    }
	}
	car->_brakeCmd = MIN(-Dv * Dxb + Dvv * Dxxb, 1.0);
	if (slip > 0.3) {
	    tdble maxslp = exp(-3.47*(slip - 0.2));
	    car->_brakeCmd = MIN(car->_brakeCmd, maxslp);
	} else {
	    RELAXATION(car->_brakeCmd, lastBrkCmd[idx], 50.0);
	}
	car->_brakeCmd = MIN(car->_brakeCmd, fabs(Dv/5.0));
	//lastAccel[idx] = 1.0;
    }

    /* shift */
    gear += car->_gearOffset;
    car->_gearCmd = car->_gear;
    if (car->_speed_x > shiftThld[idx][gear]) {
	car->_gearCmd++;
    } else if ((car->_gearCmd > 1) && (car->_speed_x < (shiftThld[idx][gear-1] - 10.0))) {
	car->_gearCmd--;
    }
    if (car->_gearCmd <= 0) {
	car->_gearCmd++;
    }
}

tdble
GetDistToStart(tCarElt *car)
{
    tTrackSeg	*seg;
    tdble	lg;
    
    seg = car->_trkPos.seg;
    lg = seg->lgfromstart;
    
    switch (seg->type) {
    case TR_STR:
	lg += car->_trkPos.toStart;
	break;
    default:
	lg += car->_trkPos.toStart * seg->radius;
	break;
    }
    return lg;
}

static int
isBetween(tdble lgfs, tdble start, tdble end)
{
    if (start < 0) {
	start += DmTrack->length;
    }
    if (start > DmTrack->length) {
	start -= DmTrack->length;
    }
    if (end < 0) {
	end += DmTrack->length;
    }
    if (end > DmTrack->length) {
	end -= DmTrack->length;
    }
    if (((lgfs > start) && (lgfs < end)) ||
	((start > end) &&
	 ((lgfs > start) || (lgfs < end)))) {
	return 1;
    }
    return 0;
}

static tdble
getOffset(int idx, tCarElt* car, tdble *maxSpeed)
{
    tdble	offset = 0;
    tdble	lgfs = 0;
    static tTrackPitInfo *pits = &DmTrack->pits;;
    static tdble	 Entry = pits->pitEntry->lgfromstart;
    static tdble	 Start = pits->pitStart->lgfromstart;
    static tdble	 End   = pits->pitEnd->lgfromstart;
    static tdble	 Exit  = pits->pitExit->lgfromstart;
    

    switch (PitState[idx]) {
    case PIT_STATE_NONE:
	break;
	
    case PIT_STATE_ASKED:
    case PIT_STATE_ENTERED:
	lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, Entry - O1[idx], Start - O2[idx])) {
	    //GfOut("PIT_STATE_ENTERED\n");
	    offset = Spline(0, OffsetApproach[idx], lgfs, Entry - O1[idx], Start - O2[idx]);
	    if (PitState[idx] == PIT_STATE_ASKED) {
		VI[idx] = car->_speed_x;
	    }
	    *maxSpeed = Spline(VI[idx], VM, lgfs, Entry - O1[idx], Start - O2[idx]);
	    hold[idx] = 0;
	    PitState[idx] = PIT_STATE_ENTERED;
	    break;
	}
	if (PitState[idx] == PIT_STATE_ASKED) {
	    break;
	}
	
	/* FALL THROUGH */
    case PIT_STATE_DECEL:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, Start - O2[idx], LgfsFinal[idx] - OP[idx] - O5[idx])) {
	    //GfOut("PIT_STATE_DECEL\n");
	    PitState[idx] = PIT_STATE_DECEL;
	    offset = OffsetApproach[idx];
	    *maxSpeed = VM;
	    hold[idx] = 0;
	    break;
	}
	
	/* FALL THROUGH */
    case PIT_STATE_PITLANE_BEFORE:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, LgfsFinal[idx] - OP[idx] - O5[idx], LgfsFinal[idx] - OP[idx])) {
	    //GfOut("PIT_STATE_PITLANE_BEFORE\n");
	    PitState[idx] = PIT_STATE_PITLANE_BEFORE;
	    offset = OffsetApproach[idx];
	    VM1[idx] = MIN(VM1[idx], VM);
	    *maxSpeed = Spline(VM, VM1[idx], lgfs, LgfsFinal[idx] - OP[idx] - O5[idx], LgfsFinal[idx] - OP[idx]);
	    hold[idx] = 0;
	    break;
	}

	/* FALL THROUGH */
    case PIT_STATE_PIT_ENTRY:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;
	
	if (isBetween(lgfs, LgfsFinal[idx] - OP[idx], LgfsFinal[idx] - OA[idx])) {
	    //GfOut("PIT_STATE_PIT_ENTRY\n");
	    PitState[idx] = PIT_STATE_PIT_ENTRY;
	    //offset = Spline(OffsetApproach, OffsetFinal, lgfs, LgfsFinal[idx] - OP[idx], LgfsFinal[idx]);
	    offset = OffsetFinal[idx] + SIGN(OffsetFinal[idx]) * OW[idx];
	    *maxSpeed = Spline(VM2[idx], 0, lgfs, LgfsFinal[idx] - OP[idx], LgfsFinal[idx]);
	    hold[idx] = 0;
	    car->_raceCmd = RM_CMD_PIT_ASKED;
	    break;
	}

	/* FALL THROUGH */
    case PIT_STATE_PIT_ALIGN:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;
	
	if (isBetween(lgfs, LgfsFinal[idx] - OA[idx], LgfsFinal[idx])) {
	    //GfOut("PIT_STATE_PIT_ALIGN\n");
	    PitState[idx] = PIT_STATE_PIT_ALIGN;
	    offset = OffsetApproach[idx];
	    *maxSpeed = Spline(VM2[idx], 0, lgfs, LgfsFinal[idx] - OP[idx], LgfsFinal[idx]);
	    hold[idx] = 0;
	    car->_raceCmd = RM_CMD_PIT_ASKED;
	    break;
	}

	/* FALL THROUGH */
    case PIT_STATE_PIT_EXIT:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, LgfsFinal[idx] - OP[idx], LgfsFinal[idx] + OP[idx])) {
	    //GfOut("PIT_STATE_PIT_EXIT\n");
	    PitState[idx] = PIT_STATE_PIT_EXIT;
	    //offset = Spline(OffsetFinal, OffsetApproach, lgfs, LgfsFinal[idx], LgfsFinal[idx] + OP[idx]);
	    offset = OffsetExit[idx];
	    VM3[idx] = MIN(VM3[idx], VM);
	    *maxSpeed = Spline(VM3[idx], VM, lgfs, LgfsFinal[idx], LgfsFinal[idx] + OP[idx]);
	    hold[idx] = 0;
	    break;
	}

	/* FALL THROUGH */
    case PIT_STATE_PITLANE_AFTER:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, LgfsFinal[idx] + OP[idx], End + O3[idx])) {
	    //GfOut("PIT_STATE_PITLANE_AFTER %f - %f - %f\n", lgfs, LgfsFinal[idx] + OP[idx], End + O3[idx]);
	    PitState[idx] = PIT_STATE_PITLANE_AFTER;
	    offset = OffsetExit[idx];
	    *maxSpeed = VM;
	    hold[idx] = 0;
	    break;
	}

	/* FALL THROUGH */
    case PIT_STATE_EXIT:
	if (!lgfs) lgfs = GetDistToStart(car);
	car->_lightCmd |= RM_LIGHT_HEAD2;

	if (isBetween(lgfs, End + O3[idx], Exit + O4[idx])) {
	    //GfOut("PIT_STATE_EXIT\n");
	    PitState[idx] = PIT_STATE_EXIT;
	    offset = Spline(OffsetExit[idx], 0, lgfs, End + O3[idx], Exit + O4[idx]);
	    hold[idx] = 0;
	    break;
	}
	//GfOut("End of Pit\n");
	PitState[idx] = PIT_STATE_NONE;
	break;
    }

    return offset;
}


void
CollDet(tCarElt* car, int idx, tSituation *s, tdble Curtime, tdble dny)
{
    int		i;
    tCarElt	*otherCar;
    tdble	lgfs, lgfs2, dlg;
    tdble	dspd;
    tdble	maxdlg;
    tTrackSeg	*seg;
    int		canOverlap = 1;
    const tdble MARGIN = 8.0;

    maxdlg = 200.0;
    seg = car->_trkPos.seg;
    lgfs = GetDistToStart(car);

    DynOffset[idx] = 0;
    /* Automatic pit every lap (test) */
#if 0
        if ((PitState[idx] == PIT_STATE_NONE) && (car->_laps)) {
     	PitState[idx] = PIT_STATE_ASKED;
    }
#endif

    if ((PitState[idx] == PIT_STATE_NONE) && ((s->_raceState & RM_RACE_FINISHING) == 0) && 
	(((car->_dammage > 5000) && ((s->_totLaps - car->_laps) > 2)) || 
	 ((car->_fuel < ConsFactor[idx]) && ((s->_totLaps - car->_laps) > 1)))) {
	PitState[idx] = PIT_STATE_ASKED;
    }
    if (PitState[idx] != PIT_STATE_NO) {
	DynOffset[idx] = getOffset(idx, car, &MaxSpeed[idx]);
	if ((PitState[idx] != PIT_STATE_NONE) && (PitState[idx] != PIT_STATE_ASKED)) {
	    canOverlap = 0;
	}
    }

    for (i = 0; i < s->_ncars; i++) {
	otherCar = s->cars[i];
	if ((otherCar == car) || (otherCar->_state & RM_CAR_STATE_NO_SIMU)) {
	    continue;
	}
	lgfs2 = GetDistToStart(otherCar);
	dlg = lgfs2 - lgfs;
	if (dlg > (DmTrack->length / 2.0)) dlg -= DmTrack->length;
	if (dlg < -(DmTrack->length / 2.0)) dlg += DmTrack->length;

	dspd = car->_speed_x - otherCar->_speed_x;
	if ((car->_laps < otherCar->_laps) && 
	    (dlg > -maxdlg) && (dlg < (car->_dimension_x + 1.0)) &&
	    (dlg > (dspd * 6.0))) {
	    if ((fabs(car->_trkPos.toRight - otherCar->_trkPos.toRight) < (MARGIN / 2.0)) &&
		(otherCar->_speed_x > car->_speed_x)) {
		maxdlg = fabs(dlg);
		hold[idx] = Curtime + 1.0;
		if (car->_trkPos.toRight < otherCar->_trkPos.toRight) {
		    Tright[idx] = otherCar->_trkPos.toRight - (MARGIN * 3.0);
		} else {
		    Tright[idx] = otherCar->_trkPos.toRight + (MARGIN * 3.0);
		}
	    }
	} else	if (((dlg < maxdlg) && (dlg > -(car->_dimension_x + 1.0))) &&
		    ((dlg < (dspd*4.5)) ||
		     (dlg < (car->_dimension_x * 4.0)))) {

	    if (canOverlap) {
		maxdlg = fabs(dlg);
		/* risk of collision */
		car->_lightCmd |= RM_LIGHT_HEAD2;

		if (fabs(car->_trkPos.toRight - otherCar->_trkPos.toRight) < (MARGIN  - 2.0)) {
		    if (car->_trkPos.toRight < otherCar->_trkPos.toRight) {
			if (otherCar->_trkPos.toRight > MARGIN / 2.0) {
			    Tright[idx] = otherCar->_trkPos.toRight - (MARGIN * 2.0 - 1.0);
			    if (dny < 0) {
				if (car->_trkPos.toRight > 2.0) {
				    MaxSpeed[idx] = otherCar->_speed_x * .99;
				} else {
				    Tright[idx] += MARGIN * 2.0;
				}
			    }
			} else {
			    if ((dlg > (car->_dimension_x * 2.0)) &&
				(fabs(car->_trkPos.toRight - otherCar->_trkPos.toRight) < MARGIN)) {
				MaxSpeed[idx] = otherCar->_speed_x * .99;
				Tright[idx] = otherCar->_trkPos.toRight + (MARGIN * 2.0);
			    }
			}
		    } else {
			if (otherCar->_trkPos.toRight < seg->width - MARGIN / 2.0) {
			    Tright[idx] = otherCar->_trkPos.toRight + (MARGIN * 2.0 - 1.0);
			    if (dny > 0) {
				if (car->_trkPos.toRight < (seg->width - 2.0)) {
				    MaxSpeed[idx] = otherCar->_speed_x * .99;
				} else {
				    Tright[idx] -= MARGIN * 2.0;
				}
			    }
			} else {
			    if ((dlg > (car->_dimension_x * 2.0)) &&
				(fabs(car->_trkPos.toRight - otherCar->_trkPos.toRight) < (MARGIN / 2.0))) {
				MaxSpeed[idx] = otherCar->_speed_x * .99;
				Tright[idx] = otherCar->_trkPos.toRight - (MARGIN * 2.0);
			    }
			}
		    }
		    hold[idx] = Curtime + 1.0;
		    if ((dlg > (car->_dimension_x /2.0)) && (dlg < (car->_dimension_x * 3.0)) && (fabs(car->_trkPos.toRight - otherCar->_trkPos.toRight) < 2.0)) {
			MaxSpeed[idx] = otherCar->_speed_x * .95;
			car->_lightCmd |= RM_LIGHT_HEAD1;
		    }
		}
	    } else {
		/* Stay behind the front car */
		MaxSpeed[idx] = MIN(MaxSpeed[idx], otherCar->_speed_x * .99);
	    }
	}
    }

    if (Tright[idx] < 0.0) {
	Tright[idx] = 0.0;
    } else if (Tright[idx] > seg->width) {
	Tright[idx] = seg->width;
    }
    
    
}
