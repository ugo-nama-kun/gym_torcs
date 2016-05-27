/***************************************************************************

    file                 : susp.cpp
    created              : Sun Mar 19 00:08:41 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: susp.cpp,v 1.10 2007/10/30 21:45:23 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include "sim.h"

/*
 * b2 and b3 calculus
 */
static void initDamper(tSuspension *susp)
{
	tDamper *damp;
	
	damp = &(susp->damper);	
	damp->bump.b2 = (damp->bump.C1 - damp->bump.C2) * damp->bump.v1 + damp->bump.b1;
	damp->rebound.b2 = (damp->rebound.C1 - damp->rebound.C2) * damp->rebound.v1 + damp->rebound.b1;
}




/*
 * get damper force
 */
static tdble damperForce(tSuspension *susp)
{
	tDamperDef *dampdef;
	tdble     f;
	tdble     av;
	tdble     v;

	v = susp->v;
	
	if (fabs(v) > 10.0f) {
		v = SIGN(v) * 10.0f;
	}
	
	if (v < 0.0f) {
		/* rebound */
		dampdef = &(susp->damper.rebound);
	} else {
		/* bump */
		dampdef = &(susp->damper.bump);
	}
	
	av = fabs(v);
	if (av < dampdef->v1) {
		f = (dampdef->C1 * av + dampdef->b1);
	} else {
		f = (dampdef->C2 * av + dampdef->b2);
	}
	
	f *= SIGN(v);
	
	return f;
}




/*
 * get spring force
 */
static tdble springForce(tSuspension *susp)
{
	tSpring *spring = &(susp->spring);
	tdble f;
	
	/* K is < 0 */
	f = spring->K * (susp->x - spring->x0) + spring->F0;
	if (f < 0.0f) {
		f = 0.0f;
	}
	
	return f;
}




void SimSuspCheckIn(tSuspension *susp)
{
	susp->state = 0;
	if (susp->x < susp->spring.packers) {
		susp->x = susp->spring.packers;
		susp->state = SIM_SUSP_COMP;
	}
	susp->x *= susp->spring.bellcrank;
	if (susp->x > susp->spring.xMax) {
		susp->x = susp->spring.xMax;
		susp->state = SIM_SUSP_EXT;
	}
}




void SimSuspUpdate(tSuspension *susp)
{
	susp->force = (springForce(susp) + damperForce(susp)) * susp->spring.bellcrank;
}




void SimSuspConfig(void *hdle, char *section, tSuspension *susp, tdble F0, tdble X0)
{
	susp->spring.K          = GfParmGetNum(hdle, section, PRM_SPR, (char*)NULL, 175000.0f);
	susp->spring.xMax       = GfParmGetNum(hdle, section, PRM_SUSPCOURSE, (char*)NULL, 0.5f);
	susp->spring.bellcrank  = GfParmGetNum(hdle, section, PRM_BELLCRANK, (char*)NULL, 1.0f);
	susp->spring.packers    = GfParmGetNum(hdle, section, PRM_PACKERS, (char*)NULL, 0.0f);
	susp->damper.bump.C1    = GfParmGetNum(hdle, section, PRM_SLOWBUMP, (char*)NULL, 0.0f);
	susp->damper.rebound.C1 = GfParmGetNum(hdle, section, PRM_SLOWREBOUND, (char*)NULL, 0.0f);
	susp->damper.bump.C2    = GfParmGetNum(hdle, section, PRM_FASTBUMP, (char*)NULL, 0.0f);
	susp->damper.rebound.C2 = GfParmGetNum(hdle, section, PRM_FASTREBOUND, (char*)NULL, 0.0f);
	
	susp->spring.x0 = susp->spring.bellcrank * X0;
	susp->spring.F0 = F0 / susp->spring.bellcrank;
	susp->spring.K = - susp->spring.K;
	susp->damper.bump.b1 = 0.0f;
	susp->damper.rebound.b1 = 0.0f;
	susp->damper.bump.v1 = 0.5f;
	susp->damper.rebound.v1 = 0.5f;
	
	initDamper(susp);
}

