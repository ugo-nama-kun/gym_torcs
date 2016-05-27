/***************************************************************************

    file                 : car.cpp
    created              : Sun Mar 19 00:05:43 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: car.cpp,v 1.24.2.1 2008/11/09 17:50:22 berniw Exp $

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

const tdble aMax = 0.35f; /*  */

void
SimCarConfig(tCar *car)
{
	void	*hdle = car->params;
	tdble	k;
	tdble	w;
	tdble	gcfrl, gcrrl, gcfr;
	tdble	wf0, wr0;
	tdble	overallwidth;
	int		i;
	tCarElt	*carElt = car->carElt;
	
	car->dimension.x = GfParmGetNum(hdle, SECT_CAR, PRM_LEN, (char*)NULL, 4.7f);
	car->dimension.y = GfParmGetNum(hdle, SECT_CAR, PRM_WIDTH, (char*)NULL, 1.9f);
	overallwidth     = GfParmGetNum(hdle, SECT_CAR, PRM_OVERALLWIDTH, (char*)NULL, car->dimension.y);
	car->dimension.z = GfParmGetNum(hdle, SECT_CAR, PRM_HEIGHT, (char*)NULL, 1.2f);
	car->mass        = GfParmGetNum(hdle, SECT_CAR, PRM_MASS, (char*)NULL, 1500);
	car->Minv        = 1.0 / car->mass;
	gcfr             = GfParmGetNum(hdle, SECT_CAR, PRM_FRWEIGHTREP, (char*)NULL, .5);
	gcfrl            = GfParmGetNum(hdle, SECT_CAR, PRM_FRLWEIGHTREP, (char*)NULL, .5);
	gcrrl            = GfParmGetNum(hdle, SECT_CAR, PRM_RRLWEIGHTREP, (char*)NULL, .5);
	car->statGC.y    = - (gcfr * gcfrl + (1 - gcfr) * gcrrl) * car->dimension.y + car->dimension.y / 2.0;
	car->statGC.z    = GfParmGetNum(hdle, SECT_CAR, PRM_GCHEIGHT, (char*)NULL, .5);
	
	car->tank        = GfParmGetNum(hdle, SECT_CAR, PRM_TANK, (char*)NULL, 80);
	car->fuel        = GfParmGetNum(hdle, SECT_CAR, PRM_FUEL, (char*)NULL, 80);
	k                = GfParmGetNum(hdle, SECT_CAR, PRM_CENTR, (char*)NULL, 1.0);
	carElt->_drvPos_x = GfParmGetNum(hdle, SECT_DRIVER, PRM_XPOS, (char*)NULL, 0.0);
	carElt->_drvPos_y = GfParmGetNum(hdle, SECT_DRIVER, PRM_YPOS, (char*)NULL, 0.0);
	carElt->_drvPos_z = GfParmGetNum(hdle, SECT_DRIVER, PRM_ZPOS, (char*)NULL, 0.0);
	carElt->_bonnetPos_x = GfParmGetNum(hdle, SECT_BONNET, PRM_XPOS, (char*)NULL, carElt->_drvPos_x);
	carElt->_bonnetPos_y = GfParmGetNum(hdle, SECT_BONNET, PRM_YPOS, (char*)NULL, carElt->_drvPos_y);
	carElt->_bonnetPos_z = GfParmGetNum(hdle, SECT_BONNET, PRM_ZPOS, (char*)NULL, carElt->_drvPos_z);
	
	if (car->fuel > car->tank) {
		car->fuel = car->tank;
	}
	k = k * k;
	car->Iinv.x = 12.0 / (car->mass * (car->dimension.y * car->dimension.y + car->dimension.z * car->dimension.z));
	car->Iinv.y = 12.0 / (car->mass * (car->dimension.x * car->dimension.x + car->dimension.z * car->dimension.z));
	car->Iinv.z = 12.0 / (car->mass * (car->dimension.y * car->dimension.y + k * car->dimension.x * car->dimension.x));
	
	/* configure components */
	w = car->mass * G;
	
	wf0 = w * gcfr;
	wr0 = w * (1 - gcfr);
	
	car->wheel[FRNT_RGT].weight0 = wf0 * gcfrl;
	car->wheel[FRNT_LFT].weight0 = wf0 * (1 - gcfrl);
	car->wheel[REAR_RGT].weight0 = wr0 * gcrrl;
	car->wheel[REAR_LFT].weight0 = wr0 * (1 - gcrrl);
	
	for (i = 0; i < 2; i++) {
		SimAxleConfig(car, i);
	}
	
	for (i = 0; i < 4; i++) {
		SimWheelConfig(car, i); 
	}
	
	
	SimEngineConfig(car);
	SimTransmissionConfig(car);
	SimSteerConfig(car);
	SimBrakeSystemConfig(car);
	SimAeroConfig(car);
	for (i = 0; i < 2; i++) {
		SimWingConfig(car, i);
	}
	
	/* Set the origin to GC */
	car->wheelbase = car->wheeltrack = 0;
	car->statGC.x = car->wheel[FRNT_RGT].staticPos.x * gcfr + car->wheel[REAR_RGT].staticPos.x * (1 - gcfr);
	
	carElt->_dimension = car->dimension;
	carElt->_statGC = car->statGC;
	carElt->_tank = car->tank;
	for (i = 0; i < 4; i++) {
		carElt->priv.wheel[i].relPos = car->wheel[i].relPos;
	}
	
	for (i = 0; i < 4; i++) {
		car->wheel[i].staticPos.x -= car->statGC.x;
		car->wheel[i].staticPos.y -= car->statGC.y;
	}
	car->wheelbase = (car->wheel[FRNT_RGT].staticPos.x 
				+ car->wheel[FRNT_LFT].staticPos.x
				- car->wheel[REAR_RGT].staticPos.x
				- car->wheel[REAR_LFT].staticPos.x) / 2.0;
	car->wheeltrack = (-car->wheel[REAR_LFT].staticPos.y 
				- car->wheel[FRNT_LFT].staticPos.y
				+ car->wheel[FRNT_RGT].staticPos.y
				+ car->wheel[REAR_RGT].staticPos.y) / 2.0;
	
	/* set corners pos */
	car->corner[FRNT_RGT].pos.x = car->dimension.x * .5 - car->statGC.x;
	car->corner[FRNT_RGT].pos.y = - overallwidth * .5 - car->statGC.y;
	car->corner[FRNT_RGT].pos.z = 0;
	
	car->corner[FRNT_LFT].pos.x = car->dimension.x * .5 - car->statGC.x;
	car->corner[FRNT_LFT].pos.y = overallwidth * .5 - car->statGC.y;
	car->corner[FRNT_LFT].pos.z = 0;
	
	car->corner[REAR_RGT].pos.x = - car->dimension.x * .5 - car->statGC.x;
	car->corner[REAR_RGT].pos.y = - overallwidth * .5 - car->statGC.y;
	car->corner[REAR_RGT].pos.z = 0;
	
	car->corner[REAR_LFT].pos.x = - car->dimension.x * .5 - car->statGC.x;
	car->corner[REAR_LFT].pos.y = overallwidth * .5 - car->statGC.y;
	car->corner[REAR_LFT].pos.z = 0;
}


