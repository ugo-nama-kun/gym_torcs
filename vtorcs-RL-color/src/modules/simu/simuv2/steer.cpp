/***************************************************************************

    file                 : steer.cpp
    created              : Sun Mar 19 00:08:20 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: steer.cpp,v 1.8 2005/08/30 09:47:45 berniw Exp $

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
SimSteerConfig(tCar *car)
{
	void *hdle = car->params;

	car->steer.steerLock = GfParmGetNum(hdle, SECT_STEER, PRM_STEERLOCK, (char*)NULL, 0.43f);
	car->steer.maxSpeed  = GfParmGetNum(hdle, SECT_STEER, PRM_STEERSPD, (char*)NULL, 1.0f);
	car->carElt->_steerLock = car->steer.steerLock;
}


void
SimSteerUpdate(tCar *car)
{
	tdble steer, steer2;
	tdble stdelta;
	tdble tanSteer;

	/* input control */
	steer = car->ctrl->steer;
	steer *= car->steer.steerLock;
	stdelta = steer - car->steer.steer;

	if ((fabs(stdelta) / SimDeltaTime) > car->steer.maxSpeed) {
		steer = SIGN(stdelta) * car->steer.maxSpeed * SimDeltaTime + car->steer.steer;
	}

	car->steer.steer = steer;
	tanSteer = fabs(tan(steer));
	steer2 = atan2((car->wheelbase * tanSteer) , (car->wheelbase - tanSteer * car->wheeltrack));

	if (steer > 0) {
		car->wheel[FRNT_RGT].steer = steer2;
		car->wheel[FRNT_LFT].steer = steer;
	} else {
		car->wheel[FRNT_RGT].steer = steer;
		car->wheel[FRNT_LFT].steer = -steer2;
	}
}


