/***************************************************************************

    file                 : PlibSoundInterface.cpp
    created              : Thu Apr 7 04:21 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis
    email                : dimitrak@idiap.ch
    version              : $Id: SoundInterface.cpp,v 1.4 2005/09/18 21:52:22 olethros Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "SoundInterface.h"
#include "CarSoundData.h"

SoundInterface::SoundInterface(float sampling_rate, int n_channels)
{
	this->sampling_rate = sampling_rate;
	this->n_channels = n_channels;
	int i;
	for (i = 0; i<4; i++) {
		skid_sound[i]=NULL;
	}
	road_ride_sound=NULL;
	grass_ride_sound=NULL;
	grass_skid_sound=NULL;
	metal_skid_sound=NULL;
	axle_sound=NULL;
	turbo_sound=NULL;
	backfire_loop_sound=NULL;
	for (i = 0; i<NB_CRASH_SOUND; i++) {
		crash_sound[i]=NULL;
	}
	curCrashSnd = 0;
	bang_sound=NULL;
	bottom_crash_sound=NULL;
	gear_change_sound=NULL;
	
	n_engine_sounds = n_channels - 12;

	int MAX_N_ENGINE_SOUNDS = 8;
	if (n_engine_sounds<1) {
		n_engine_sounds = 1;
		fprintf (stderr, "Warning: maybe insufficient channels\n");
	} else if (n_engine_sounds > MAX_N_ENGINE_SOUNDS) {
		n_engine_sounds = MAX_N_ENGINE_SOUNDS;
	}
}

void SoundInterface::SortSingleQueue (CarSoundData** car_sound_data, QueueSoundMap* smap, int n_cars)
{
	float max_vol = 0.0f;
	int max_id = 0;
	for (int id=0; id<n_cars; id++) {
		CarSoundData* sound_data = car_sound_data[id];
		QSoundChar CarSoundData::*p2schar = smap->schar;
		QSoundChar* schar = &(sound_data->*p2schar);
		float vol = sound_data->attenuation * schar->a;
		if (vol > max_vol) {
			max_vol = vol;
			max_id = id;
		}
	}
	smap->id = max_id;
	smap->max_vol = max_vol;
}

void SoundInterface::SetMaxSoundCar(CarSoundData** car_sound_data, QueueSoundMap* smap)
{
	int id = smap->id;
	float max_vol = smap->max_vol;
	QSoundChar CarSoundData::*p2schar = smap->schar;
	QSoundChar* schar = &(car_sound_data[id]->*p2schar);
	TorcsSound* snd = smap->snd;

	sgVec3 p;
	sgVec3 u;

	car_sound_data[id]->getCarPosition(p);
	car_sound_data[id]->getCarSpeed(u);
	snd->setSource (p, u);
	snd->setVolume (schar->a);
	snd->setPitch (schar->f);
	snd->update();
	if (max_vol > VOLUME_CUTOFF) {
		snd->start();
	} else {
		snd->stop();
	}
}
