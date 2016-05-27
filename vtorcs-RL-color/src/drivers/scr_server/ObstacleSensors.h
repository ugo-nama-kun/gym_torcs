/***************************************************************************
 
    file                 : ObstacleSensors.h
    copyright            : (C) 2008 Lugi Cardamone, Daniele Loiacono, Matteo Verzola
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _SENSOBST_H_
#define _SENSOBST_H_

#include <stdio.h>
#include <math.h>
#include <car.h>
#include <robottools.h>
#include <raceman.h>

#include "spline.h"
#include "trackdesc.h"
#include "mycar.h"
#include "pathfinder.h"

class Obstacle
{
	public:
		double dist_vert;
		double dist_horiz;
		double dist;
};

class SingleObstacleSensor {
	public:
		void init(CarElt *car,double start_angle, double angle_covered,double range) ;

		void setSingleObstacleSensor(double sens_value);
		inline double getSingleObstacleSensorOut() { return sensor_out; }
		inline double getSingleObstacleSensorAngleStart() { return sensor_angle_start; }

	protected:
		

		double sensor_range; //meters
		double sensor_angle_start; 
		double sensor_angle_covered;
		double sensor_out;   //if = RANGE then no obstacle in range, else equals the distance from the obstacle in that direction

		tCarElt *car;
};

class ObstacleSensors {
	public:
		ObstacleSensors(int sensors_number, tTrack* track, tCarElt* car, tSituation *situation, int range );
		~ObstacleSensors();

		double getObstacleSensorOut(int sensor_id);

		void sensors_update(tSituation *situation);

	protected:
		SingleObstacleSensor *sensors;
		int sensors_num;
		OtherCar* ocar;			 //opponents that to sense
		Obstacle* obstacles_in_range;    //info on opponents to sense
		Obstacle* all_obstacles;	//info on ALL opponents, no matter what their position is!
		MyCar* myc;
		TrackDesc* myTrackDesc;
		int sensorsRange;
		double anglePerSensor;

	private:
		void printSensors();
};


#endif