static void
SimCarUpdateForces(tCar *car)
{
	tForces	F;
	int		i;
	tdble	m, w, minv;
	tdble	SinTheta;
	tdble	Cosz, Sinz;
	tdble	v, R, Rv, Rm, Rx, Ry;
	
	Cosz = car->Cosz = cos(car->DynGCg.pos.az);
	Sinz = car->Sinz = sin(car->DynGCg.pos.az);
	
	car->preDynGC = car->DynGCg;
	
	/* total mass */
	m = car->mass + car->fuel;
	minv = 1.0 / m;
	w = -m * G;
	
	/* Weight */
	SinTheta = (-car->wheel[FRNT_RGT].zRoad - car->wheel[FRNT_LFT].zRoad
		+ car->wheel[REAR_RGT].zRoad + car->wheel[REAR_LFT].zRoad) / (2.0 * car->wheelbase);
	F.F.x = -w * SinTheta;
	SinTheta = (-car->wheel[FRNT_RGT].zRoad - car->wheel[REAR_RGT].zRoad
		+ car->wheel[FRNT_LFT].zRoad + car->wheel[REAR_LFT].zRoad) / (2.0 * car->wheeltrack);
	F.F.y = -w * SinTheta;
	F.F.z = w; /* not 3D */
	F.M.x = F.M.y = F.M.z = 0;
	
	/* Wheels */
	for (i = 0; i < 4; i++) {
		/* forces */
		F.F.x += car->wheel[i].forces.x;
		F.F.y += car->wheel[i].forces.y;
		F.F.z += car->wheel[i].forces.z;
		/* moments */
		F.M.x += car->wheel[i].forces.z * car->wheel[i].staticPos.y +
			car->wheel[i].forces.y * car->wheel[i].rollCenter;
			// Eventually TODO: activate fix below and make all cars/robots fit.
			//car->wheel[i].forces.y * (car->statGC.z + car->wheel[i].rideHeight);
		F.M.y -= car->wheel[i].forces.z * car->wheel[i].staticPos.x +
			car->wheel[i].forces.x * (car->statGC.z + car->wheel[i].rideHeight);
		F.M.z += -car->wheel[i].forces.x * car->wheel[i].staticPos.y +
			car->wheel[i].forces.y * car->wheel[i].staticPos.x;
	}
	
	/* Aero Drag */
	F.F.x += car->aero.drag;
	
	/* Wings & Aero Downforce */
	for (i = 0; i < 2; i++) {
	/* forces */
	F.F.z += car->wing[i].forces.z + car->aero.lift[i];
	F.F.x += car->wing[i].forces.x;
	/* moments */
	F.M.y -= (car->wing[i].forces.z + car->aero.lift[i]) * car->wing[i].staticPos.x +
		car->wing[i].forces.x * car->wing[i].staticPos.z;
	}
	
	/* Rolling Resistance */
	v = sqrt(car->DynGCg.vel.x * car->DynGCg.vel.x + car->DynGCg.vel.y * car->DynGCg.vel.y);
	R = 0;
	for (i = 0; i < 4; i++) {
		R += car->wheel[i].rollRes;
	}
	if (v > 0.00001) {
		Rv = R / v;
		if ((Rv * minv * SimDeltaTime) > v) {
			Rv = v * m / SimDeltaTime;
		}
	} else {
		Rv = 0;
	}
	Rx = Rv * car->DynGCg.vel.x;
	Ry = Rv * car->DynGCg.vel.y;
	
	if ((R * car->wheelbase / 2.0 * car->Iinv.z) > fabs(car->DynGCg.vel.az)) {
		Rm = car->DynGCg.vel.az / car->Iinv.z;
	} else {
		Rm = SIGN(car->DynGCg.vel.az) * R * car->wheelbase / 2.0;
	}
	
	/* compute accelerations */
	car->DynGC.acc.x = F.F.x * minv;
	car->DynGC.acc.y = F.F.y * minv;
	car->DynGC.acc.z = F.F.z * minv;
	
	car->DynGCg.acc.x = (F.F.x * Cosz - F.F.y * Sinz - Rx) * minv;
	car->DynGCg.acc.y = (F.F.x * Sinz + F.F.y * Cosz - Ry) * minv;
	car->DynGCg.acc.z = car->DynGC.acc.z;
	
	car->DynGCg.acc.ax = car->DynGC.acc.ax = F.M.x * car->Iinv.x;
	car->DynGCg.acc.ay = car->DynGC.acc.ay = F.M.y * car->Iinv.y;
	car->DynGCg.acc.az = car->DynGC.acc.az = (F.M.z - Rm) * car->Iinv.z;
}

