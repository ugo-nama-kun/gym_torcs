/***************************************************************************

    file                 : PlibSoundInterface.cpp
    created              : Thu Apr 7 04:21 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis, Bernhard Wymann
    email                : dimitrak@idiap.ch
    version              : $Id: OpenalSoundInterface.cpp,v 1.11 2006/01/05 09:53:37 olethros Exp $

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
#include "TorcsSound.h"


/// Define this to use the OpenAL Doppler.
#undef USE_OPENAL_DOPPLER

const int OpenalSoundInterface::OSI_MIN_DYNAMIC_SOURCES = 4;



OpenalSoundInterface::OpenalSoundInterface(float sampling_rate, int n_channels): SoundInterface (sampling_rate, n_channels)
{
	car_src = NULL;

	ALfloat far_away[] = { 0.0f, 0.0f,  1000.0f };
	ALfloat zeroes[] = { 0.0f, 0.0f,  0.0f };
	ALfloat front[]  = { 0.0f, 0.0f,  1.0f, 0.0f, 1.0f, 0.0f };
	dev = alcOpenDevice( NULL );
	if( dev == NULL ) {
		throw ("Could not open device");
	}
	
	// Last zero is termination of the array, I think the current official beat SDK ignores that.
	// ALCint attr[] = { ALC_MONO_SOURCES, 1024, ALC_STEREO_SOURCES, 0, 0};
	cc = alcCreateContext( dev, NULL);
	if(cc == NULL) {
		alcCloseDevice( dev );
		throw ("Could not create context.");
	}

	alcMakeContextCurrent( cc );
	alcGetError(dev);
	alGetError();

	// Figure out the number of possible sources, watch out for an API update, perhaps
	// one can get that easier later with al(c)GetInteger (I'm sure that there is no way to
	// query this now) or even request a number with the context.
	const int MAX_SOURCES = 1024;
	int sources;
	ALuint sourcelist[MAX_SOURCES];
	for (sources = 0; sources < MAX_SOURCES; sources++) {
		alGenSources(1, &sourcelist[sources]);
		if (alGetError() != AL_NO_ERROR) {
			break;
		}
	}

	int clear;
	for (clear = 0; clear < sources; clear++) {
		if (alIsSource(sourcelist[clear])) {
			alDeleteSources(1, &sourcelist[clear]);
			if (alGetError() != AL_NO_ERROR) {
				printf("Error in probing OpenAL sources.\n");
			}
		} else {
			printf("Error in probing OpenAL sources.\n");
		}
	}

	OSI_MAX_SOURCES = sources;
	OSI_MAX_STATIC_SOURCES = MAX(0, OSI_MAX_SOURCES - OSI_MIN_DYNAMIC_SOURCES);

	// Figure out the number of buffers.
	int buffers;
	ALuint bufferlist[MAX_SOURCES];
	for (buffers = 0; buffers < MAX_SOURCES; buffers++) {
		alGenBuffers(1, &bufferlist[buffers]);
		if (alGetError() != AL_NO_ERROR) {
			break;
		}
	}

	for (clear = 0; clear < buffers; clear++) {
		if (alIsBuffer(bufferlist[clear])) {
			alDeleteBuffers(1, &bufferlist[clear]);
			if (alGetError() != AL_NO_ERROR) {
				printf("Error in probing OpenAL buffers.\n");
			}
		} else {
			printf("Error in probing OpenAL buffers.\n");
		}
	}

	OSI_MAX_BUFFERS = buffers;

	printf("OpenAL backend info:\n  Vendor: %s\n  Renderer: %s\n  Version: %s\n", alGetString(AL_VENDOR), alGetString(AL_RENDERER), alGetString(AL_VERSION));
	printf("  Available sources: %d%s\n", OSI_MAX_SOURCES, (sources >= MAX_SOURCES) ? " or more" : "");
	printf("  Available buffers: %d%s\n", OSI_MAX_BUFFERS, (buffers >= MAX_SOURCES) ? " or more" : "");
	
	alDistanceModel ( AL_INVERSE_DISTANCE );
	int error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d alDistanceModel\n", error);
	}

	alDopplerFactor (1.0f);
	//alSpeedOfSound (SPEED_OF_SOUND); // not defined in linux yet.
	alDopplerVelocity (SPEED_OF_SOUND);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d alDopplerX\n", error);
	}


	alListenerfv(AL_POSITION, far_away );
	alListenerfv(AL_VELOCITY, zeroes );
	alListenerfv(AL_ORIENTATION, front );
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d alListenerfv\n", error);
	}
	
	engpri = NULL;
	global_gain = 1.0f;
	
	// initialise mappings
	grass.schar = &CarSoundData::grass;
	grass_skid.schar = &CarSoundData::grass_skid;
	road.schar = &CarSoundData::road;
	metal_skid.schar = &CarSoundData::drag_collision;
	backfire_loop.schar = &CarSoundData::engine_backfire;
	turbo.schar = &CarSoundData::turbo;
	axle.schar = &CarSoundData::axle;

	n_static_sources_in_use = 0;
}

OpenalSoundInterface::~OpenalSoundInterface()
{
	delete sourcepool;
	for (unsigned int i=0; i<sound_list.size(); i++) {
		delete sound_list[i];
	}
	delete [] engpri;
	alcDestroyContext (cc);
	alcCloseDevice (dev);

	if (car_src) {
		delete [] car_src;
	}
}

void OpenalSoundInterface::setNCars(int n_cars)
{
	engpri = new SoundPri[n_cars];
	car_src = new SoundSource[n_cars];
}


TorcsSound* OpenalSoundInterface::addSample (const char* filename, int flags, bool loop, bool static_pool)
{
	TorcsSound* sound = new OpenalTorcsSound (filename, this, flags, loop, static_pool);
	sound_list.push_back (sound);
	return sound;
}
	
void OpenalSoundInterface::update(CarSoundData** car_sound_data, int n_cars, sgVec3 p_obs, sgVec3 u_obs, sgVec3 c_obs, sgVec3 a_obs)
{
	
	ALfloat listener_pos[3];
	ALfloat listener_speed[3];
	ALfloat listener_orientation[6];
    ALfloat zeros[] = {0.0f, 0.0f, 0.0f};
	
	int i;
	for (i = 0; i<3; i++) {
		listener_pos[i] = p_obs[i];
		listener_speed[i] = 0;// u_obs[i];
		listener_orientation[i] = c_obs[i];
		listener_orientation[i+3] = a_obs[i];
	}
	
	alListenerfv(AL_POSITION, listener_pos );
#ifdef USE_OPENAL_DOPPLER
	alListenerfv(AL_VELOCITY, listener_speed );
#else 
    alListenerfv(AL_VELOCITY, zeros);
#endif
	alListenerfv(AL_ORIENTATION, listener_orientation );
	alListenerf(AL_GAIN, getGlobalGain());

	for (i = 0; i<n_cars; i++) {
		car_sound_data[i]->copyEngPri(engpri[i]);
		int id = engpri[i].id;
		sgVec3 p;
		sgVec3 u;
		car_sound_data[id]->getCarPosition(p);
		car_sound_data[id]->getCarSpeed(u);
		car_src[id].setSource (p, u);
		car_src[id].setListener (p_obs, u_obs);
		car_src[id].update();
		engpri[id].a = car_src[id].a;
	}


	qsort ((void*) engpri, n_cars, sizeof(SoundPri), &sortSndPriority);


	int nsrc = MIN(sourcepool->getNbSources(), n_engine_sounds);

	// Reverse order is important to gain free sources from stopped engine sounds
	// before attempting to start new ones.
	for (i = n_cars - 1; i >= 0; i--) {
		int id = engpri[i].id;
		sgVec3 p;
		sgVec3 u;
		CarSoundData* sound_data = car_sound_data[id];
		sound_data->getCarPosition(p);
		sound_data->getCarSpeed(u);
		TorcsSound* engine = sound_data->getEngineSound();
		engine->setSource(p, u);
#ifdef USE_OPENAL_DOPPLER
		engine->setPitch (sound_data->engine.f);
#else
		engine->setPitch (car_src[id].f*sound_data->engine.f);
#endif
		engine->setVolume (sound_data->engine.a * 1.5f * exp(sound_data->engine.lp-1.0));
		//engine->setLPFilter(sound_data->engine.lp);

		engine->update();
		if (i < nsrc) {
			engine->start();
		} else {
			engine->stop();
		}
	}
	
	float max_skid_vol[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	int max_skid_id[4] = {0,0,0,0};
	int id;
	for (id = 0; id<n_cars; id++) {
		CarSoundData* sound_data = car_sound_data[id];
		for (int j=0; j<4; j++) {
			float skvol=sound_data->attenuation*sound_data->wheel[j].skid.a;
			if (skvol > max_skid_vol[j]) {
				max_skid_vol[j] = skvol;
				max_skid_id[j] = id;
			}
		}
	}

	for (i = 0; i<4; i++) {
		int id = max_skid_id[i];
		WheelSoundData* sound_data = car_sound_data[id]->wheel;
		skid_sound[i]->setSource (sound_data[i].p, sound_data[i].u);
		skid_sound[i]->setVolume (sound_data[i].skid.a);
#ifdef USE_OPENAL_DOPPLER
        /// \note Why MIN() here?
		skid_sound[i]->setPitch (MIN(sound_data[i].skid.f, 1.0f));
#else
		float mod_f = car_src[id].f;
		skid_sound[i]->setPitch (sound_data[i].skid.f * mod_f);
#endif
		skid_sound[i]->update();
		if (sound_data[i].skid.a > VOLUME_CUTOFF) {
			skid_sound[i]->start();
		} else {
			skid_sound[i]->stop();
		}
	}

	
	// other looping sounds
	road.snd = road_ride_sound;
	SortSingleQueue (car_sound_data, &road, n_cars);
	SetMaxSoundCar (car_sound_data, &road);

	grass.snd = grass_ride_sound;
	SortSingleQueue (car_sound_data, &grass, n_cars);
	SetMaxSoundCar (car_sound_data, &grass);

	grass_skid.snd = grass_skid_sound;
	SortSingleQueue (car_sound_data, &grass_skid, n_cars);
	SetMaxSoundCar (car_sound_data, &grass_skid);

	metal_skid.snd = metal_skid_sound;
	SortSingleQueue (car_sound_data, &metal_skid, n_cars);
	SetMaxSoundCar (car_sound_data, &metal_skid);

	backfire_loop.snd = backfire_loop_sound;
	SortSingleQueue (car_sound_data, &backfire_loop, n_cars);
	SetMaxSoundCar (car_sound_data, &backfire_loop);

	backfire_loop.snd = backfire_loop_sound;
	SortSingleQueue (car_sound_data, &backfire_loop, n_cars);
	SetMaxSoundCar (car_sound_data, &backfire_loop);

	turbo.snd = turbo_sound;
	SortSingleQueue (car_sound_data, &turbo, n_cars);
	SetMaxSoundCar (car_sound_data, &turbo);

	axle.snd = axle_sound;
	SortSingleQueue (car_sound_data, &axle, n_cars);
	SetMaxSoundCar (car_sound_data, &axle);

	// One-off sounds
	for (id = 0; id<n_cars; id++) {
		CarSoundData* sound_data = car_sound_data[id];
		sgVec3 p;
		sgVec3 u = {0, 0, 0};
		if (sound_data->crash) {
			if (++curCrashSnd>=NB_CRASH_SOUND) {
				curCrashSnd = 0;
			}
			sound_data->getCarPosition(p);
			sound_data->getCarSpeed(u);
			crash_sound[curCrashSnd]->setSource (p, u);
			crash_sound[curCrashSnd]->setVolume (1.0f);
			crash_sound[curCrashSnd]->setPitch (1.0f);
			crash_sound[curCrashSnd]->update();
			crash_sound[curCrashSnd]->start();
		}

		if (sound_data->bang) {
			sound_data->getCarPosition(p);
			sound_data->getCarSpeed(u);
			bang_sound->setSource (p, u);
			bang_sound->setVolume (1.0f);
			bang_sound->setPitch (1.0f);
			bang_sound->update();
			bang_sound->start();
		}

		if (sound_data->bottom_crash) {
			sound_data->getCarPosition(p);
			sound_data->getCarSpeed(u);
			bottom_crash_sound->setSource (p, u);
			bottom_crash_sound->setVolume (1.0f);
			bottom_crash_sound->setPitch (1.0f);
			bottom_crash_sound->update();
			bottom_crash_sound->start();
		}

		if (sound_data->gear_changing) {
			sound_data->getCarPosition(p);
			sound_data->getCarSpeed(u);
			gear_change_sound->setSource (p, u);
			gear_change_sound->setReferenceDistance (1.0f);
			gear_change_sound->setVolume (1.0f);
			gear_change_sound->setPitch (1.0f);
			gear_change_sound->update();
			gear_change_sound->start();
		}
	}

}


void OpenalSoundInterface::initSharedSourcePool(void)
{
	int nbdynsources = OSI_MAX_SOURCES - n_static_sources_in_use;
	sourcepool = new SharedSourcePool(nbdynsources);
	printf("  #static sources: %d\n", n_static_sources_in_use);
	printf("  #dyn sources   : %d\n", sourcepool->getNbSources());
}


bool OpenalSoundInterface::getStaticSource(ALuint *source)
{
	// Do we have a source left for static assigned sources?
	if (n_static_sources_in_use < OSI_MAX_STATIC_SOURCES - 1) {
		alGenSources (1, source);
		int error = alGetError();
		if (error != AL_NO_ERROR) {
			return false;
		} else {
			n_static_sources_in_use++;
			return true;
		}
	} else {
		return false;
	}
}

void OpenalSoundInterface::SetMaxSoundCar(CarSoundData** car_sound_data, QueueSoundMap* smap)
{
	int id = smap->id;
	float max_vol = smap->max_vol;
	QSoundChar CarSoundData::*p2schar = smap->schar;
	QSoundChar* schar = &(car_sound_data[id]->*p2schar);
	TorcsSound* snd = smap->snd;

	sgVec3 p;
	sgVec3 u = {0.0f, 0.0f, 0.0f};

	car_sound_data[id]->getCarPosition(p);
#ifdef USE_OPENAL_DOPPLER
	car_sound_data[id]->getCarSpeed(u);
#endif
	snd->setSource (p, u);
	snd->setVolume (schar->a);
#ifdef USE_OPENAL_DOPPLER
	snd->setPitch (schar->f);
#else
	snd->setPitch (schar->f * car_src[id].f);
#endif
	snd->update();
	if (max_vol > VOLUME_CUTOFF) {
		snd->start();
	} else {
		snd->stop();
	}
}
