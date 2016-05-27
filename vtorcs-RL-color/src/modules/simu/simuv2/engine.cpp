/***************************************************************************

    file                 : engine.cpp
    created              : Sun Mar 19 00:06:55 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: engine.cpp,v 1.25 2007/10/30 21:45:23 berniw Exp $

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

void
SimEngineConfig(tCar *car)
{
	void	*hdle = car->params;
	int		i;
	tdble	maxTq;
	tdble	rpmMaxTq = 0;
	char	idx[64];
	tEngineCurveElem *data;
	struct tEdesc {
		tdble rpm;
		tdble tq;
	} *edesc;
	
	
	car->engine.revsLimiter = GfParmGetNum(hdle, SECT_ENGINE, PRM_REVSLIM, (char*)NULL, 800);
	car->carElt->_enginerpmRedLine = car->engine.revsLimiter;
	car->engine.revsMax     = GfParmGetNum(hdle, SECT_ENGINE, PRM_REVSMAX, (char*)NULL, 1000);
	car->carElt->_enginerpmMax = car->engine.revsMax;
	car->engine.tickover    = GfParmGetNum(hdle, SECT_ENGINE, PRM_TICKOVER, (char*)NULL, 150);
	car->engine.I           = GfParmGetNum(hdle, SECT_ENGINE, PRM_INERTIA, (char*)NULL, 0.2423f);
	if (getFuelConsumption())
		car->engine.fuelcons    = GfParmGetNum(hdle, SECT_ENGINE, PRM_FUELCONS, (char*)NULL, 0.0622f);
	else
		car->engine.fuelcons = 0.0;
	car->engine.brakeCoeff  = GfParmGetNum(hdle, SECT_ENGINE, PRM_ENGBRKCOEFF, (char*)NULL, 0.33f);
	car->engine.exhaust_pressure = 0.0f;
	car->engine.exhaust_refract = 0.1f;
	
	
	sprintf(idx, "%s/%s", SECT_ENGINE, ARR_DATAPTS);
	car->engine.curve.nbPts = GfParmGetEltNb(hdle, idx);
	edesc = (struct tEdesc*)malloc((car->engine.curve.nbPts + 1) * sizeof(struct tEdesc));
	
	for (i = 0; i < car->engine.curve.nbPts; i++) {
		sprintf(idx, "%s/%s/%d", SECT_ENGINE, ARR_DATAPTS, i+1);
		edesc[i].rpm = GfParmGetNum(hdle, idx, PRM_RPM, (char*)NULL, car->engine.revsMax);
		edesc[i].tq  = GfParmGetNum(hdle, idx, PRM_TQ, (char*)NULL, 0);
	}
	edesc[i].rpm = edesc[i - 1].rpm;
	edesc[i].tq  = edesc[i - 1].tq;
	
	maxTq = 0;
	car->engine.curve.maxPw = 0;
	car->engine.curve.data = (tEngineCurveElem *)malloc(car->engine.curve.nbPts * sizeof(tEngineCurveElem));
	for(i = 0; i < car->engine.curve.nbPts; i++) {
		data = &(car->engine.curve.data[i]);
		
		data->rads = edesc[i+1].rpm;
		if ((data->rads>=car->engine.tickover)
			&& (edesc[i+1].tq > maxTq)
			&& (data->rads < car->engine.revsLimiter)) {
			maxTq = edesc[i+1].tq;
			rpmMaxTq = data->rads;
		}
		if ((data->rads>=car->engine.tickover)
			&& (data->rads * edesc[i+1].tq > car->engine.curve.maxPw)
			&& (data->rads < car->engine.revsLimiter)) {
			car->engine.curve.TqAtMaxPw = edesc[i+1].tq;
			car->engine.curve.maxPw = data->rads * edesc[i+1].tq;
			car->engine.curve.rpmMaxPw = data->rads;
		}
	
		data->a = (edesc[i+1].tq - edesc[i].tq) / (edesc[i+1].rpm - edesc[i].rpm);
		data->b = edesc[i].tq - data->a * edesc[i].rpm;
	}
	
	car->engine.curve.maxTq = maxTq;
	car->carElt->_engineMaxTq = maxTq;
	car->carElt->_enginerpmMaxTq = rpmMaxTq;
	car->carElt->_engineMaxPw = car->engine.curve.maxPw;
	car->carElt->_enginerpmMaxPw = car->engine.curve.rpmMaxPw;
	
	car->engine.rads = car->engine.tickover;
	
	free(edesc);
}

/* Update torque output with engine rpm and accelerator command */
void
SimEngineUpdateTq(tCar *car)
{
	int i;
	tEngine	*engine = &(car->engine);
	tEngineCurve *curve = &(engine->curve);
	
	if ((car->fuel <= 0.0f) || (car->carElt->_state & (RM_CAR_STATE_BROKEN | RM_CAR_STATE_ELIMINATED))) {
		engine->rads = 0;
		engine->Tq = 0;
		return;
	}
	
	if (engine->rads > engine->revsLimiter) {
		engine->rads = engine->revsLimiter;
		engine->Tq = 0;
	} else {
		for (i = 0; i < car->engine.curve.nbPts; i++) {
			if (engine->rads < curve->data[i].rads) {
				tdble Tmax = engine->rads * curve->data[i].a + curve->data[i].b;
				tdble EngBrkK = engine->brakeCoeff * (engine->rads - engine->tickover) / (engine->revsMax - engine->tickover);
		
				engine->Tq = Tmax * (car->ctrl->accelCmd * (1.0 + EngBrkK) - EngBrkK);
				// Engines comsume always fuel (needed for keeping the process running, inner friction,
				// braking, accelerating pistons, etc.
				// TODO: Evaluate if it is worth implementing it.
				car->fuel -= fabs(engine->Tq) * engine->rads * engine->fuelcons * 0.0000001 * SimDeltaTime;
				if (car->fuel <= 0.0) {
					car->fuel = 0.0;
				}
				return;
			}
		}
	} 
}

