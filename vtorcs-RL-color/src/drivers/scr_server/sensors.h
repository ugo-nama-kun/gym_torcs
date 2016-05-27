/***************************************************************************
 
    file                 : sensors.h
    copyright            : (C) 2007 Alessandro Prete
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *  
 ***************************************************************************/ 
#ifndef _SENSORS_H_
#define _SENSORS_H_

#include <stdio.h>
#include <math.h>
#include <car.h>
#include <robottools.h>
#include <raceman.h>

class SingleSensor {
	public:
		inline void init(CarElt *car) { this->car = car; }

		void setSingleSensor(float angle, float range);
		inline float getSingleSensorOut() { return sensor_out; }
		void update();

	protected:
		float sensor_calc_str(tTrackSeg *seg, float x, float y, float sensor_angle, float remaining_range);
		float sensor_calc_lft_rgt(tTrackSeg *seg, float x, float y, float sensor_angle, float remaining_range);
		
		bool check_max_circle_intersect(float angle, float radius_max, float radius_x, float y_dn, float y_up, float remaining_range, float &dist_returned);
		bool check_min_circle_intersect(float angle, float radius_min, float radius_x, float y_dn, float y_up, float remaining_range, float &dist_returned);
		bool check_up_border_intersect(float angle, float radius_min, float radius_x, float y_up, float remaining_range, float &dist_returned, float &angle_returned, float &x_returned);
		bool check_down_border_intersect(float angle, float radius_min, float radius_x, float y_dn, float remaining_range, float &dist_returned, float &angle_returned, float &x_returned);

		float sensor_range; //meters
		float sensor_angle; //radians
		float sensor_out;	//meters

		tCarElt *car;
};

class Sensors {
	public:
		Sensors(tCarElt *car, int sensors_number);
		~Sensors();

		//Params:
		//			id from 0 to sensors_number-1
		//			angle in degree, positive for left angle, negative for right angle
		//			range in meters
		void setSensor(int sensor_id, float angle, float range);

		//Return Value:
		//				sensor range if no boarder detected in the range
		//				border distance if boarder detected
		//note:	if the car reach the border the value is 0.
		//		if the car go behind the track border the value in < 0, don't consider it!
		float getSensorOut(int sensor_id);

		void sensors_update();

	protected:
		SingleSensor *sensor;
		int sensors_num;
};

#endif
