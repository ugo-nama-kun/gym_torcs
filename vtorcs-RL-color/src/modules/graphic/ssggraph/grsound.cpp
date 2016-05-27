/***************************************************************************

    file                 : grsound.cpp
    created              : Thu Aug 17 23:57:10 CEST 2000
    copyright            : (C) 2000-2003 by Eric Espie, Christos Dimitrakakis
    email                : torcs@free.fr, dimitrak@idiap.ch
    version              : $Id: grsound.cpp,v 1.31 2007/11/18 09:13:16 torcs Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under1 the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <math.h>

#include <tgfclient.h>
#include <graphic.h>
#include <car.h>

#include "grsound.h"
#include "grmain.h"
#include "sound_defines.h"
#include "SoundInterface.h"
#include "CarSoundData.h"


static int soundInitialized = 0;
static double lastUpdated;

static SoundInterface* sound_interface = NULL;
static CarSoundData** car_sound_data = NULL;

enum SoundMode {DISABLED, OPENAL_MODE, PLIB_MODE};

static enum SoundMode sound_mode = OPENAL_MODE;


void grInitSound(tSituation* s, int ncars)
{
	char	buf[256];

	GfOut("-- grInitSound\n");

	// Check if we want sound (sound.xml).
	char *soundDisabledStr = GR_ATT_SOUND_STATE_DISABLED;
	char *soundOpenALStr = GR_ATT_SOUND_STATE_OPENAL;
	char *soundPlibStr = GR_ATT_SOUND_STATE_PLIB;
	char fnbuf[1024];
	sprintf(fnbuf, "%s%s", GetLocalDir(), GR_SOUND_PARM_CFG);
	void *paramHandle = GfParmReadFile(fnbuf, GFPARM_RMODE_REREAD | GFPARM_RMODE_CREAT);
	char *optionName = GfParmGetStr(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_STATE, soundOpenALStr);
	float global_volume = GfParmGetNum(paramHandle, GR_SCT_SOUND, GR_ATT_SOUND_VOLUME, "%", 100.0f);
	if (!strcmp(optionName, soundDisabledStr)) {
		sound_mode = DISABLED;
	} else if (!strcmp(optionName, soundOpenALStr)) {
		sound_mode = OPENAL_MODE;
	} else if (!strcmp(optionName, soundPlibStr)) {
		sound_mode = PLIB_MODE;
	}
	//printf ("vol:%f\n", global_volume);
	GfParmReleaseHandle(paramHandle);
	paramHandle = NULL;

	lastUpdated = -1000.0;
	
	switch (sound_mode) {
	case OPENAL_MODE:
		sound_interface = new OpenalSoundInterface (44100, 32);
		break;
	case PLIB_MODE:
		sound_interface = new PlibSoundInterface(44100, 32);
		break;
	case DISABLED:
		return;
	default:
		GfOut (" -- Unknown sound mode %d\n", sound_mode);
		exit(-1);
	}

	sound_interface->setGlobalGain(global_volume/100.0f);

	car_sound_data = new CarSoundData* [ncars];

	int i;
	for (i = 0; i<ncars; i++) {
		void* handle = s->cars[i]->_carHandle;
		tCarElt	*car = s->cars[i];
		char* param;
		char filename[512];
        FILE *file = NULL;

		// ENGINE PARAMS
		tdble rpm_scale;
		param = GfParmGetStr(handle, "Sound", "engine sample", "engine-1.wav");
		rpm_scale = GfParmGetNum(handle, "Sound", "rpm scale", NULL, 1.0);
        sprintf (filename, "cars/%s/%s", car->_carName, param);
        file = fopen(filename, "r");
        if (!file)
        {
 		    sprintf (filename, "data/sound/%s", param);
        }
        else
        {
            fclose(file);
        }

		car_sound_data[car->index] = new CarSoundData (car->index, sound_interface);
		TorcsSound* engine_sound = sound_interface->addSample(filename, ACTIVE_VOLUME | ACTIVE_PITCH | ACTIVE_LP_FILTER, true, false);
		car_sound_data[i]->setEngineSound (engine_sound, rpm_scale);

		// TURBO PARAMS
		float default_turbo_rpm = 100.0f;//0.5f*car->_enginerpmMaxTq;
		bool turbo_on;
		param = GfParmGetStr(handle, SECT_ENGINE, PRM_TURBO, "false");
		if (!strcmp(param, "true")) {
			turbo_on = true;
		} else {
			if (strcmp(param, "false")) {
				fprintf (stderr, "expected true or false, found %s\n", param);
			}
			turbo_on = false;
		}
		
		float turbo_rpm = GfParmGetNum(handle, SECT_ENGINE, PRM_TURBO_RPM, NULL, default_turbo_rpm);
		float turbo_lag = GfParmGetNum(handle, SECT_ENGINE, PRM_TURBO_LAG, NULL, 1.0f);
		car_sound_data[i]->setTurboParameters (turbo_on, turbo_rpm, turbo_lag);
	}


	
	sound_interface->setSkidSound("data/sound/skid_tyres.wav");
	sound_interface->setRoadRideSound("data/sound/road-ride.wav");
	sound_interface->setGrassRideSound("data/sound/out_of_road.wav");
	sound_interface->setGrassSkidSound("data/sound/out_of_road-3.wav");
	sound_interface->setMetalSkidSound("data/sound/skid_metal.wav");
	sound_interface->setAxleSound("data/sound/axle.wav");
	sound_interface->setTurboSound("data/sound/turbo1.wav");
	sound_interface->setBackfireLoopSound("data/sound/backfire_loop.wav");

    for (i = 0; i < NB_CRASH_SOUND; i++) {
		sprintf(buf, "data/sound/crash%d.wav", i+1);
		sound_interface->setCrashSound(buf, i);
    }

	sound_interface->setBangSound("data/sound/boom.wav");
	sound_interface->setBottomCrashSound("data/sound/bottom_crash.wav");
	sound_interface->setBackfireSound("data/sound/backfire.wav");
	sound_interface->setGearChangeSound("data/sound/gear_change1.wav");

	sound_interface->setNCars(ncars);
    soundInitialized = 1;
	// Must happen after all static non-shared have been allocated. 
	sound_interface->initSharedSourcePool();
}


void 
grShutdownSound(int ncars)
{
    GfOut("-- grShutdownSound\n");

	if (sound_mode == DISABLED) {
		return;
	}

    if (!soundInitialized) {
		return;
    }
    soundInitialized = 0;

	delete sound_interface;

    if (__slPendingError) {
		GfOut("!!! error ignored: %s\n", __slPendingError);
		__slPendingError = 0;	/* ARG!!! ugly ugly bad thing... but should not occur anymore now */
    }
}



