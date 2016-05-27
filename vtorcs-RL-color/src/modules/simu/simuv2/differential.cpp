/***************************************************************************

    file                 : differential.cpp
    created              : Sun Mar 19 00:06:33 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: differential.cpp,v 1.11 2007/10/30 21:45:23 berniw Exp $

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

void SimDifferentialConfig(void *hdle, char *section, tDifferential *differential)
{
	char *type;
	
	differential->I		= GfParmGetNum(hdle, section, PRM_INERTIA, (char*)NULL, 0.1f);
	differential->efficiency	= GfParmGetNum(hdle, section, PRM_EFFICIENCY, (char*)NULL, 1.0f);
	differential->ratio		= GfParmGetNum(hdle, section, PRM_RATIO, (char*)NULL, 1.0f);
	differential->bias		= GfParmGetNum(hdle, section, PRM_BIAS, (char*)NULL, 0.1f);
	differential->dTqMin	= GfParmGetNum(hdle, section, PRM_MIN_TQ_BIAS, (char*)NULL, 0.05f);
	differential->dTqMax	= GfParmGetNum(hdle, section, PRM_MAX_TQ_BIAS, (char*)NULL, 0.80f) - differential->dTqMin;
	differential->dSlipMax	= GfParmGetNum(hdle, section, PRM_MAX_SLIP_BIAS, (char*)NULL, 0.2f);
	differential->lockInputTq	= GfParmGetNum(hdle, section, PRM_LOCKING_TQ, (char*)NULL, 300.0f);
	differential->viscosity	= GfParmGetNum(hdle, section, PRM_VISCOSITY_FACTOR, (char*)NULL, 2.0f);
	differential->viscomax	= 1 - exp(-differential->viscosity);
	
	type = GfParmGetStr(hdle, section, PRM_TYPE, VAL_DIFF_NONE);
	if (strcmp(type, VAL_DIFF_LIMITED_SLIP) == 0) {
		differential->type = DIFF_LIMITED_SLIP; 
	} else if (strcmp(type, VAL_DIFF_VISCOUS_COUPLER) == 0) {
		differential->type = DIFF_VISCOUS_COUPLER;
	} else if (strcmp(type, VAL_DIFF_SPOOL) == 0) {
		differential->type = DIFF_SPOOL;
	}  else if (strcmp(type, VAL_DIFF_FREE) == 0) {
		differential->type = DIFF_FREE;
	} else {
		differential->type = DIFF_NONE; 
	}
		
	differential->feedBack.I = differential->I * differential->ratio * differential->ratio +
		(differential->inAxis[0]->I + differential->inAxis[1]->I) / differential->efficiency;
}




static void updateSpool(tCar *car, tDifferential *differential, int first)
{
	tdble	DrTq;
	tdble	ndot;
	tdble	spinVel;
	tdble	BrTq;
	tdble	engineReaction;
	tdble	I;
	tdble	inTq, brkTq;
	
	DrTq = differential->in.Tq;
	
	I = differential->outAxis[0]->I + differential->outAxis[1]->I;
	inTq = differential->inAxis[0]->Tq + differential->inAxis[1]->Tq;
	brkTq = differential->inAxis[0]->brkTq + differential->inAxis[1]->brkTq;
	
	ndot = SimDeltaTime * (DrTq - inTq) / I;
	spinVel = differential->inAxis[0]->spinVel + ndot;
	
	BrTq = - SIGN(spinVel) * brkTq;
	ndot = SimDeltaTime * BrTq / I;
	
	if (((ndot * spinVel) < 0.0) && (fabs(ndot) > fabs(spinVel))) {
		ndot = -spinVel;
	}
	if ((spinVel == 0.0) && (ndot < 0.0)) ndot = 0;
	
	spinVel += ndot;
	if (first) {
		engineReaction = SimEngineUpdateRpm(car, spinVel);
		if (engineReaction != 0.0) {
			spinVel = engineReaction;
		}
	}
	differential->outAxis[0]->spinVel = differential->outAxis[1]->spinVel = spinVel;
	
	differential->outAxis[0]->Tq = (differential->outAxis[0]->spinVel - differential->inAxis[0]->spinVel) / SimDeltaTime * differential->outAxis[0]->I;
	differential->outAxis[1]->Tq = (differential->outAxis[1]->spinVel - differential->inAxis[1]->spinVel) / SimDeltaTime * differential->outAxis[1]->I;
}


void 
SimDifferentialUpdate(tCar *car, tDifferential *differential, int first)
{
	tdble	DrTq, DrTq0, DrTq1;
	tdble	ndot0, ndot1;
	tdble	spinVel0, spinVel1;
	tdble	inTq0, inTq1;
	tdble	spdRatio, spdRatioMax;
	tdble	deltaSpd, deltaTq;
	tdble	BrTq;
	tdble	engineReaction;
	tdble	meanv;
	
	if (differential->type == DIFF_SPOOL) {
		updateSpool(car, differential, first);
		return;
	}
	
	DrTq = differential->in.Tq;
	
	spinVel0 = differential->inAxis[0]->spinVel;
	spinVel1 = differential->inAxis[1]->spinVel;
	
	inTq0 = differential->inAxis[0]->Tq;
	inTq1 = differential->inAxis[1]->Tq;
	
	spdRatio = fabs(spinVel0 + spinVel1);
	if (spdRatio != 0) {
		spdRatio = fabs(spinVel0 - spinVel1) / spdRatio;
		
		switch (differential->type) {
		case DIFF_FREE:
			// I would think that the following is what a FREE
			// differential should look like, with both wheels
			// independent and linked through a spider gear.
			//
			// The reaction from each wheel is transmitted back to the
			// spider gear. If both wheels react equally, then the
			// spider gear does not turn. If one of the wheel is
			// immobile, so that DrTq/2=inTq0 for example, then the
			// reaction does not act against the drivetrain, but since
			// the spider gear can turn freely, it acts on the other wheel.
			// 
			// This system is equivalent to a rotating gear attached
			// in between two parallel surfaces, with DrTq being
			// equivalent to a force acting in the center of the
			// gear. If one surface is fixed, only the other surface
			// moves and all the force is 'transferred' to the moving
			// surface. Or, the way I like to think of it, the
			// immobile surface reacts with an equal and opposite
			// force[1] that cancels DrTq/2 exactly and which is
			// transmitted directly with the rotating gear to the
			// other, free, surface.
			//
			//
			// A lot of explanation for 3 lines of code..  TODO: Check
			// what bias would mean in such a system. Would it be
			// implemented between the spider and the wheels?  Or
			// between the spider and the drivetrain? If the latter
			// then it meanst the spider would always be turning, even
			// under an even load. I think in this case it is safest
			// to ignore it completely because it is frequently used
			// in cars with just FWD or RWD, and very frequently in
			// just the front part of 4WD cars, while the default
			// differential bias setting is 0.1...
			//
			// [1] For an object to remain at rest, all forces acting
			// on it must sum to 0.
				
			{
				float spiderTq = inTq1 - inTq0;
				DrTq0 = DrTq*0.5f + spiderTq;
				DrTq1 = DrTq*0.5f - spiderTq;
			}
			break;
		case DIFF_LIMITED_SLIP:
			if (DrTq > differential->lockInputTq) {
				updateSpool(car, differential, first);
				return;
			}
			spdRatioMax = differential->dSlipMax - DrTq * differential->dSlipMax / differential->lockInputTq;
			if (spdRatio > spdRatioMax) {
				deltaSpd = (spdRatio - spdRatioMax) * fabs(spinVel0 + spinVel1) / 2.0;
				if (spinVel0 > spinVel1) {
					spinVel0 -= deltaSpd;
					spinVel1 += deltaSpd;
				} else {
					spinVel0 += deltaSpd;
					spinVel1 -= deltaSpd;
				}
			}
			if (spinVel0 > spinVel1) {
				DrTq1 = DrTq * (0.5 + differential->bias);
				DrTq0 = DrTq * (0.5 - differential->bias);
			} else {
				DrTq1 = DrTq * (0.5 - differential->bias);
				DrTq0 = DrTq * (0.5 + differential->bias);
			}
			break;
		case DIFF_VISCOUS_COUPLER:
			if (spinVel0 >= spinVel1) {
				DrTq0 = DrTq * differential->dTqMin;
				DrTq1 = DrTq * (1 - differential->dTqMin);
			} else {
				deltaTq = differential->dTqMin + (1.0 - exp(-fabs(differential->viscosity * spinVel0 - spinVel1))) /
									differential->viscomax * differential->dTqMax;
				DrTq0 = DrTq * deltaTq;
				DrTq1 = DrTq * (1 - deltaTq);
			}
		
			break;
		default: /* NONE ? */
			DrTq0 = DrTq1 = 0;
			break;
		}
	} else {
		DrTq0 = DrTq / 2.0;
		DrTq1 = DrTq / 2.0;
	}
	
	ndot0 = SimDeltaTime * (DrTq0 - inTq0) / differential->outAxis[0]->I;
	spinVel0 += ndot0;
	ndot1 = SimDeltaTime * (DrTq1 - inTq1) / differential->outAxis[1]->I;
	spinVel1 += ndot1;

	BrTq = - SIGN(spinVel0) * differential->inAxis[0]->brkTq;
	ndot0 = SimDeltaTime * BrTq / differential->outAxis[0]->I;
	if (((ndot0 * spinVel0) < 0.0) && (fabs(ndot0) > fabs(spinVel0))) {
		ndot0 = -spinVel0;
	}
	if ((spinVel0 == 0.0) && (ndot0 < 0.0)) ndot0 = 0;
	spinVel0 += ndot0;
	
	BrTq = - SIGN(spinVel1) * differential->inAxis[1]->brkTq;
	ndot1 = SimDeltaTime * BrTq / differential->outAxis[1]->I;
	if (((ndot1 * spinVel1) < 0.0) && (fabs(ndot1) > fabs(spinVel1))) {
		ndot1 = -spinVel1;
	}
	if ((spinVel1 == 0.0) && (ndot1 < 0.0)) ndot1 = 0;
	spinVel1 += ndot1;
	
	if (first) {
		meanv = (spinVel0 + spinVel1) / 2.0;
		engineReaction = SimEngineUpdateRpm(car, meanv);
		if (meanv != 0.0) {
			engineReaction = engineReaction / meanv;
			if (engineReaction != 0.0) {
				spinVel1 *= engineReaction;
				spinVel0 *= engineReaction;
			}
		}
	}
	
	differential->outAxis[0]->spinVel = spinVel0;
	differential->outAxis[1]->spinVel = spinVel1;
	
	differential->outAxis[0]->Tq = (differential->outAxis[0]->spinVel - differential->inAxis[0]->spinVel) / SimDeltaTime * differential->outAxis[0]->I;
	differential->outAxis[1]->Tq = (differential->outAxis[1]->spinVel - differential->inAxis[1]->spinVel) / SimDeltaTime * differential->outAxis[1]->I;	
}


