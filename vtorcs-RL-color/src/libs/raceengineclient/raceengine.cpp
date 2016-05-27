/***************************************************************************

    file        : raceengine.cpp
    created     : Sat Nov 23 09:05:23 CET 2002
    copyright   : (C) 2002 by Eric Espié
    email       : eric.espie@torcs.org
    version     : $Id: raceengine.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $

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
    @version	$Id: raceengine.cpp,v 1.19 2007/11/06 20:43:32 torcs Exp $
*/

#include <stdlib.h>
#include <stdio.h>
#include <tgfclient.h>
#include <robot.h>
#include <raceman.h>
#include <racescreens.h>
#include <robottools.h>

#include "racemain.h"
#include "racegl.h"
#include "raceinit.h"
#include "raceresults.h"

#include "raceengine.h"

#include <math.h>

static char	buf[1024];
static double	msgDisp;
static double	bigMsgDisp;

tRmInfo	*ReInfo = 0;
int	RESTART = 0;

// GIUSE - debug - size of the image to be sent through udp
// Make it zero to deactivate
int GIUSEIMGSIZE = 64;

static void ReRaceRules(tCarElt *car);

// GIUSE - VISION HERE!
static unsigned char* tmpimg;

/* Compute Pit stop time */
static void
ReUpdtPitTime(tCarElt *car)
{
	tSituation *s = ReInfo->s;
	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);
	int i;

	switch (car->_pitStopType) {
		case RM_PIT_REPAIR:
			info->totalPitTime = 2.0f + fabs((double)(car->_pitFuel)) / 8.0f + (tdble)(fabs((double)(car->_pitRepair))) * 0.007f;
			car->_scheduledEventTime = s->currentTime + info->totalPitTime;
			ReInfo->_reSimItf.reconfig(car);
			for (i=0; i<4; i++) {
				car->_tyreCondition(i) = 1.01;
				car->_tyreT_in(i) = 50.0;
				car->_tyreT_mid(i) = 50.0;
				car->_tyreT_out(i) = 50.0;
			}
			break;
		case RM_PIT_STOPANDGO:
			info->totalPitTime = 0.0;
			car->_scheduledEventTime = s->currentTime;
			break;
	}
}

/* Return from interactive pit information */
static void
ReUpdtPitCmd(void *pvcar)
{
	tCarElt *car = (tCarElt*)pvcar;

	ReUpdtPitTime(car);
	//ReStart(); /* resynchro */
	GfuiScreenActivate(ReInfo->_reGameScreen);
}

static void
ReRaceMsgUpdate(void)
{
	if(getTextOnly()==false)
	{
		if (ReInfo->_reCurTime > msgDisp) {
			ReSetRaceMsg("");
		}
		if (ReInfo->_reCurTime > bigMsgDisp) {
			ReSetRaceBigMsg("");
		}
	}
}

static void
ReRaceMsgSet(char *msg, double life)
{
	if(getTextOnly()) return;
	ReSetRaceMsg(msg);
	msgDisp = ReInfo->_reCurTime + life;
}


static void
ReRaceBigMsgSet(char *msg, double life)
{
	if(getTextOnly()) return;
	ReSetRaceBigMsg(msg);
	bigMsgDisp = ReInfo->_reCurTime + life;
}


// GIUSE - TODO: quick hack, find them a place!
static unsigned char* tmpRGBimg = (unsigned char*)malloc( 3 * GIUSEIMGSIZE * GIUSEIMGSIZE * sizeof(unsigned char) );
static double* RGBscales = (double*)malloc( 3 * sizeof(double) );


