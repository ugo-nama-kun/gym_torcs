/***************************************************************************

    file                 : mycar.cpp
    created              : Mon Oct 10 13:51:00 CET 2001
    copyright            : (C) 2001-2006 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: mycar.cpp,v 1.1.2.1 2008/05/31 11:27:04 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	this class holds some properties of the car
*/

#include "berniw.h"

const double MyCar::PATHERR = 0.5;					// If derror > PATHERR we take actions to come back to the path [m].
const double MyCar::CORRLEN = 30.0;					// CORRLEN * derror is the length of the correction [m].
const double MyCar::TURNTOL = 1.0;					// Tolerance for end backing up [m].
const double MyCar::TURNSPEED = 3.0;				// if speed lower than this you can back up [m/s].
const double MyCar::MARGIN = 0.2;					// Security margin from track border [m].
const double MyCar::STABLESPEED = 80.0;				// We brake currentspeed/stablespeed if car seems unstable [m/s].
const double MyCar::TIMETOCATCH = 2.0;				// When do we start thinking about overtaking [s].
const double MyCar::MINOVERTAKERANGE = 100.0;		// Minimum length for overtaking [m].
const double MyCar::OVERTAKERADIUS = 100.0;			// Min allowed radius to start overtaking [m].
const double MyCar::OVERTAKEDIST = 4.0;				// Planning distance of CG's while overtaking [m].
const double MyCar::OVERTAKEMINDIST = 1.7;			// Minimal distance between cars overtaking [m].
const double MyCar::OVERTAKEANGLE = 0.03;			// Minimum angle between cars [radians].
const double MyCar::MAXALLOWEDPITCH = 0.06;			// Max allowed angle between road and car [radians].
const double MyCar::FLYSPEED = 55.0;				// Above this speed anti flying system is activated [m/s].
const double MyCar::OVERLAPSTARTDIST = 70.0;		// Distance where we start to check the need to let pass the opponent.
const double MyCar::OVERLAPPASSDIST = 30.0;			// Distance smaller than that and waiting long enough -> let him pass.
const double MyCar::OVERLAPWAITTIME = 5.0;			// Minimal waiting time before we consider let him pass.
const double MyCar::LAPBACKTIMEPENALTY = -30.0;		// Penalty if i am able to "lap back" [s].
const double MyCar::TCL_SLIP = 2.0;					// [m/s] range [0..10].
const double MyCar::TCL_RANGE = 5.0;				// [m/s] range [0..10].
const double MyCar::SHIFT = 0.9;					// [-] (% of rpmredline).
const double MyCar::SHIFT_MARGIN = 4.0;				// [m/s].
const double MyCar::MAX_SPEED = 84.0;				// [m/s].
const double MyCar::MAX_FUEL_PER_METER = 0.0008;	// [liter/m] fuel consumtion.
const double MyCar::LOOKAHEAD_MAX_ERROR = 2.0;		// [m].
const double MyCar::STEER_D_CONTROLLER_GAIN = 0.1;	// [-] Output gain of steering d-controller.
const int MyCar::TEAM_DAMAGE_CHANGE_LEAD = 1500;	// When to change position in the team?

// TODO: simulation timestep dependence.
const double MyCar::LOOKAHEAD_FACTOR = 1.0/(3.0*TRACKRES); // [-].

const float MyCar::CLUTCH_SPEED = 5.0f;				// [m/s]
const float MyCar::CLUTCH_FULL_MAX_TIME = 2.0f;		// [s] Time to apply full clutch.
const float MyCar::FUEL_SAFETY_MARGIN = 0.15f;		// [kg] How much additional fuel in parts of maxfuelperlap for random errors.


