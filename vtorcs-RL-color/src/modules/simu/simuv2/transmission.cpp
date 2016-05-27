/***************************************************************************

    file                 : transmission.cpp
    created              : Sun Mar 19 00:07:19 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: transmission.cpp,v 1.24 2007/10/30 21:45:23 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sim.h"

static char *gearname[MAX_GEARS] = {"r", "n", "1", "2", "3", "4", "5", "6", "7", "8"};

void
SimTransmissionConfig(tCar *car)
{
	void		*hdle = car->params;
	tCarElt		*carElt = car->carElt;
	tdble		clutchI;
	tTransmission	*trans = &(car->transmission);
	tClutch		*clutch = &(trans->clutch);
	tDifferential	*differential;
	char		*transType;
	int			i, j;
	tdble		gRatio, fRatio, gEff, fEff;
	tdble		gearI;
	char		path[256];
	
	clutchI		= GfParmGetNum(hdle, SECT_CLUTCH, PRM_INERTIA, (char*)NULL, 0.12f);
	transType		= GfParmGetStr(hdle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	clutch->releaseTime	= GfParmGetNum(hdle, SECT_GEARBOX, PRM_SHIFTTIME, (char*)NULL, 0.2f);
	
	fRatio = 0;
	gEff   = 0;
	
	/* Link between the differentials */
	for (j = 0; j < 2; j++) {
		trans->differential[TRANS_FRONT_DIFF].inAxis[j]  = &(car->wheel[j].feedBack);
		trans->differential[TRANS_FRONT_DIFF].outAxis[j] = &(car->wheel[j].in);
	}

	for (j = 0; j < 2; j++) {
		trans->differential[TRANS_REAR_DIFF].inAxis[j]  = &(car->wheel[2+j].feedBack);
		trans->differential[TRANS_REAR_DIFF].outAxis[j] = &(car->wheel[2+j].in);
	}

	trans->differential[TRANS_CENTRAL_DIFF].inAxis[0]  = &(trans->differential[TRANS_FRONT_DIFF].feedBack);
	trans->differential[TRANS_CENTRAL_DIFF].outAxis[0] = &(trans->differential[TRANS_FRONT_DIFF].in);
	
	trans->differential[TRANS_CENTRAL_DIFF].inAxis[1]  = &(trans->differential[TRANS_REAR_DIFF].feedBack);
	trans->differential[TRANS_CENTRAL_DIFF].outAxis[1] = &(trans->differential[TRANS_REAR_DIFF].in);
	
	if (strcmp(VAL_TRANS_RWD, transType) == 0) {
		SimDifferentialConfig(hdle, SECT_REARDIFFERENTIAL, &(trans->differential[TRANS_REAR_DIFF]));
		trans->type = TRANS_RWD;
		fRatio = trans->differential[TRANS_REAR_DIFF].ratio;
		fEff   = trans->differential[TRANS_REAR_DIFF].efficiency;
	} else if (strcmp(VAL_TRANS_FWD, transType) == 0) {
		SimDifferentialConfig(hdle, SECT_FRNTDIFFERENTIAL, &(trans->differential[TRANS_FRONT_DIFF]));
		trans->type = TRANS_FWD;
		fRatio = trans->differential[TRANS_FRONT_DIFF].ratio;
		fEff   = trans->differential[TRANS_FRONT_DIFF].efficiency;
	} else if (strcmp(VAL_TRANS_4WD, transType) == 0) {
		SimDifferentialConfig(hdle, SECT_FRNTDIFFERENTIAL, &(trans->differential[TRANS_FRONT_DIFF]));
		SimDifferentialConfig(hdle, SECT_REARDIFFERENTIAL, &(trans->differential[TRANS_REAR_DIFF]));
		SimDifferentialConfig(hdle, SECT_CENTRALDIFFERENTIAL, &(trans->differential[TRANS_CENTRAL_DIFF]));
		trans->type = TRANS_4WD;
		fRatio = trans->differential[TRANS_CENTRAL_DIFF].ratio;
		fEff   = trans->differential[TRANS_FRONT_DIFF].efficiency * trans->differential[TRANS_CENTRAL_DIFF].efficiency * trans->differential[TRANS_REAR_DIFF].efficiency;
	}
	
	trans->gearbox.gearMax = 0;
	for (i = MAX_GEARS - 1; i >= 0; i--) {
		sprintf(path, "%s/%s/%s", SECT_GEARBOX, ARR_GEARS, gearname[i]);
		gRatio = GfParmGetNum(hdle, path, PRM_RATIO, (char*)NULL, 0.0);
		if ((trans->gearbox.gearMax == 0) && (gRatio != 0.0)) {
			trans->gearbox.gearMax = i - 1;
		}
		if (gRatio == 0.0) {
			carElt->priv.gearRatio[i] = trans->overallRatio[i] = 0;
			trans->freeI[i] = trans->driveI[i] = 0;
			trans->gearEff[i] = 1.0;
			continue;
		}
		carElt->priv.gearRatio[i] = trans->overallRatio[i] = gRatio * fRatio;
		gEff = GfParmGetNum(hdle, path, PRM_EFFICIENCY, (char*)NULL, 1.0);
		if (gEff > 1.0) gEff = 1.0;
		if (gEff < 0.0) gEff = 0.0;
		gearI = GfParmGetNum(hdle, path, PRM_INERTIA, (char*)NULL, 0.0);
		trans->driveI[i] = (car->engine.I + gearI) * (gRatio * gRatio * fRatio * fRatio);
		trans->freeI[i] = gearI * (gRatio * gRatio * fRatio * fRatio);
		trans->gearEff[i] = gEff;
	}

	if (gRatio == 0) {
		/* no reverse */
		trans->gearbox.gearMin = 0;
		carElt->priv.gearOffset = 0;
	} else {
		trans->gearbox.gearMin = -1;
		carElt->priv.gearOffset = 1;
	}

	carElt->priv.gearNb = trans->gearbox.gearMax + 1;
	
	/* initial state */
	clutch->state = CLUTCH_RELEASING;
	clutch->timeToRelease = 0;
	trans->gearbox.gear = 0; /* neutral */
	trans->curI = trans->freeI[1];
	switch(trans->type) {
		case TRANS_RWD:
			differential = &(trans->differential[TRANS_REAR_DIFF]);
			differential->outAxis[0]->I = trans->curI / 2.0 + differential->inAxis[0]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[1]->I = trans->curI / 2.0 + differential->inAxis[1]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[0]->Tq = 0;
			differential->outAxis[1]->Tq = 0;
			break;
		case TRANS_FWD:
			differential = &(trans->differential[TRANS_FRONT_DIFF]);
			differential->outAxis[0]->I = trans->curI / 2.0 + differential->inAxis[0]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[1]->I = trans->curI / 2.0 + differential->inAxis[1]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[0]->Tq = 0;
			differential->outAxis[1]->Tq = 0;
			break;
		case TRANS_4WD:
			differential = &(trans->differential[TRANS_FRONT_DIFF]);
			differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[0]->Tq = 0;
			differential->outAxis[1]->Tq = 0;
			differential = &(trans->differential[TRANS_REAR_DIFF]);
			differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[0]->Tq = 0;
			differential->outAxis[1]->Tq = 0;
			differential = &(trans->differential[TRANS_CENTRAL_DIFF]);
			differential->outAxis[0]->I = trans->curI / 2.0 + differential->inAxis[0]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[1]->I = trans->curI / 2.0 + differential->inAxis[1]->I / trans->gearEff[trans->gearbox.gear+1];
			differential->outAxis[0]->Tq = 0;
			differential->outAxis[1]->Tq = 0;
		break;
	}
	
}

