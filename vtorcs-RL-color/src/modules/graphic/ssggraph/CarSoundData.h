// -*- Mode: c++ -*-
/***************************************************************************
    file                 : CarSoundData.h
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis
    email                : dimitrak@idiap.ch
    version              : $Id: CarSoundData.h,v 1.2 2005/07/02 14:01:33 olethros Exp $

 ***************************************************************************/

#ifndef CAR_SOUND_DATA_H
#define CAR_SOUND_DATA_H

#include <vector>

#include "QSoundChar.h"

typedef struct WheelSoundData_
{
	sgVec3 p; ///< position
	sgVec3 u; ///< speed
	QSoundChar skid;
} WheelSoundData;

/// Manages the source sound of each individual car.
class CarSoundData {
protected:
	sgVec3 listener_position;
	sgVec3 position;
	sgVec3 speed;
	TorcsSound* engine_sound;
	SoundInterface* sound_interface;
	void calculateAttenuation (tCarElt* car);
	void calculateEngineSound (tCarElt* car);
	void calculateBackfireSound (tCarElt* car);
	void calculateTyreSound (tCarElt* car);
	void calculateGearChangeSound (tCarElt* car);
	void calculateCollisionSound (tCarElt* car);
public:
	SoundPri eng_pri;
	WheelSoundData wheel[4];
	float attenuation; ///< global distance attenuation
	float base_frequency; ///< engine base frequency for ~ 6000 rpm
	float smooth_accel; ///< smoothed acceleration input
	float pre_axle; ///< axle related
	bool turbo_on; ///< use turbo sound
	float turbo_rpm; ///< when turbo comes in
	float turbo_ilag; ///< how fast turbo catches up with engine
	QSoundChar engine;
	QSoundChar drag_collision;
	QSoundChar turbo;
	QSoundChar axle;
	QSoundChar engine_backfire;
	QSoundChar grass_skid;
	QSoundChar grass;
	QSoundChar road;
	QSoundChar skid_metal;
	int prev_gear;

	CarSoundData (int id, SoundInterface* sound_interface);
	void setEngineSound (TorcsSound* engine_sound, float rpm_scale);
	void setTurboParameters (bool turbo_on, float turbo_rpm, float turbo_lag);
	TorcsSound* getEngineSound () {return engine_sound;}
	void copyEngPri (SoundPri& epri) 	{epri = eng_pri;}
	void setCarPosition (sgVec3 p)
	{
		for (int i=0; i<3; i++) {
			position[i] = p[i];
		}
	}
	void setCarSpeed (sgVec3 u)
	{
		for (int i=0; i<3; i++) {
			speed[i] = u[i];
		}
	}
	void getCarPosition (sgVec3 p)
	{
		for (int i=0; i<3; i++) {
			p[i] = position[i];
		}
	}
	void getCarSpeed (sgVec3 u)
	{
		for (int i=0; i<3; i++) {
			u[i] = speed[i];
		}
	}
	void setListenerPosition (sgVec3 p)
	{
		for (int i=0; i<3; i++) {
			listener_position[i] = p[i];
		}
	}
	void update (tCarElt* car);
	bool gear_changing;
	bool bottom_crash;
	bool bang;
	bool crash;
};

#endif /* CAR_SOUND_DATA_H */