static void
SimCarUpdateSpeed(tCar *car)
{
	tdble	Cosz, Sinz;
	tdble	mass;
	
	mass = car->mass + car->fuel;
		
	Cosz = car->Cosz;
	Sinz = car->Sinz;
	
	car->DynGCg.vel.x += car->DynGCg.acc.x * SimDeltaTime;
	car->DynGCg.vel.y += car->DynGCg.acc.y * SimDeltaTime;
	car->DynGCg.vel.z += car->DynGCg.acc.z * SimDeltaTime;

	car->DynGCg.vel.ax += car->DynGCg.acc.ax * SimDeltaTime;
	car->DynGCg.vel.ay += car->DynGCg.acc.ay * SimDeltaTime;
	car->DynGCg.vel.az += car->DynGCg.acc.az * SimDeltaTime;
	
	/* spin limitation */
	if (fabs(car->DynGCg.vel.az) > 9.0) {
		car->DynGCg.vel.az = SIGN(car->DynGCg.vel.az) * 9.0;
	}
		
	car->DynGC.vel.ax = car->DynGCg.vel.ax;
	car->DynGC.vel.ay = car->DynGCg.vel.ay;
	car->DynGC.vel.az = car->DynGCg.vel.az;
	
	car->DynGC.vel.x = car->DynGCg.vel.x * Cosz + car->DynGCg.vel.y * Sinz;
	car->DynGC.vel.y = -car->DynGCg.vel.x * Sinz + car->DynGCg.vel.y * Cosz;
	car->DynGC.vel.z = car->DynGCg.vel.z;
}

