/***************************************************************************
 
    file                 : ObstacleSensors.cpp
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
//#define __DEBUG_OPP_SENS__


#include "ObstacleSensors.h"
#include "berniw.h"

void SingleObstacleSensor::init(tCarElt *car,double start_angle, double angle_covered,double range)
{
	this->car = car;
	sensor_range=range;
	sensor_angle_covered = angle_covered; 
	sensor_angle_start = start_angle;
	sensor_out=0;   //value ranges from 0 to 1: 0="No obstacle in sight" 1="Obstacle too close (collision)!"
		
}

void SingleObstacleSensor::setSingleObstacleSensor(double sens_value)
{
	sensor_out=sens_value;
}

ObstacleSensors::ObstacleSensors(int sensors_number, tTrack* track, tCarElt* car, tSituation *situation, int range )
{
	sensors_num=sensors_number;
	anglePerSensor = 360.0 / (double)sensors_number; 
	ocar = new OtherCar[situation->_ncars];
	obstacles_in_range = new Obstacle[situation->_ncars];
	myTrackDesc = new TrackDesc(track);

	all_obstacles = new Obstacle[situation->_ncars];
	
	sensors = new SingleObstacleSensor[sensors_number];

	for (int i = 0; i < sensors_number; i++) {
		sensors[i].init(car,i*anglePerSensor, anglePerSensor, range);

	}
	//holds the distances in segments
	for (int i = 0; i < situation->_ncars; i++) {
		ocar[i].init(myTrackDesc, situation->cars[i], situation);
	}

	myc = new MyCar(myTrackDesc, car, situation);

	sensorsRange= range;
	
}

ObstacleSensors::~ObstacleSensors()
{
	delete myc;
	delete myTrackDesc;
	delete [] ocar;
	delete [] sensors;
	delete [] obstacles_in_range;
	delete [] all_obstacles;
}

double ObstacleSensors::getObstacleSensorOut(int sensor_id)
{
	return sensors[sensor_id].getSingleObstacleSensorOut();
}

void ObstacleSensors::sensors_update(tSituation *situation)
{
	
	for (int i = 0; i < situation->_ncars && situation->_ncars != 1; i++) 
	{
		/* Update the internal structure of the opponents and of my car */
		ocar[i].update();
		myc->update(myTrackDesc, myc->getCarPtr(),situation);

		/* Compute the distance between my car and the opponent in the (x,y) space */
		all_obstacles[i].dist_horiz = myc->getCurrentPos()->x - ocar[i].getCurrentPos()->x;
		all_obstacles[i].dist_vert = myc->getCurrentPos()->y - ocar[i].getCurrentPos()->y;

		/* Compute distance as the crow flies */
		all_obstacles[i].dist = sqrt(pow(all_obstacles[i].dist_vert,2) + pow(all_obstacles[i].dist_horiz,2));
	}

	/* Remove the opponents out the sensor range (to avoid useless computation) */
	int j=0;  
	for(int i=0; i< situation->_ncars && situation->_ncars != 1; i++)
	{		
		if(all_obstacles[i].dist < sensorsRange && all_obstacles[i].dist > 0)
		{
			obstacles_in_range[j].dist_vert = all_obstacles[i].dist_vert;
			obstacles_in_range[j].dist_horiz = all_obstacles[i].dist_horiz;
			obstacles_in_range[j].dist = all_obstacles[i].dist;
			j++;
		}
	}

	/* Initizialization of sensors */
	for(int i=0; i<sensors_num; i++)
	{
		sensors[i].setSingleObstacleSensor( sensorsRange );
	}

	for(int i=0; i<j; i++) 
	{		

			
		/* Compute the absolute angle, i.e., the angle between car and opponent w.r.t. (x,y) axis */
		float deltax = -obstacles_in_range[i].dist_horiz;
		float deltay = -obstacles_in_range[i].dist_vert;

		double abs_angle;
		if (deltax >=0 && deltay >=0)
			abs_angle=atan(deltay/deltax)* 180 / PI;
		if (deltax <0 && deltay >=0)
			abs_angle=180 + atan(deltay/deltax)* 180 / PI;
		if (deltax <0 && deltay <0)
			abs_angle=-180+atan(deltay/deltax)* 180 / PI;			
		if (deltax >=0 && deltay <0)
			abs_angle=atan(deltay/deltax)* 180 / PI;	
		
		/* Angle between the car w.r.t. to the x axis*/
		double relative_angle = myc->getCarPtr()->_yaw*180/PI;

		/* Angle between the car and the opponent w.r.t. to the car axis (normalized between -180 and +180)*/
		double car_angle = (relative_angle - abs_angle);
		if (car_angle>=180)
			car_angle-=360;
		if (car_angle<=-180)
			car_angle+=360;

		/* Angle used by the opponent sensors, i.e. the car_angle mapped to [0,360] range*/
		double sensor_angle = 180 + car_angle;

		
#ifdef __DEBUG_OPP_SENS__
		printf("deltax: %f deltay: %f abs_angle: %f rel_angle: %f car_angle: %g sensor_angle: %f\n", 	
						deltax,deltay,abs_angle,relative_angle,car_angle, sensor_angle);


#endif
		/* Compute in which opponent sensor the opponent perceived falls */
		int position;  
		position = (int)( sensor_angle / (360/sensors_num) );

		if(position >= 0 && position < sensors_num )		
			if (obstacles_in_range[i].dist < sensors[position].getSingleObstacleSensorOut())
				sensors[position].setSingleObstacleSensor(obstacles_in_range[i].dist);
	}

	/* Saturation of sensors readings*/
	for(int i=0; i<sensors_num; i++)
	{
		if(sensors[i].getSingleObstacleSensorOut() >= sensorsRange)
			sensors[i].setSingleObstacleSensor(sensorsRange);
	}
}

void ObstacleSensors::printSensors()
{
	int tabsBefore;
	int tabsAfter;
	if(sensors_num % 2 == 0)
	{
		for(int curLevel=0; curLevel<sensors_num/2; curLevel++)
		{
			tabsBefore=sensors_num/2 - curLevel - 1;
			tabsAfter= curLevel*2+1;
			for(int i=0; i<tabsBefore; i++)
				printf("\t");
			printf("%.2f",sensors[sensors_num/2-1-curLevel].getSingleObstacleSensorOut());
			for(int i=0; i<tabsAfter; i++)
				printf("\t");
			printf("%.2f",sensors[sensors_num/2+curLevel].getSingleObstacleSensorOut());
			printf("\n");
		}
		
	}
	else
	{
		for(int curLevel=0; curLevel<(sensors_num/2+1); curLevel++)
		{
			tabsBefore=sensors_num/2 - curLevel ;
			tabsAfter= curLevel*2 ;
			for(int i=0; i<tabsBefore; i++)
				printf("\t");
			printf("%.2f",sensors[sensors_num/2-1-curLevel].getSingleObstacleSensorOut());
			if(curLevel>0)
			{
				for(int i=0; i<tabsAfter; i++)
					printf("\t");
				if(curLevel==0)
					printf(" ");
				printf("%.2f",sensors[sensors_num/2+curLevel].getSingleObstacleSensorOut());
			}
			printf("\n");
		}
	}
}