// GIUSE - VISION HERE!!!
static void
visionUpdate()
{
//  printf("START visionUpdate\n");
    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    // set grayscale conversion scales before get pixels
    // GIUSE - BUG!! IT'S SO INCREDIBLY SLOOOOOW!
//    glPixelTransferf(GL_RED_SCALE, 0.299f);
//    glPixelTransferf(GL_GREEN_SCALE, 0.587f);
//    glPixelTransferf(GL_BLUE_SCALE, 0.114f);

    glReadBuffer(GL_FRONT);
//    glReadPixels((sw-vw)/2, (sh-vh)/2, vw, vh, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)ReInfo.vision->img);
// documentation: http://www.opengl.org/sdk/docs/man/xhtml/glReadPixels.xml

// GIUSE - there's a bug in using glPixelTransferf and GL_LUMINANCE, it gets SLOOOOOW...
// switching to doing it by hand from rgb image (which seems quite fast)
    glReadPixels(
      (ReInfo->vision->sw - ReInfo->vision->vw) / 2,
      (ReInfo->vision->sh - ReInfo->vision->vh) / 2,
      ReInfo->vision->vw,  ReInfo->vision->vh,
//      100,100,100,100,
//      GL_LUMINANCE, GL_UNSIGNED_BYTE,
      GL_RGB, GL_UNSIGNED_BYTE,
      (GLvoid*)tmpRGBimg
//      (GLvoid*)ReInfo->vision->img
    );


    // GIUSE - TODO: that was only a quick hack, bring them out of here!!
/*    RGBscales[0]=0.299;
    RGBscales[1]=0.587;
    RGBscales[2]=0.114;

    double avg;
    for (int pixel=0; pixel < GIUSEIMGSIZE*GIUSEIMGSIZE; pixel++)
    {
	    for (int channel=0, avg=0; channel<3; channel++)
	    {
		    avg += RGBscales[channel] * tmpRGBimg[3*pixel+channel];
	    }

	    ReInfo->vision->img[pixel] = (unsigned char) avg;
    }
*/
    // JAN: a hack of a hack: RGB -> HSB (using just the S value)
    //double avg,r,g,b,min,max,s,delta;
		/*
    for (int pixel=0; pixel < GIUSEIMGSIZE*GIUSEIMGSIZE; pixel++)
    {
	r = tmpRGBimg[3*pixel];
	g = tmpRGBimg[3*pixel+1];
	b = tmpRGBimg[3*pixel+2];
	min = fmin(fmin(r,g),b);
	max = fmax(fmax(r,g),b);
	delta = max - min;
	if(max != 0){
	    s = delta / max;
	}else{
	    s = 0;
	}
        ReInfo->vision->img[pixel] = (unsigned char) (255.0*s);
    }
		*/

		double avg,r,g,b,min,max,s,delta;
		for (int pixel=0; pixel < 3*GIUSEIMGSIZE*GIUSEIMGSIZE; pixel++)
		{
				ReInfo->vision->img[pixel] = (unsigned char) (tmpRGBimg[pixel]);
		}

    // must(?) restore scales to default values
//    glPixelTransferf(GL_RED_SCALE, 1);
//    glPixelTransferf(GL_GREEN_SCALE, 1);
//    glPixelTransferf(GL_BLUE_SCALE, 1);

}