void
SimCarUpdateWheelPos(tCar *car)
{
	int i;
	tdble vx;
	tdble vy;
	tdble Cosz, Sinz;
	
	Cosz = car->Cosz;
	Sinz = car->Sinz;
	vx = car->DynGC.vel.x;
	vy = car->DynGC.vel.y;
	
	/* Wheels data */
	for (i = 0; i < 4; i++) {
		tdble x = car->wheel[i].staticPos.x;
		tdble y = car->wheel[i].staticPos.y;
		tdble dx = x * Cosz - y * Sinz;
		tdble dy = x * Sinz + y * Cosz;
		
		car->wheel[i].pos.x = car->DynGCg.pos.x + dx;
		car->wheel[i].pos.y = car->DynGCg.pos.y + dy;
		car->wheel[i].pos.z = car->DynGCg.pos.z - car->statGC.z - x * sin(car->DynGCg.pos.ay) + y * sin(car->DynGCg.pos.ax);
		
		car->wheel[i].bodyVel.x = vx - car->DynGC.vel.az * y;
		car->wheel[i].bodyVel.y = vy + car->DynGC.vel.az * x;
	}
}

static void
SimCarUpdatePos(tCar *car)
{
	tdble vx, vy;
	tdble accx, accy;
	
	vx = car->DynGCg.vel.x;
	vy = car->DynGCg.vel.y;
	
	accx = car->DynGCg.acc.x;
	accy = car->DynGCg.acc.y;
	
	car->DynGCg.pos.x += vx * SimDeltaTime;
	car->DynGCg.pos.y += vy * SimDeltaTime;
	car->DynGCg.pos.z += car->DynGCg.vel.z * SimDeltaTime;
	
	car->DynGCg.pos.ax += car->DynGCg.vel.ax * SimDeltaTime;
	car->DynGCg.pos.ay += car->DynGCg.vel.ay * SimDeltaTime;
	car->DynGCg.pos.az += car->DynGCg.vel.az * SimDeltaTime;
		
	NORM_PI_PI(car->DynGCg.pos.az);
	
	if (car->DynGCg.pos.ax > aMax) car->DynGCg.pos.ax = aMax;
	if (car->DynGCg.pos.ax < -aMax) car->DynGCg.pos.ax = -aMax;
	if (car->DynGCg.pos.ay > aMax) car->DynGCg.pos.ay = aMax;
	if (car->DynGCg.pos.ay < -aMax) car->DynGCg.pos.ay = -aMax;
	
	car->DynGC.pos.x = car->DynGCg.pos.x;
	car->DynGC.pos.y = car->DynGCg.pos.y;
	car->DynGC.pos.z = car->DynGCg.pos.z;
	
	car->DynGC.pos.ax = car->DynGCg.pos.ax;
	car->DynGC.pos.ay = car->DynGCg.pos.ay;
	car->DynGC.pos.az = car->DynGCg.pos.az;
	
	RtTrackGlobal2Local(car->trkPos.seg, car->DynGCg.pos.x, car->DynGCg.pos.y, &(car->trkPos), TR_LPOS_MAIN);
}