MyCar::MyCar(TrackDesc* track, tCarElt* car, tSituation *situation)
{
    AEROMAGIC = GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_AMAGIC, (char*)NULL, 1.6f);
	CFRICTION = GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_FMAGIC, (char*)NULL, 1.0f);

	setCarPtr(car);
	initCGh();
	initCarGeometry();
	updatePos();
	updateDir();
	updateSpeedSqr();
	updateSpeed();

	// Damage and fuel status.
	lastfuel = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_FUEL, NULL, 100.0);
	undamaged = situation->_maxDammage;
	if (undamaged == 0) {
		undamaged = 10000;
	}
	MAXDAMMAGE = undamaged / 2;
	fuelperlap = 0.0;
	lastpitfuel = 0.0;

	// Set up some car properties.
	wheelbase = car->priv.wheel[FRNT_RGT].relPos.x - car->priv.wheel[REAR_RGT].relPos.x;
	wheeltrack = 2* fabs(car->priv.wheel[REAR_RGT].relPos.y);

	carmass = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 0.0);
	mass = carmass + lastfuel;

	// Which wheels are driven.
	char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
		drivetrain = DRWD;
	} else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
		drivetrain = DFWD;
	} else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
		drivetrain = D4WD;
	}

	updateCa();
	double cx = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*)NULL, 0.0);
	double frontarea = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*)NULL, 0.0);
	cw = 0.625*cx*frontarea;

	// Get PGAIN.
	STEER_P_CONTROLLER_GAIN = GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_STEERPGAIN, (char*)NULL, 0.02f);
	STEER_P_CONTROLLER_MAX = GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_STEERPGAIN_MAX, (char*)NULL, 0.1f);
	//printf("PC: %f, %f\n", STEER_P_CONTROLLER_GAIN, STEER_P_CONTROLLER_MAX);
	// Magic number for friction in pit lane, because the current code does not consider the pit surface.
	// TODO: Consider the pit surface properties.
	cgcorr_b = 0.46;

	pf = new Pathfinder(track, car, situation);
	currentsegid = destsegid = pf->getCurrentSegment(car);

	currentseg = track->getSegmentPtr(currentsegid);
	destseg = track->getSegmentPtr(destsegid);
	currentpathsegid = currentsegid;
	destpathsegid = destsegid;
	dynpath = pf->getPath();

	turnaround = 0.0;
    tr_mode = 0;
	accel = 1.0;
	fuelchecked = false;
	startmode = true;
	trtime = 0.0;
	derror = 0.0;
	clutchtime = 0.0;

	// DIST; MAXRELAX; MAXANGLE; ACCELINC; SPEEDSQRFACTOR; GCTIME; ACCELLIMIT; PATHERRFACTOR
	double ba[6][8] = {
		{1.2, 0.9, 25.0, 0.1, 1.2, 0.2, 1.0, 5.0},
		{1.2, 0.9, 20.0, 0.1, 1.1, 0.5, 1.0, 5.0},
		{1.2, 0.9, 15.0, 0.1, 1.0, 0.5, 1.0, 5.0},
		{1.3, 0.9, 15.0, 0.02, 0.98, 0.5, 1.0, 5.0},
		{1.6, 0.9, 15.0, 0.01, 0.95, 0.5, 1.0, 5.0},
		{1.2, 0.9, 45.0, 0.1, 1.0, 0.5, 1.0, 1.0}
	};

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 8; j++) {
			behaviour[i][j] = ba[i][j];
		}
	}

	loadBehaviour(NORMAL);

	// Init optimal trajectory.
	pf->plan(this, currentsegid);
}


MyCar::~MyCar()
{
	delete pf;
}


// Info for debugging
void MyCar::info(void)
{
	printf("wheelbase: %f\n", wheelbase);
	printf("wheeltrack: %f\n", wheeltrack);
	for (int i = 0; i < MAX_GEARS; i++) {
			printf("%d\t%f\n", i, getCarPtr()->_gearRatio[i]);
	}
	printf("Offset: %d\n", getCarPtr()->_gearOffset);
	printf("#gears: %d\n", getCarPtr()->_gearNb);
	printf("gear: %d\n", getCarPtr()->_gear);
	printf("steerlock: %f rad, %f\n", getCarPtr()->_steerLock, getCarPtr()->_steerLock * 180 / PI);
	printf("cgcorr_b: %f\n", cgcorr_b);
	printf("car index: %d\n", getCarPtr()->index);
	printf("race nb: %d\n", getCarPtr()->_raceNumber);
}


// Update data.
void MyCar::update(TrackDesc* track, tCarElt* car, tSituation *situation)
{
	updatePos();
	updateDir();
	updateSpeedSqr();
	updateSpeed();

	// Update currentsegment and destination segment id's.
	int searchrange = MAX((int) ceil(situation->deltaTime*speed+1.0) * 2, 4);
	currentsegid = destsegid = pf->getCurrentSegment(car, searchrange);
	double l = 0.0;

	while (l < 2.0 * wheelbase) {
		l = l + dynpath->getLength(destsegid);
		destsegid = (destsegid + 1 + pf->getnPathSeg()) % pf->getnPathSeg();
	}

	currentseg = track->getSegmentPtr(currentsegid);
	destseg = track->getSegmentPtr(destsegid);
	currentpathsegid = currentsegid;
	updateDError();
	int lookahead = (destsegid + (int) (MIN(LOOKAHEAD_MAX_ERROR,derror)*speed*LOOKAHEAD_FACTOR)) % pf->getnPathSeg();
	destpathsegid = lookahead;

	mass = carmass + car->priv.fuel;
	trtime += situation->deltaTime;
	deltapitch = MAX(-track->getSegmentPtr(currentsegid)->getKgamma() - me->_pitch, 0.0);
}