static void
ReManage(tCarElt *car)
{
	int i, pitok;
	tTrackSeg *sseg;
	tdble wseg;
	static float color[] = {0.0, 0.0, 1.0, 1.0};
	tSituation *s = ReInfo->s;

	tReCarInfo *info = &(ReInfo->_reCarInfo[car->index]);

	// GIUSE: VISION HERE!!
  if( getVision() ){
	  // GIUSE: TODO: this assignment needs to be done ONLY ONCE!! TAKE THIS OUT somehow!
    car->vision = ReInfo->vision;
	  visionUpdate();
  }


	if (car->_speed_x > car->_topSpeed) {
		car->_topSpeed = car->_speed_x;
	}

	// For practice and qualif.
	if (car->_speed_x > info->topSpd) {
		info->topSpd = car->_speed_x;
	}
	if (car->_speed_x < info->botSpd) {
		info->botSpd = car->_speed_x;
	}

	// Pitstop.
	if (car->_pit) {
		if (car->ctrl.raceCmd & RM_CMD_PIT_ASKED) {
			// Pit already occupied?
			if (car->_pit->pitCarIndex == TR_PIT_STATE_FREE) {
				sprintf(car->ctrl.msg[2], "Can Pit");
			} else {
				sprintf(car->ctrl.msg[2], "Pit Occupied");
			}
			memcpy(car->ctrl.msgColor, color, sizeof(car->ctrl.msgColor));
		}
		if (car->_state & RM_CAR_STATE_PIT) {
			car->ctrl.raceCmd &= ~RM_CMD_PIT_ASKED; // clear the flag.
			if (car->_scheduledEventTime < s->currentTime) {
				car->_state &= ~RM_CAR_STATE_PIT;
				car->_pit->pitCarIndex = TR_PIT_STATE_FREE;
				sprintf(buf, "%s pit stop %.1fs", car->_name, info->totalPitTime);
				if (getTextOnly()==false)
					ReRaceMsgSet(buf, 5);
			} else {
				sprintf(car->ctrl.msg[2], "in pits %.1fs", s->currentTime - info->startPitTime);
			}
		} else if ((car->ctrl.raceCmd & RM_CMD_PIT_ASKED) &&
					car->_pit->pitCarIndex == TR_PIT_STATE_FREE )//&&
// GIUSE - VISION HERE!! - actually it's for the evolution, removing the damage limits while still computing the damage
//				   (s->_maxDammage == 0 || car->_dammage <= s->_maxDammage))
		{
			tdble lgFromStart = car->_trkPos.seg->lgfromstart;
			switch (car->_trkPos.seg->type) {
				case TR_STR:
					lgFromStart += car->_trkPos.toStart;
					break;
				default:
					lgFromStart += car->_trkPos.toStart * car->_trkPos.seg->radius;
					break;
			}
			if ((lgFromStart > car->_pit->lmin) && (lgFromStart < car->_pit->lmax)) {
				pitok = 0;
				int side;
				tdble toBorder;
				if (ReInfo->track->pits.side == TR_RGT) {
					side = TR_SIDE_RGT;
					toBorder = car->_trkPos.toRight;
				} else {
					side = TR_SIDE_LFT;
					toBorder = car->_trkPos.toLeft;
				}
				sseg = car->_trkPos.seg->side[side];
				wseg = RtTrackGetWidth(sseg, car->_trkPos.toStart);
				if (sseg->side[side]) {
					sseg = sseg->side[side];
					wseg += RtTrackGetWidth(sseg, car->_trkPos.toStart);
				}
				if (((toBorder + wseg) < (ReInfo->track->pits.width - car->_dimension_y / 2.0)) &&
					(fabs(car->_speed_x) < 1.0) &&
					(fabs(car->_speed_y) < 1.0))
				{
					pitok = 1;
				}
				if (pitok) {
					car->_state |= RM_CAR_STATE_PIT;
					car->_nbPitStops++;
					for (i = 0; i < car->_pit->freeCarIndex; i++) {
						if (car->_pit->car[i] == car) {
							car->_pit->pitCarIndex = i;
							break;
						}
					}
					info->startPitTime = s->currentTime;
					sprintf(buf, "%s in pits", car->_name);
					if(getTextOnly()==false)
						ReRaceMsgSet(buf, 5);
					if (car->robot->rbPitCmd(car->robot->index, car, s) == ROB_PIT_MENU) {
						// the pit cmd is modified by menu.
						if(getTextOnly()==false)
						{
							ReStop();
							RmPitMenuStart(car, (void*)car, ReUpdtPitCmd);
						}
						// GIUSE - VISION HERE
						else if(getVision())
						{
							ReStop();
							RmPitMenuStart(car, (void*)car, ReUpdtPitCmd);
						}

					} else {
						//if(getTextOnly()==false) //LUIGI - inutile?? - TODO
						//{
							ReUpdtPitTime(car);
						//}
					}
				}
			}
		}
	}

	/* Start Line Crossing */
	if (info->prevTrkPos.seg != car->_trkPos.seg) {
	if ((info->prevTrkPos.seg->raceInfo & TR_LAST) && (car->_trkPos.seg->raceInfo & TR_START)) {
		if (info->lapFlag == 0) {
		if ((car->_state & RM_CAR_STATE_FINISH) == 0) {
			car->_laps++;
			car->_remainingLaps--;
			if (car->_laps > 1) {
			car->_lastLapTime = s->currentTime - info->sTime;
			car->_curTime += car->_lastLapTime;
			if (car->_bestLapTime != 0) {
				car->_deltaBestLapTime = car->_lastLapTime - car->_bestLapTime;
			}
			if ((car->_lastLapTime < car->_bestLapTime) || (car->_bestLapTime == 0)) {
				car->_bestLapTime = car->_lastLapTime;
			}
			if (car->_pos != 1) {
				car->_timeBehindLeader = car->_curTime - s->cars[0]->_curTime;
				car->_lapsBehindLeader = s->cars[0]->_laps - car->_laps;
				car->_timeBehindPrev = car->_curTime - s->cars[car->_pos - 2]->_curTime;
				s->cars[car->_pos - 2]->_timeBeforeNext = car->_timeBehindPrev;
			} else {
				car->_timeBehindLeader = 0;
				car->_lapsBehindLeader = 0;
				car->_timeBehindPrev = 0;
			}
			if (getTextOnly() && car->_pos == 1) printf("lap %d: %f\n",car->_laps-1,car->_lastLapTime);
			info->sTime = s->currentTime;
			switch (ReInfo->s->_raceType) {
			case RM_TYPE_PRACTICE:
				if (ReInfo->_displayMode == RM_DISP_MODE_NONE) {
				ReInfo->_refreshDisplay = 1;
				char *t1, *t2;
				t1 = GfTime2Str(car->_lastLapTime, 0);
				t2 = GfTime2Str(car->_bestLapTime, 0);
				if(getTextOnly()==false)
				{
					sprintf(buf,"lap: %02d   time: %s  best: %s  top spd: %.2f    min spd: %.2f    damage: %d",
							car->_laps - 1, t1, t2,
							info->topSpd * 3.6, info->botSpd * 3.6, car->_dammage);
					ReResScreenAddText(buf);
				}
				// else	if(getVision())
//				{
//					sprintf(buf,"lap: %02d   time: %s  best: %s  top spd: %.2f    min spd: %.2f    damage: %d",
//							car->_laps - 1, t1, t2,
//							info->topSpd * 3.6, info->botSpd * 3.6, car->_dammage);
//					ReResScreenAddText(buf);
//				}
				free(t1);
				free(t2);
				}
				/* save the lap result */
				ReSavePracticeLap(car);
				break;
			case RM_TYPE_QUALIF:
				if (ReInfo->_displayMode == RM_DISP_MODE_NONE) {
				ReUpdateQualifCurRes(car);
				}
				break;
			}
			} else {
			if ((ReInfo->_displayMode == RM_DISP_MODE_NONE) && (ReInfo->s->_raceType == RM_TYPE_QUALIF)) {
				ReUpdateQualifCurRes(car);
			}
			}
			info->topSpd = car->_speed_x;
			info->botSpd = car->_speed_x;
			if ((car->_remainingLaps < 0) || (s->_raceState == RM_RACE_FINISHING)) {
			car->_state |= RM_CAR_STATE_FINISH;
			s->_raceState = RM_RACE_FINISHING;
			if (ReInfo->s->_raceType == RM_TYPE_RACE) {
				if (car->_pos == 1) {
					sprintf(buf, "Winner %s", car->_name);
					ReRaceBigMsgSet(buf, 10);
					if (getTextOnly()){
						printf( "*****************************************************\n");
						printf( "Winner %s\n", car->_name);
					}
				} else {
				char *numSuffix = "th";
				if (abs(12 - car->_pos) > 1) { /* leave suffix as 'th' for 11 to 13 */
					switch (car->_pos % 10) {
					case 1:
					numSuffix = "st";
					break;
					case 2:
					numSuffix = "nd";
					break;
					case 3:
					numSuffix = "rd";
					break;
					default:
					break;
					}
				}
				if(getTextOnly()==false)
				{
					sprintf(buf, "%s Finished %d%s", car->_name, car->_pos, numSuffix);
					ReRaceMsgSet(buf, 5);
				}else
					printf("%s Finished %d%s\n", car->_name, car->_pos, numSuffix);
				}
			}
			}
		} else {
			/* prevent infinite looping of cars around track, allow one lap after finish for the first car */
			for (i = 0; i < s->_ncars; i++) {
				s->cars[i]->_state |= RM_CAR_STATE_FINISH;
			}
			return;
		}
		} else {
		info->lapFlag--;
		}
	}
	if ((info->prevTrkPos.seg->raceInfo & TR_START) && (car->_trkPos.seg->raceInfo & TR_LAST)) {
		/* going backward through the start line */
		info->lapFlag++;
	}
	}
	ReRaceRules(car);
	info->prevTrkPos = car->_trkPos;
	car->_curLapTime = s->currentTime - info->sTime;
	car->_distFromStartLine = car->_trkPos.seg->lgfromstart +
	(car->_trkPos.seg->type == TR_STR ? car->_trkPos.toStart : car->_trkPos.toStart * car->_trkPos.seg->radius);
	car->_distRaced = (car->_laps - 1) * ReInfo->track->length + car->_distFromStartLine;
}