float
grRefreshSound(tSituation *s, cGrCamera	*camera)
{
	if (sound_mode == DISABLED) {
		return 0.0f;
	}

	// Update sound at most 50 times a second.
	const double UPDATE_DT = 0.02;
	if (s->currentTime - lastUpdated < UPDATE_DT) {
		return 0.0f;
	}
	lastUpdated = s->currentTime;

    tCarElt	*car;//= s->cars[s->current];

	// TODO: Fix for a lot of cars. I guess in this implementation we can change the Update() call to have _ncars = 1?

	// TODO: Just consider cars near the camera, doing computations  just for them?

	if (camera) {
		sgVec3* p_camera = camera->getPosv();
		sgVec3* u_camera = camera->getSpeedv();
		sgVec3 c_camera;
		sgVec3* a_camera = camera->getUpv();
		int i;
		for (i = 0; i<3; i++) {
			c_camera[i] = (*camera->getCenterv())[i] - (*camera->getPosv())[i];
		}

		//sgNormaliseVec3 (c_camera);


		for (i = 0; i < s->_ncars; i++) {
			car = s->cars[i];
			car_sound_data[car->index]->setListenerPosition(*p_camera);
			car_sound_data[car->index]->update(car);
		}

		sound_interface->update (car_sound_data, s->_ncars, *p_camera, *u_camera, c_camera, *a_camera);


	}
	return 0.0f;
}