static void
SimCarUpdateCornerPos(tCar *car)
{
	tdble Cosz = car->Cosz;
	tdble Sinz = car->Sinz;
	tdble vx = car->DynGCg.vel.x;
	tdble vy = car->DynGCg.vel.y;
	int i;
	
	for (i = 0; i < 4; i++) {
		tdble x = car->corner[i].pos.x + car->statGC.x;
		tdble y = car->corner[i].pos.y + car->statGC.y;
		tdble dx = x * Cosz - y * Sinz;
		tdble dy = x * Sinz + y * Cosz;
		
		car->corner[i].pos.ax = car->DynGCg.pos.x + dx;
		car->corner[i].pos.ay = car->DynGCg.pos.y + dy;
		/*car->corner[i].pos.az = car->DynGC.pos.z - car->statGC.z + x * sin(car->DynGC.pos.ay) + y * sin(car->DynGC.pos.ax);*/
		
		/* add the body rotation to the wheel        */
		/* the speed is vel.az * r                   */
		/* where r = sqrt(x*x + y*y)                 */
		/* the tangent vector is -y / r and x / r    */
		// compute corner velocity at local frame
		car->corner[i].vel.ax = - car->DynGC.vel.az * y;
		car->corner[i].vel.ay = car->DynGC.vel.az * x;
		
		// rotate to global and add global center of mass velocity
		// note: global to local.
		car->corner[i].vel.x = vx
			+ car->corner[i].vel.ax * Cosz - car->corner[i].vel.ay * Sinz;
		car->corner[i].vel.y = vy
			+ car->corner[i].vel.ax * Sinz + car->corner[i].vel.ay * Cosz;
		
		// add local center of mass velocity
		car->corner[i].vel.ax += car->DynGC.vel.x;
		car->corner[i].vel.ay += car->DynGC.vel.y;
	}
}

void 
SimTelemetryOut(tCar *car)
{
	int i;
	tdble Fzf, Fzr;
	
	printf("-----------------------------\nCar: %d %s ---\n", car->carElt->index, car->carElt->_name);
	printf("Seg: %d (%s)  Ts:%f  Tr:%f\n",
		car->trkPos.seg->id, car->trkPos.seg->name, car->trkPos.toStart, car->trkPos.toRight);
	printf("---\nMx: %f  My: %f  Mz: %f (N/m)\n", car->DynGC.acc.ax, car->DynGC.acc.ay, car->DynGC.acc.az);
	printf("Wx: %f  Wy: %f  Wz: %f (rad/s)\n", car->DynGC.vel.ax, car->DynGC.vel.ay, car->DynGC.vel.az);
	printf("Ax: %f  Ay: %f  Az: %f (rad)\n", car->DynGCg.pos.ax, car->DynGCg.pos.ay, car->DynGCg.pos.az);
	printf("---\nAx: %f  Ay: %f  Az: %f (Gs)\n", car->DynGC.acc.x/9.81, car->DynGC.acc.y/9.81, car->DynGC.acc.z/9.81);
	printf("Vx: %f  Vy: %f  Vz: %f (m/s)\n", car->DynGC.vel.x, car->DynGC.vel.y, car->DynGC.vel.z);
	printf("Px: %f  Py: %f  Pz: %f (m)\n---\n", car->DynGCg.pos.x, car->DynGCg.pos.y, car->DynGCg.pos.z);
	printf("As: %f\n---\n", sqrt(car->airSpeed2));
	for (i = 0; i < 4; i++) {
	printf("wheel %d - RH:%f susp:%f zr:%.2f ", i, car->wheel[i].rideHeight, car->wheel[i].susp.x, car->wheel[i].zRoad);
	printf("sx:%f sa:%f w:%f ", car->wheel[i].sx, car->wheel[i].sa, car->wheel[i].spinVel);
	printf("fx:%f fy:%f fz:%f\n", car->wheel[i].forces.x, car->wheel[i].forces.y, car->wheel[i].forces.z);
	}
	Fzf = (car->aero.lift[0] + car->wing[0].forces.z) / 9.81;
	Fzr = (car->aero.lift[1] + car->wing[1].forces.z) / 9.81;
	printf("Aero Fx:%f Fz:%f Fzf=%f Fzr=%f ratio=%f\n", car->aero.drag / 9.81, Fzf + Fzr,
		Fzf, Fzr, (Fzf + Fzr) / (car->aero.drag + 0.1) * 9.81);
	
}

void
SimCarUpdate(tCar *car, tSituation * /* s */)
{
	SimCarUpdateForces(car);
	CHECK(car);
	SimCarUpdateSpeed(car);
	CHECK(car);
	SimCarUpdateCornerPos(car);
	CHECK(car);
	SimCarUpdatePos(car);
	CHECK(car);
	SimCarCollideZ(car);
	CHECK(car);
	SimCarCollideXYScene(car);
	CHECK(car);
}

void
SimCarUpdate2(tCar *car, tSituation * /* s */)
{
    if (SimTelemetry == car->carElt->index) SimTelemetryOut(car);
}