static void
ReSortCars(void)
{
    int		i,j;
    tCarElt	*car;
    int		allfinish;
    tSituation	*s = ReInfo->s;

    if ((s->cars[0]->_state & RM_CAR_STATE_FINISH) == 0) {
	allfinish = 0;
    } else {
	allfinish = 1;
    }
    for (i = 1; i < s->_ncars; i++) {
	j = i;
	while (j > 0) {
	    if ((s->cars[j]->_state & RM_CAR_STATE_FINISH) == 0) {
		allfinish = 0;
		if (s->cars[j]->_distRaced > s->cars[j-1]->_distRaced) {
		    car = s->cars[j];
		    s->cars[j] = s->cars[j-1];
		    s->cars[j-1] = car;
		    s->cars[j]->_pos = j+1;
		    s->cars[j-1]->_pos = j;
		    j--;
		    continue;
		}
	    }
	    j = 0;
	}
    }
    if (allfinish) {
	ReInfo->s->_raceState = RM_RACE_ENDED;
    }

    for  (i = 0; i < s->_ncars; i++)
    {
	if (s->cars[i]->RESET==1)
	{
		//printf("******* RESETTING *****\n");
		ReInfo->_reSimItf.config(s->cars[i], ReInfo);
		s->cars[i]->RESET=0;
		sprintf(buf, "RELOADING");
		ReRaceMsgSet(buf, 4);
	}
	if (s->cars[i]->RESTART==1)
	{
		printf("******* RESTARTING *****\n");
		RESTART = 1;
		s->cars[i]->RESTART=0;
	}
    }

    if (RESTART == 1)
    {
	ReInfo->_reState = RE_STATE_RACE_STOP;
    }
}

