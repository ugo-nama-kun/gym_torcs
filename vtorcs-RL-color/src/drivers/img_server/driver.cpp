/***************************************************************************

    file                 : driver.cpp
    created              : Thu Dec 20 01:21:49 CET 2002
    copyright            : (C) 2002 Bernhard Wymann

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "driver.h"

const float Driver::MAX_UNSTUCK_ANGLE = 30.0/180.0*PI;		/* [radians] */
const float Driver::UNSTUCK_TIME_LIMIT = 2.0;				/* [s] */
const float Driver::MAX_UNSTUCK_SPEED = 5.0;   /* [m/s] */
const float Driver::MIN_UNSTUCK_DIST = 3.0;    /* [m] */
const float Driver::STEERING_CONSTANT = 1.0;    // steering aggressiveness constant factor


Driver::Driver(int index)
{
	INDEX = index;
}


/* Called for every track change or new race. */
void Driver::initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s)
{
	track = t;
	*carParmHandle = NULL;
}


/* Start a new race. */
void Driver::newRace(tCarElt* car, tSituation *s)
{
	MAX_UNSTUCK_COUNT = int(UNSTUCK_TIME_LIMIT/RCM_MAX_DT_ROBOTS);
	stuck = 0;
}


/* Drive during race. */
void Driver::drive(tCarElt* car, tSituation *s)
{
	update(car, s);
  isStuck(car) ? unstuck(car) : followcenter(car);

}


/* Set pitstop commands. */
int Driver::pitCommand(tCarElt* car, tSituation *s)
{
	return ROB_PIT_IM; /* return immediately */
}


/* End of the current race */
void Driver::endRace(tCarElt *car, tSituation *s)
{
}





///////////////////////////////////////
// DRIVING TOOLS
///////////////////////////////////////



/* Update my private data every timestep - and reset control */
void Driver::update(tCarElt* car, tSituation *s)
{
	trackangle = RtTrackSideTgAngleL(&(car->_trkPos));
	angle = trackangle - car->_yaw;
	NORM_PI_PI(angle);
  memset(&car->ctrl, 0, sizeof(tCarCtrl));
}


/* Check if I'm stuck */
bool Driver::isStuck(tCarElt* car)
{
    if (fabs(angle) > MAX_UNSTUCK_ANGLE &&
        car->_speed_x < MAX_UNSTUCK_SPEED &&
        fabs(car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) {
        if (stuck > MAX_UNSTUCK_COUNT && car->_trkPos.toMiddle*angle < 0.0) {
            return true;
        } else {
            stuck++;
            return false;
        }
    } else {
        stuck = 0;
        return false;
    }
}


///////////////////////////////////////
// GIUSE
///////////////////////////////////////


/* follow the center of the track */
void Driver::followcenter(tCarElt* car)
{
    angle -= STEERING_CONSTANT * car->_trkPos.toMiddle / car->_trkPos.seg->width;
    NORM_PI_PI(angle); // put the angle back in the range from -PI to PI
    
    car->ctrl.steer = angle / car->_steerLock;
    car->ctrl.gear = 2; // first gear
    car->ctrl.accelCmd = 0.8; // accelerator pedal
    car->ctrl.brakeCmd = 0.0; // no brakes

    printf("%d ", car->vision->img[1]);

//    printf("Lap: %d", car->_laps);
//    std::cout << "Lap: " << car->_lap << std::endl;
}


/* get the car unstuck */
void Driver::unstuck(tCarElt* car)
{
    car->ctrl.steer = -angle / car->_steerLock; // renormalize with steering
    car->ctrl.gear = -1; // reverse gear
    car->ctrl.accelCmd = 0.5; // 50% accelerator pedal
    car->ctrl.brakeCmd = 0.0; // no brakes
}