void
SimGearboxUpdate(tCar *car)
{
	/* manages gear change */
	tTransmission *trans = &(car->transmission);
	tClutch *clutch = &(trans->clutch);
	tGearbox *gearbox = &(trans->gearbox);
	tDifferential *differential = NULL;
	
	switch(trans->type) {
		case TRANS_RWD:
			differential = &(trans->differential[TRANS_REAR_DIFF]);
			break;
		case TRANS_FWD:
			differential = &(trans->differential[TRANS_FRONT_DIFF]);
			break;
		case TRANS_4WD:
			differential = &(trans->differential[TRANS_CENTRAL_DIFF]);
			break;
	}

	trans->curI = trans->driveI[gearbox->gear + 1] * clutch->transferValue + trans->freeI[gearbox->gear +  1] * (1.0f - clutch->transferValue);
	if (clutch->state == CLUTCH_RELEASING) {
		clutch->timeToRelease -= SimDeltaTime;
		if (clutch->timeToRelease <= 0.0f) {
			clutch->state = CLUTCH_RELEASED;
		} else  {
			// If user does not engage clutch, we do it automatically.
			if (clutch->transferValue > 0.99f) {
				clutch->transferValue = 0.0f;
				trans->curI = trans->freeI[gearbox->gear +  1];
				if (car->ctrl->accelCmd > 0.1f) {
					car->ctrl->accelCmd = 0.1f;
				}
		    }
		}
    } else if ((car->ctrl->gear > gearbox->gear)) {
		if (car->ctrl->gear <= gearbox->gearMax) {
			gearbox->gear = car->ctrl->gear;
			clutch->state = CLUTCH_RELEASING;
			if (gearbox->gear != 0) {
				clutch->timeToRelease = clutch->releaseTime;
			} else {
				clutch->timeToRelease = 0;
			}
			trans->curOverallRatio = trans->overallRatio[gearbox->gear+1];
			trans->curI = trans->freeI[gearbox->gear+1];
			differential->in.I = trans->curI + differential->feedBack.I / trans->gearEff[gearbox->gear+1];
			differential->outAxis[0]->I = trans->curI / 2.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
			differential->outAxis[1]->I = trans->curI / 2.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
			if (trans->type == TRANS_4WD) {
				differential = &(trans->differential[TRANS_FRONT_DIFF]);
				differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
				differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
				differential = &(trans->differential[TRANS_REAR_DIFF]);
				differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
				differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
			}
		}
    } else if ((car->ctrl->gear < gearbox->gear)) {
		if (car->ctrl->gear >= gearbox->gearMin) {
			gearbox->gear = car->ctrl->gear;
			clutch->state = CLUTCH_RELEASING;
			if (gearbox->gear != 0) {
				clutch->timeToRelease = clutch->releaseTime;
			} else {
				clutch->timeToRelease = 0;
			}
			trans->curOverallRatio = trans->overallRatio[gearbox->gear+1];
			trans->curI = trans->freeI[gearbox->gear+1];
			differential->in.I = trans->curI + differential->feedBack.I / trans->gearEff[gearbox->gear+1];
			differential->outAxis[0]->I = trans->curI / 2.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
			differential->outAxis[1]->I = trans->curI / 2.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
			if (trans->type == TRANS_4WD) {
				differential = &(trans->differential[TRANS_FRONT_DIFF]);
				differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
				differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
				differential = &(trans->differential[TRANS_REAR_DIFF]);
				differential->outAxis[0]->I = trans->curI / 4.0 + differential->inAxis[0]->I / trans->gearEff[gearbox->gear+1];
				differential->outAxis[1]->I = trans->curI / 4.0 + differential->inAxis[1]->I / trans->gearEff[gearbox->gear+1];
			}
		}
    }
}