/* Compute the race rules and penalties */
static void
ReRaceRules(tCarElt *car)
{
    tCarPenalty		*penalty;
    tTrack		*track = ReInfo->track;
    tRmCarRules		*rules = &(ReInfo->rules[car->index]);
    tTrackSeg		*seg = RtTrackGetSeg(&(car->_trkPos));
    tReCarInfo		*info = &(ReInfo->_reCarInfo[car->index]);
    tTrackSeg		*prevSeg = RtTrackGetSeg(&(info->prevTrkPos));
    static float	color[] = {0.0, 0.0, 1.0, 1.0};

	// DNF cars which need too much time for the current lap, this is mainly to avoid
	// that a "hanging" driver can stop the quali from finishing.
	// Allowed time is longest pitstop possible + time for tracklength with speed??? (currently fixed 10 [m/s]).
	// for simplicity. Human driver is an exception to this rule, to allow explorers
	// to enjoy the landscape.
	// TODO: Make it configurable.
	if ((car->_curLapTime > 84.5 + ReInfo->track->length/10.0) &&
		(car->_driverType != RM_DRV_HUMAN) && getLaptimeLimit())
	{
		car->_state |= RM_CAR_STATE_ELIMINATED;
	    return;
	}

	if (car->_skillLevel < 3) {
	/* only for the pros */
	return;
    }

	penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
    if (penalty) {
	if (car->_laps > penalty->lapToClear) {
	    /* too late to clear the penalty, out of race */
	    car->_state |= RM_CAR_STATE_ELIMINATED;
	    return;
	}
	switch (penalty->penalty) {
	case RM_PENALTY_DRIVETHROUGH:
	    sprintf(car->ctrl.msg[3], "Drive Through Penalty");
	    break;
	case RM_PENALTY_STOPANDGO:
	    sprintf(car->ctrl.msg[3], "Stop And Go Penalty");
	    break;
	default:
	    *(car->ctrl.msg[3]) = 0;
	    break;
	}
	memcpy(car->ctrl.msgColor, color, sizeof(car->ctrl.msgColor));
    }


    if (prevSeg->raceInfo & TR_PITSTART) {
	/* just entered the pit lane */
	if (seg->raceInfo & TR_PIT) {
	    /* may be a penalty can be cleaned up */
	    if (penalty) {
		switch (penalty->penalty) {
		case RM_PENALTY_DRIVETHROUGH:
		    sprintf(buf, "%s DRIVE THROUGH PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(buf, 5);
		    rules->ruleState |= RM_PNST_DRIVETHROUGH;
		    break;
		case RM_PENALTY_STOPANDGO:
		    sprintf(buf, "%s STOP&GO PENALTY CLEANING", car->_name);
		    ReRaceMsgSet(buf, 5);
		    rules->ruleState |= RM_PNST_STOPANDGO;
		    break;
		}
	    }
	}
    } else if (prevSeg->raceInfo & TR_PIT) {
	if (seg->raceInfo & TR_PIT) {
	    /* the car stopped in pits */
	    if (car->_state & RM_CAR_STATE_PIT) {
		if (rules->ruleState & RM_PNST_DRIVETHROUGH) {
		    /* it's not more a drive through */
		    rules->ruleState &= ~RM_PNST_DRIVETHROUGH;
		} else if (rules->ruleState & RM_PNST_STOPANDGO) {
		    rules->ruleState |= RM_PNST_STOPANDGO_OK;
		}
	    } else {
                if(rules->ruleState & RM_PNST_STOPANDGO_OK && car->_pitStopType != RM_PIT_STOPANDGO) {
		    rules->ruleState &= ~ ( RM_PNST_STOPANDGO | RM_PNST_STOPANDGO_OK );
		}
	    }
	} else if (seg->raceInfo & TR_PITEND) {
	    /* went out of the pit lane, check if the current penalty is cleared */
	    if (rules->ruleState & (RM_PNST_DRIVETHROUGH | RM_PNST_STOPANDGO_OK)) {
		/* clear the penalty */
		sprintf(buf, "%s penalty cleared", car->_name);
		ReRaceMsgSet(buf, 5);
		penalty = GF_TAILQ_FIRST(&(car->_penaltyList));
		GF_TAILQ_REMOVE(&(car->_penaltyList), penalty, link);
		FREEZ(penalty);
	    }
	    rules->ruleState = 0;
	} else {
	    /* went out of the pit lane illegally... */
	    /* it's a new stop and go... */
	    if (!(rules->ruleState & RM_PNST_STNGO)) {
		sprintf(buf, "%s STOP&GO PENALTY", car->_name);
		ReRaceMsgSet(buf, 5);
		penalty = (tCarPenalty*)calloc(1, sizeof(tCarPenalty));
		penalty->penalty = RM_PENALTY_STOPANDGO;
		penalty->lapToClear = car->_laps + 5;
		GF_TAILQ_INSERT_TAIL(&(car->_penaltyList), penalty, link);
		rules->ruleState = RM_PNST_STNGO;
	    }
	}
    } else if (seg->raceInfo & TR_PITEND) {
	rules->ruleState = 0;
    } else if (seg->raceInfo & TR_PIT) {
	/* entrered the pits not from the pit entry... */
	/* it's a new stop and go... */
	if (!(rules->ruleState & RM_PNST_STNGO)) {
	    sprintf(buf, "%s STOP&GO PENALTY", car->_name);
	    ReRaceMsgSet(buf, 5);
	    penalty = (tCarPenalty*)calloc(1, sizeof(tCarPenalty));
	    penalty->penalty = RM_PENALTY_STOPANDGO;
	    penalty->lapToClear = car->_laps + 5;
	    GF_TAILQ_INSERT_TAIL(&(car->_penaltyList), penalty, link);
	    rules->ruleState = RM_PNST_STNGO;
	}
    }

    if (seg->raceInfo & TR_SPEEDLIMIT) {
	if (!(rules->ruleState & (RM_PNST_SPD | RM_PNST_STNGO)) && (car->_speed_x > track->pits.speedLimit)) {
	    sprintf(buf, "%s DRIVE THROUGH PENALTY", car->_name);
	    ReRaceMsgSet(buf, 5);
	    rules->ruleState |= RM_PNST_SPD;
	    penalty = (tCarPenalty*)calloc(1, sizeof(tCarPenalty));
	    penalty->penalty = RM_PENALTY_DRIVETHROUGH;
	    penalty->lapToClear = car->_laps + 5;
	    GF_TAILQ_INSERT_TAIL(&(car->_penaltyList), penalty, link);
	}
    }


}


static void
ReOneStep(double deltaTimeIncrement)
{
	int i;
	tRobotItf *robot;
	tSituation *s = ReInfo->s;

	// following code are from TORCS FAQ about the reset by robots
	/*
	bool restartRequested;

	for (i = 0; i < s->_ncars; i++)
	{
		if(s->cars[i]->ctrl.askRestart)
		{
			restartRequested ~ true;
			s->cars[i]->ctrl.askRestart = false;
		}
		if(restartRequested)
		{
			ReraceCleanup();
			ReInfo->_reState = RE_STATE_PRE_RACE;
			GfuiScreenActivate(ReInfo->_reGameScreen);
		}
	}
	*/
//	printf ("ReOneStep\n");


/* GIUSE - skip the message and start the race immediately

    if (ReInfo->s->currentTime < -1.0) {
	ReInfo->s->currentTime = -1.0;
	ReInfo->_reLastTime = -1.0;


	if (getTextOnly() == false)
	{
		if (floor(s->currentTime) == -2.0) {
			ReRaceBigMsgSet("Ready", 1.0);
		} else if (floor(s->currentTime) == -1.0) {
			ReRaceBigMsgSet("Set", 1.0);
		} else if (floor(s->currentTime) == 0.0) {
			ReRaceBigMsgSet("Go", 1.0);
		}
	}
*/

  // GIUSE - FASTER THEN RUNTIME ACTIVATION FOR NON-TEXTUAL COMPUTATION
	ReInfo->_reCurTime += deltaTimeIncrement * ReInfo->_reTimeMult * getSpeedMult();
//	ReInfo->_reCurTime += deltaTimeIncrement * ReInfo->_reTimeMult; /* "Real" time */
	s->currentTime += deltaTimeIncrement; /* Simulated time */

// GIUSE
//	if (s->currentTime < 0) {
	if (s->currentTime <= 0) {
		/* no simu yet */
		ReInfo->s->_raceState = RM_RACE_PRESTART;
	} else if (ReInfo->s->_raceState == RM_RACE_PRESTART) {
		ReInfo->s->_raceState = RM_RACE_RUNNING;
		s->currentTime = 0.0; /* resynchronize */
		ReInfo->_reLastTime = 0.0;
	}

	START_PROFILE("rbDrive*");
	if ((s->currentTime - ReInfo->_reLastTime) >= RCM_MAX_DT_ROBOTS) {
		s->deltaTime = s->currentTime - ReInfo->_reLastTime;
		for (i = 0; i < s->_ncars; i++) {
			if ((s->cars[i]->_state & RM_CAR_STATE_NO_SIMU) == 0) {
				robot = s->cars[i]->robot;
				robot->rbDrive(robot->index, s->cars[i], s);
			}
		}
		ReInfo->_reLastTime = s->currentTime;
	}
	STOP_PROFILE("rbDrive*");

	START_PROFILE("_reSimItf.update*");
	ReInfo->_reSimItf.update(s, deltaTimeIncrement, -1);
	for (i = 0; i < s->_ncars; i++) {
		ReManage(s->cars[i]);//get image here
	}
	STOP_PROFILE("_reSimItf.update*");

	ReRaceMsgUpdate();
	ReSortCars();
}

void
ReStart(void)
{
    ReInfo->_reRunning = 1;
    ReInfo->_reCurTime = GfTimeClock() - RCM_MAX_DT_SIMU;

    // fill the vision structure
    if( getVision() ){
      ReInfo->vision = (tRmVisionImg*) malloc( sizeof(tRmVisionImg) );

      GfScrGetSize(&ReInfo->vision->sw, &ReInfo->vision->sh, &ReInfo->vision->vw, &ReInfo->vision->vh);

      // GIUSE - debug - fixed image size to try the speed of udp
      if( GIUSEIMGSIZE > 0 )
        ReInfo->vision->sw = ReInfo->vision->sh = ReInfo->vision->vw = ReInfo->vision->vh = GIUSEIMGSIZE;

      ReInfo->vision->imgsize = 3*ReInfo->vision->vw * ReInfo->vision->vh; // for RGB
      ReInfo->vision->img = (unsigned char*)malloc(ReInfo->vision->imgsize * sizeof(unsigned char));
      if (ReInfo->vision->img == NULL)  exit(-1); // malloc fail

      // GIUSE - let's avoid zero-images if sent before grabbing the next frame
      memset(ReInfo->vision->img, 1, ReInfo->vision->imgsize * sizeof(unsigned char));
      memset(tmpRGBimg, 1, 3 * GIUSEIMGSIZE * GIUSEIMGSIZE * sizeof(unsigned char));
      memset(RGBscales, 1, 3 * sizeof(double));

      printf( "sw %d - sh %d - vw %d - vh %d - imgsize %d\n", ReInfo->vision->sw, ReInfo->vision->sh, ReInfo->vision->vw, ReInfo->vision->vh, ReInfo->vision->imgsize);

      visionUpdate(); // put first image
    }
}

void
ReStop(void)
{
    ReInfo->_reRunning = 0;
    if( getVision() ){
      free(ReInfo->vision->img);
      free(tmpimg);
    }
}

static void
reCapture(void)
{
    unsigned char	*img;
    int			sw, sh, vw, vh;
    tRmMovieCapture	*capture = &(ReInfo->movieCapture);

    GfScrGetSize(&sw, &sh, &vw, &vh);
    img = (unsigned char*)malloc(vw * vh * 3);
    if (img == NULL) {
	return;
    }

    glPixelStorei(GL_PACK_ROW_LENGTH, 0);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    glReadBuffer(GL_FRONT);
    glReadPixels((sw-vw)/2, (sh-vh)/2, vw, vh, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)img);

    sprintf(buf, "%s/torcs-%4.4d-%8.8d.png", capture->outputBase, capture->currentCapture, capture->currentFrame++);
    GfImgWritePng(img, buf, vw, vh);
    free(img);
}


