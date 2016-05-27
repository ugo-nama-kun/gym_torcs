// -*- Mode: c++ -*-
/***************************************************************************
    file                 : SoundInterface.h
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis, Bernhard Wymann
    email                : dimitrak@idiap.ch
    version              : $Id: SoundInterface.h,v 1.7 2005/11/18 00:20:32 olethros Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SOUND_INTERFACE_H
#define SOUND_INTERFACE_H

#define USE_OPENAL 1

#undef SOUND_LOG


#ifdef SOUND_LOG
#define logmsg printf ("# "); printf
#else
#define logmsg empty_log
static void empty_log(char* s, ...)
{
}
#endif



#include <plib/sl.h>
#include <plib/sg.h>
#include <raceman.h>
#include <vector>

#include "TorcsSound.h"
//#include "grsound.h"
#include "QSoundChar.h"


#define VOLUME_CUTOFF 0.001f


class CarSoundData;

/** A queue containing mappings between sounds and sound sources.
 *
 * Provides a mapping various sound sources and actual sounds. This is
 * used for the case where we have many sound sources emitting exactly
 * the same sound and where we don't allow more than 1 source to play
 * simultaneously. This structure can be used to sort competing
 * sources in order to decide which one is going to take priority.
 *
 * \sa SortSingleQueue(), SetMaxSoundChar()
 */
typedef struct QueueSoundMap_ {
	QSoundChar CarSoundData::*schar; ///< The calculated sound characteristic
	TorcsSound* snd; ///< The raw sound.
	float max_vol; ///< Max.
	int id; ///< The id of the car producing the sound, used for retrieving doppler effects etc
} QueueSoundMap;

/** Current state of sound*/
enum SoundPriState {
	None=0x0, Loaded, Playing, Paused, Stopped, Cleared
};

/** Sound priority, used to sort cars according to amplitude attenuation */
typedef struct SoundPri_ {
	float a; ///< amplitude
	int id; ///< car ID.
} SoundPri;

/// Sound interface
class SoundInterface {
 protected:
	float sampling_rate; ///< sampling rate
	int n_channels; ///< number of channels
	int n_engine_sounds; ///< number of simultaneous engines
	int curCrashSnd; ///< holds current crash sound used - the sound cycles
	TorcsSound* skid_sound[4]; ///< set of skid sounds, one per tyre
	TorcsSound* road_ride_sound; ///< rolling on normal road
	TorcsSound* grass_ride_sound; ///< rolling on dirt/grass
	TorcsSound* grass_skid_sound; ///< skidding on dirt/grass
	TorcsSound* metal_skid_sound; ///< metal skidding on metal 
	TorcsSound* axle_sound; ///< axle/gear spinning sound
	TorcsSound* turbo_sound; ///< turbo spinning sound
	TorcsSound* backfire_loop_sound; ///< exhaust backfire sound
	TorcsSound* crash_sound[NB_CRASH_SOUND]; ///< list of crash sounds
	TorcsSound* bang_sound; ///< sounds when suspension fully compressed
	TorcsSound* bottom_crash_sound; ///< bang when crashing from a great height
	TorcsSound* backfire_sound; ///< one-off backfire sound
	TorcsSound* gear_change_sound; ///< sound when changing gears

	std::vector<TorcsSound*> sound_list; ///< keeps track of sounds used
	SoundPri* engpri; ///< the engine priority, used for sorting

    /// The following are mappings for sound prioritisation
	QueueSoundMap road; 
	QueueSoundMap grass; 
	QueueSoundMap grass_skid;
	QueueSoundMap metal_skid;
	QueueSoundMap backfire_loop;
	QueueSoundMap turbo;
	QueueSoundMap axle;

    /** Find the max amplitude sound in car_sound_data and put it in smap  */
	void SortSingleQueue (CarSoundData** car_sound_data, 
			      QueueSoundMap* smap,
			      int n_cars);

    /** Using the smap->id, get the appropriate entry in
    car_sound_data and call apprioriate methods for smap->snd in order
    to play the sound.*/
	void SetMaxSoundCar(CarSoundData** car_sound_data,
			    QueueSoundMap* smap);
	
