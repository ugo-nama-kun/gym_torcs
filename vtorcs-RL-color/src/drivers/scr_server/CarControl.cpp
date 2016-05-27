/***************************************************************************
 
    file                 : CarControl.cpp
    copyright            : (C) 2007 Daniele Loiacono
 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "CarControl.h"

// meta-command value for race restart
int CarControl::META_RESTART=1;

CarControl::CarControl(float accel, float brake, int gear, float steer, float clutch, int focus, int meta)
{
	this->accel = accel;
	this->brake = brake;
	this->gear  = gear;
	this->steer = steer;
	this->clutch = clutch;
	this->focus = focus;
	this->meta = meta;
}

CarControl::CarControl(float accel, float brake, int gear, float steer, float clutch, int focus)
{
	this->accel = accel;
	this->brake = brake;
	this->gear  = gear;
	this->steer = steer;
	this->clutch = clutch;
	this->focus = focus;
	this->meta = 0;
}

CarControl::CarControl(string sensors)
{
        fromString(sensors);
}

string 
CarControl::toString()
{
	string str;
	
	str  = SimpleParser::stringify("accel", accel);
	str += SimpleParser::stringify("brake", brake);
	str += SimpleParser::stringify("gear",  gear);
	str += SimpleParser::stringify("steer", steer);
	str += SimpleParser::stringify("clutch", clutch);
	str += SimpleParser::stringify("focus",  focus);
	str += SimpleParser::stringify("meta", meta);
	
	return str;	
}

void 
CarControl::fromString(string sensors)
{
	if (SimpleParser::parse(sensors, "accel", accel)==false)
		accel=0.0;
	if (SimpleParser::parse(sensors, "brake", brake)==false)
		brake=0.0;
	if (SimpleParser::parse(sensors, "gear",  gear)==false)
		gear=1;
	if (SimpleParser::parse(sensors, "steer", steer)==false)
		steer=0.0;
	if (SimpleParser::parse(sensors, "clutch", clutch)==false)
			clutch=0.0;
	if (SimpleParser::parse(sensors, "meta", meta)==false)
		meta=0;
	if (SimpleParser::parse(sensors, "focus", focus)==false) //ML
		focus=0; //ML
	if (focus < -90 || focus > 90)//ML What to do with focus requests out of allowed range?
		focus=360;//ML A value of 360 is used for not requesting focus readings; -1 is returned as focus reading to the client
}

float 
CarControl::getAccel() const
{
        return this->accel;
};

void 
CarControl::setAccel (float accel)
{
        this->accel = accel;
};

float 
CarControl::getBrake() const
{
        return this->brake;
};

void 
CarControl::setBrake (float brake)
{
        this->brake = brake;
};

int
CarControl::getGear() const
{
        return this->gear;
};

void 
CarControl::setGear(int gear)
{
        this->gear = gear;
};

float 
CarControl::getSteer() const
{
        return this->steer;
};

void 
CarControl::setSteer(float steer)
{
        this->steer = steer;
};

int
CarControl::getMeta() const
{
        return this->meta;
};

void 
CarControl::setMeta(int meta)
{
        this->meta = meta;
};

float
CarControl::getClutch() const
{
       return clutch;
}

void
CarControl::setClutch(float clutch)
{
       this->clutch = clutch;
}

int
CarControl::getFocus()
{ 
        return this->focus;
};

void
CarControl::setFocus(int focus)
{ 
        this->focus = focus;
};