int
ReUpdate(void)
{
    double 		t;
    tRmMovieCapture	*capture;
    if (getTextOnly()==false)
    {
  		START_PROFILE("ReUpdate");
  		ReInfo->_refreshDisplay = 0;
		  switch (ReInfo->_displayMode) {
		  case RM_DISP_MODE_NORMAL:
		  t = GfTimeClock();

		  START_PROFILE("ReOneStep*");
		  while (ReInfo->_reRunning && ((t - ReInfo->_reCurTime) > RCM_MAX_DT_SIMU)) {
			  ReOneStep(RCM_MAX_DT_SIMU);
		  }
		  STOP_PROFILE("ReOneStep*");

		  GfuiDisplay();
		  ReInfo->_reGraphicItf.refresh(ReInfo->s);
		  glutPostRedisplay();	/* Callback -> reDisplay */
		  break;

		  case RM_DISP_MODE_NONE:
		  ReOneStep(RCM_MAX_DT_SIMU);
		  if (ReInfo->_refreshDisplay) {
			  GfuiDisplay();
		  }
		  glutPostRedisplay();	/* Callback -> reDisplay */
		  break;

		  case RM_DISP_MODE_CAPTURE:
		  capture = &(ReInfo->movieCapture);
		  while ((ReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame) {
			  ReOneStep(capture->deltaSimu);
		  }
		  capture->lastFrame = ReInfo->_reCurTime;

		  GfuiDisplay();
		  ReInfo->_reGraphicItf.refresh(ReInfo->s);
		  reCapture();
		  glutPostRedisplay();	/* Callback -> reDisplay */
		  break;

		  }
		  STOP_PROFILE("ReUpdate");
    }
    // GIUSE - VISION HERE!!
    else if(getVision())
    {
  		START_PROFILE("ReUpdate");
//  		ReInfo->_refreshDisplay = 0;
//		  switch (ReInfo->_displayMode) {

//		    case RM_DISP_MODE_NORMAL:
//		    t = GfTimeClock();
//		    START_PROFILE("ReOneStep*");
//		    while (ReInfo->_reRunning && ((t - ReInfo->_reCurTime) > RCM_MAX_DT_SIMU))
//			    ReOneStep(RCM_MAX_DT_SIMU);
//		    STOP_PROFILE("ReOneStep*");
//		    GfuiDisplay();
//		    ReInfo->_reGraphicItf.refresh(ReInfo->s);
//		    glutPostRedisplay();	/* Callback -> reDisplay */
//		    break;

//		    case RM_DISP_MODE_NONE:
		    ReOneStep(RCM_MAX_DT_SIMU);
//		    if (ReInfo->_refreshDisplay)
//			    GfuiDisplay();
		    glutPostRedisplay();	/* Callback -> reDisplay */
//		    break;

//		    case RM_DISP_MODE_CAPTURE:
//		    capture = &(ReInfo->movieCapture);
//		    while ((ReInfo->_reCurTime - capture->lastFrame) < capture->deltaFrame)
//			    ReOneStep(capture->deltaSimu);
//		    capture->lastFrame = ReInfo->_reCurTime;
//		    GfuiDisplay();
//		    ReInfo->_reGraphicItf.refresh(ReInfo->s);
//		    reCapture();
//		    glutPostRedisplay();	/* Callback -> reDisplay */
//		    break;

//		  }
		  STOP_PROFILE("ReUpdate");
    }
    else // text only
    {
    	ReOneStep(RCM_MAX_DT_SIMU);
    	START_PROFILE("ReUpdate");
    	STOP_PROFILE("ReUpdate");
    }

    return RM_ASYNC;
}

void
ReTimeMod (void *vcmd)
{
    long cmd = (long)vcmd;

    switch ((int)cmd) {
    case 0:
	ReInfo->_reTimeMult *= 2.0;
	if (ReInfo->_reTimeMult > 64.0) {
	    ReInfo->_reTimeMult = 64.0;
	}
	break;
    case 1:
	ReInfo->_reTimeMult *= 0.5;
	if (ReInfo->_reTimeMult < 0.25) {
	    ReInfo->_reTimeMult = 0.25;
	}
	break;
    case 2:
    default:
	ReInfo->_reTimeMult = 1.0;
	break;
    }
    sprintf(buf, "Time x%.2f", 1.0 / ReInfo->_reTimeMult);
    ReRaceMsgSet(buf, 5);
}