// Change the behaviour.
void MyCar::loadBehaviour(int id) {
	bmode = id;
	DIST = behaviour[id][0];
	MAXRELAX = behaviour[id][1];
	MAXANGLE = behaviour[id][2];
	ACCELINC = behaviour[id][3];
	SPEEDSQRFACTOR = behaviour[id][4];
	GCTIME = behaviour[id][5];
	ACCELLIMIT = behaviour[id][6];
	PATHERRFACTOR = behaviour[id][7];
}


// Compute approximate downforce coefficient.
void MyCar::updateCa()
{
	const char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	double rearwingarea = GfParmGetNum(me->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*)NULL, 0);
	double rearwingangle = GfParmGetNum(me->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*)NULL, 0);
	double wingca = 1.23*rearwingarea*sin(rearwingangle);
	double cl = GfParmGetNum(me->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*)NULL, 0.0) + GfParmGetNum(me->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*)NULL, 0.0);
	double h = 0.0;

	for (int i = 0; i < 4; i++) h += GfParmGetNum(me->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*)NULL, 0.20f);
	h*= 1.5; h = h*h; h = h*h; h = 2.0 * exp(-3.0*h);
	ca = AEROMAGIC*(h*cl + 4.0*wingca);
}


// Compute the slip velocity.
double MyCar::querySlipSpeed(tCarElt* car)
{
	double s;
	switch (drivetrain) {
		case DRWD:
			s = (car->_wheelSpinVel(REAR_RGT) +
				 car->_wheelSpinVel(REAR_LFT)) * car->_wheelRadius(REAR_LFT) / 2.0;
			break;
		case DFWD:
			s = (car->_wheelSpinVel(FRNT_RGT) +
				 car->_wheelSpinVel(FRNT_LFT)) * car->_wheelRadius(FRNT_LFT) / 2.0;
			break;
		case D4WD:
			s = ((car->_wheelSpinVel(REAR_RGT) +
				  car->_wheelSpinVel(REAR_LFT)) * car->_wheelRadius(REAR_LFT) +
				 (car->_wheelSpinVel(FRNT_RGT) +
				  car->_wheelSpinVel(FRNT_LFT)) * car->_wheelRadius(FRNT_LFT)) / 4.0;
			break;
		default:
			s = 0.0;
			break;
	}
	return s - car->_speed_x;
}



// Compute an acceleration value for a given speed.
double MyCar::queryAcceleration(tCarElt * car, double speed)
{
	double a, gr = car->_gearRatio[car->_gear + car->_gearOffset], rm = car->_enginerpmRedLine;
	switch (drivetrain) {
		case DRWD:
			a = speed / car->_wheelRadius(REAR_RGT) * gr / rm;
	        break;
		case DFWD:
			a = speed / car->_wheelRadius(FRNT_RGT) * gr / rm;
			break;
		case D4WD:
			a = speed / (car->_wheelRadius(REAR_RGT) + car->_wheelRadius(FRNT_RGT)) * 2.0 * gr / rm;
			break;
		default:
			a = 1.0;
			break;
	}
	if (a > 1.0) {
		return 1.0;
	} else {
		return a;
	}
}


void OtherCar::init(TrackDesc* itrack, tCarElt* car, tSituation *situation)
{
	track = itrack;
	dt = situation->deltaTime;
	setCarPtr(car);
	currentsegid = track->getCurrentSegment(car);

	initCGh();
	updatePos();
	updateDir();
	updateSpeedSqr();
	updateSpeed();
}


void OtherCar::update()
{
	updatePos();
	updateDir();
	updateSpeedSqr();
	updateSpeed();

    int searchrange = MAX((int) ceil(dt*speed+1.0) * 2, 4);
	currentsegid = track->getCurrentSegment(getCarPtr(), currentsegid, searchrange);
}


void MyCar::updateDError()
{
	derror = pf->distToPath(currentsegid, &currentpos);
	derrorsgn = (derror >= 0.0) ? 1.0 : -1.0;
	derror = fabs(derror);
}

void MyCar::initCarGeometry()
{
	CARWIDTH = me->_dimension_y;
	CARLEN = me->_dimension_x;
}