 public:
    /// Make a new sound interface
	SoundInterface(float sampling_rate, int n_channels);

    /// Destructor - does nothing
	virtual ~SoundInterface() {}

    /// Set the number of cars - must be defined in children classes
	virtual void setNCars(int n_cars) = 0;

    /// Add a new sample - must be defined in children classes
	virtual TorcsSound* addSample (const char* filename,
				       int flags = (ACTIVE_VOLUME|ACTIVE_PITCH),
				       bool loop = false, bool static_pool = true) = 0;

    /// initialised the pool of shared sources
	virtual void initSharedSourcePool() {}

	void setSkidSound (const char* sound_name)
	{
		for (int i=0; i<4; i++) {
			TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
			skid_sound[i] = sound;
		}

	}
	void setRoadRideSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		road_ride_sound = sound;
	}
	void setGrassRideSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		grass_ride_sound = sound;
	}
	void setGrassSkidSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		grass_skid_sound = sound;
	}
	void setMetalSkidSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		metal_skid_sound = sound;
	}
	void setAxleSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		axle_sound = sound;
	}
	void setTurboSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		turbo_sound = sound;
	}
	void setBackfireLoopSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, ACTIVE_VOLUME|ACTIVE_PITCH, true);
		backfire_loop_sound = sound;
	}
	void setCrashSound (const char* sound_name, int index)
	{
		TorcsSound* sound = addSample (sound_name, 0, false);
		assert (index>=0 && index<NB_CRASH_SOUND);
		crash_sound[index] = sound;
	}

	void setBangSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, 0, false);
		bang_sound = sound;
	}

	void setBottomCrashSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, 0, false);
		bottom_crash_sound = sound;
	}

	void setBackfireSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, 0, false);
		backfire_sound = sound;
	}

	void setGearChangeSound (const char* sound_name)
	{
		TorcsSound* sound = addSample (sound_name, 0, false);
		gear_change_sound = sound;
	}
	/// Update sound for a given observer.
	virtual void update(CarSoundData** car_sound_data,
			    int n_cars, sgVec3 p_obs, sgVec3 u_obs, 
			    sgVec3 c_obs = NULL, sgVec3 a_obs = NULL)
	{
		// do nothing
	}
	virtual float getGlobalGain() {return 1.0f;}
	virtual void setGlobalGain(float g) 
	{
		fprintf (stderr, "Warning, gain setting not supported\n");
	}

};



/// PLIB interface
class PlibSoundInterface : public SoundInterface {
	typedef struct SoundChar_ {
		float f; //frequency modulation
		float a; //amplitude modulation
	} SoundChar;
 protected:
	slScheduler* sched;
	std::vector<TorcsSound*> sound_list;
	SoundPri* engpri;
	SoundSource* car_src;
	SoundSource tyre_src[4];
	void DopplerShift (SoundChar* sound, float* p_src, float* u_src, float* p, float* u);
	void SetMaxSoundCar(CarSoundData** car_sound_data, QueueSoundMap* smap);
	float global_gain;
 public:
	PlibSoundInterface(float sampling_rate, int n_channels);
	virtual ~PlibSoundInterface();
	virtual void setNCars(int n_cars);
	virtual slScheduler* getScheduler();
	virtual TorcsSound* addSample (const char* filename, int flags = (ACTIVE_VOLUME|ACTIVE_PITCH), bool loop = false, bool static_pool = true);
	virtual void update(CarSoundData** car_sound_data, int n_cars, sgVec3 p_obs, sgVec3 u_obs, sgVec3 c_obs = NULL, sgVec3 a_obs = NULL);
	virtual float getGlobalGain() {return global_gain;}
	virtual void setGlobalGain(float g)
	{
		global_gain = 0.5f*g;
		logmsg ("Setting gain to %f\n", global_gain);
	}

};



/// Open AL interface

class SharedSourcePool;