void
SimTransmissionUpdate(tCar *car)
{
	tTransmission *trans = &(car->transmission);
	tClutch *clutch = &(trans->clutch);
	tDifferential *differential, *differential0, *differential1;
	tdble transfer = MIN(clutch->transferValue * 3.0, 1.0);
	
	switch(trans->type) {
		case TRANS_RWD:
			differential = &(trans->differential[TRANS_REAR_DIFF]);
			differential->in.Tq = car->engine.Tq * trans->curOverallRatio * transfer;
			SimDifferentialUpdate(car, differential, 1);
			SimUpdateFreeWheels(car, 0);
			break;
		case TRANS_FWD:
			differential = &(trans->differential[TRANS_FRONT_DIFF]);
			differential->in.Tq = car->engine.Tq * trans->curOverallRatio * transfer;
			SimDifferentialUpdate(car, differential, 1);
			SimUpdateFreeWheels(car, 1);
			break;
		case TRANS_4WD:
			differential = &(trans->differential[TRANS_CENTRAL_DIFF]);
			differential0 = &(trans->differential[TRANS_FRONT_DIFF]);
			differential1 = &(trans->differential[TRANS_REAR_DIFF]);
			
			differential->in.Tq = car->engine.Tq * trans->curOverallRatio * transfer;
			differential->inAxis[0]->spinVel = (differential0->inAxis[0]->spinVel + differential0->inAxis[1]->spinVel) / 2.0;
			differential->inAxis[1]->spinVel = (differential1->inAxis[0]->spinVel + differential1->inAxis[1]->spinVel) / 2.0;
			differential->inAxis[0]->Tq = (differential0->inAxis[0]->Tq + differential0->inAxis[1]->Tq) / differential->ratio;
			differential->inAxis[1]->Tq = (differential1->inAxis[0]->Tq + differential1->inAxis[1]->Tq) / differential->ratio;
			differential->inAxis[0]->brkTq = (differential0->inAxis[0]->brkTq + differential0->inAxis[1]->brkTq) / differential->ratio;
			differential->inAxis[1]->brkTq = (differential1->inAxis[0]->brkTq + differential1->inAxis[1]->brkTq) / differential->ratio;
			
			SimDifferentialUpdate(car, differential, 1);
			
			differential = differential0;
			SimDifferentialUpdate(car, differential, 0);
			
			differential = differential1;
			SimDifferentialUpdate(car, differential, 0);
			break;
	}
}