/*
 * Function
 *	SimEngineUpdateRpm
 *
 * Description
 *	update engine RPM with wheels RPM
 *
 * Parameters
 *	car and axle RPM
 *
 * Return
 *	axle rpm for wheel update
 *	0 if no wheel update
 */
tdble
SimEngineUpdateRpm(tCar *car, tdble axleRpm)
{
	tTransmission *trans = &(car->transmission);
	tClutch *clutch = &(trans->clutch);
	tEngine *engine = &(car->engine);
	float freerads;
	float transfer;
	
	
	if (car->fuel <= 0.0) {
		engine->rads = 0;
		clutch->state = CLUTCH_APPLIED;
		clutch->transferValue = 0.0;
		return 0.0;
	}
	
	freerads = engine->rads;
	freerads += engine->Tq / engine->I * SimDeltaTime;
	{
		tdble dp = engine->pressure;
		engine->pressure = engine->pressure*0.9f + 0.1f*engine->Tq;
		dp = (0.001f*fabs(engine->pressure - dp));
		dp = fabs(dp);
		tdble rth = urandom();
		if (dp > rth) {
			engine->exhaust_pressure += rth;
		}
		engine->exhaust_pressure *= 0.9f;
		car->carElt->priv.smoke += 5.0f*engine->exhaust_pressure;
		car->carElt->priv.smoke *= 0.99f;
	}
	
	if ((clutch->transferValue > 0.01f) && (trans->gearbox.gear)) {
		transfer = clutch->transferValue * clutch->transferValue * clutch->transferValue * clutch->transferValue;
	
		engine->rads = axleRpm * trans->curOverallRatio * transfer + freerads * (1.0f - transfer);
	
		if (engine->rads < engine->tickover) {
			engine->rads = engine->tickover;
		} else if (engine->rads > engine->revsMax) {
			engine->rads = engine->revsMax;
			return engine->revsMax / trans->curOverallRatio;
		}
	} else {
		engine->rads = freerads;
	}
	return 0.0;
}

void
SimEngineShutdown(tCar *car)
{
	free(car->engine.curve.data);
}