class OpenalSoundInterface : public SoundInterface {
	typedef struct SoundChar_ {
		float f; //frequency modulation
		float a; //amplitude modulation
	} SoundChar;
 protected:
	SoundSource* car_src;
	SoundSource tyre_src[4];
	ALCcontext* cc;
	ALCdevice* dev;
	float global_gain;
	int OSI_MAX_BUFFERS;
	int OSI_MAX_SOURCES;
	int OSI_MAX_STATIC_SOURCES;
	int n_static_sources_in_use;
	SharedSourcePool* sourcepool;
	static const int OSI_MIN_DYNAMIC_SOURCES;

	void DopplerShift (SoundChar* sound, float* p_src, float* u_src, float* p, float* u);
    virtual void SetMaxSoundCar(CarSoundData** car_sound_data, QueueSoundMap* smap);

 public:
	OpenalSoundInterface(float sampling_rate, int n_channels);
	virtual ~OpenalSoundInterface();
	virtual void setNCars(int n_cars);
	virtual TorcsSound* addSample (const char* filename,
				       int flags = (ACTIVE_VOLUME|ACTIVE_PITCH),
				       bool loop = false, bool static_pool = true);
	virtual void update(CarSoundData** car_sound_data, int n_cars, sgVec3 p_obs, sgVec3 u_obs, sgVec3 c_obs, sgVec3 a_obs);
	virtual float getGlobalGain() { return global_gain; }
	virtual void initSharedSourcePool();
	virtual void setGlobalGain(float g)
	{
		global_gain = 0.5f*g;
		logmsg ("Setting gain to %f\n", global_gain);
	}
	virtual bool getStaticSource(ALuint *source);

	virtual SharedSourcePool* getSourcePool(void) { return sourcepool; } 


};


int sortSndPriority(const void* a, const void* b);


struct sharedSource {
	ALuint source;
	TorcsSound* currentOwner;
	bool in_use;
};


class SharedSourcePool {
	public:
		SharedSourcePool(int nbsources):nbsources(nbsources) {
			pool = new sharedSource[nbsources];
			int i;
			for (i = 0; i < nbsources; i++) {
				pool[i].currentOwner = NULL;
				pool[i].in_use = false;
				alGenSources(1, &(pool[i].source));
				int error = alGetError();
				if (error != AL_NO_ERROR) {
					printf("OpenAL error, allocating dynamic source index %d\n", i);
					this->nbsources = i;
					break;
				}
			}
			printf("  Dynamic Sources: requested: %d, created: %d\n", nbsources, this->nbsources);
		}
		
		virtual ~SharedSourcePool() {
			int i;
			for (i = 0; i < nbsources; i++) {
				alDeleteSources(1, &(pool[i].source));
				alGetError();
			}			
			delete [] pool;
		}

		bool getSource(TorcsSound* sound, ALuint* source, bool* needs_init, int* index) {
			if (*index >= 0 && *index < nbsources) {
				if (sound == pool[*index].currentOwner) {
					// Resurrect source from pool.
					*source = pool[*index].source;
					*needs_init = false;
					pool[*index].in_use = true;
					return true;
				}
			}
			
			// TODO: Implement free list with ring buffer or whatever data structure
			// to go from O(n) to O(1). If the ordering is done well it will automatically
			// result in LRU (least recently used).
			int i, firstfree = -1;
			for (i = 0; i < nbsources; i++) {
				if (pool[i].in_use == false && firstfree < 0) {
					firstfree = i;
					break;
				}
			}
			
			if (firstfree < 0) {
				// No free source.
				return false;
			}

			pool[firstfree].currentOwner = sound;
			pool[firstfree].in_use = true;
			*source = pool[firstfree].source;
			*needs_init = true;
			*index = firstfree;
			return true;
		}

		bool releaseSource(TorcsSound* sound, int* index) {
			if (*index >= 0 && *index < nbsources) {
				if (sound == pool[*index].currentOwner) {
					pool[*index].in_use = false;
					return true;
				}
			}
			return false;
		}

		bool isSourceActive(TorcsSound* sound, int* index) {
			if (*index >= 0 && *index < nbsources &&
				sound == pool[*index].currentOwner &&
				true == pool[*index].in_use)
			{
				return true;
			} else {
				return false;
			}
		}

		int getNbSources(void) { return nbsources; }

	protected:
		int nbsources;
		sharedSource *pool;
};



#endif /* SOUND_INTERFACE_H */
