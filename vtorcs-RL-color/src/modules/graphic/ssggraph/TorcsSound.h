// -*- Mode: c++ -*-
/***************************************************************************
    file                 : TorcsSound.h
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis
    email                : dimitrak@idiap.ch
    version              : $Id: TorcsSound.h,v 1.3 2005/11/18 00:20:32 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef TORCS_SOUND_H
#define TORCS_SOUND_H

#define VOLUME_SLOT 0
#define PITCH_SLOT 1
#define FILTER_SLOT 2

#include <plib/sg.h>
#include <plib/sl.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>
#include "sound_defines.h"

class SoundInterface;
class OpenalSoundInterface;

/** A generic TORCS sound.  The aim is to have a more or less
 * identical interface to sounds, no matter what the backend is. In
 * practice, there are some minor differences across interfaces. In
 * particular, for Plib sounds, setting pitch and volume actually sets
 * the pitch/volume of the sound that we are listening at, while for
 * Openal sounds, it sets only the pitch/volume of the source
 * sound. Thus, the interface is not consistent across
 * backends. Making it consistent is not easily doable because some
 * things simply work too differently. Currently I have traded
 * complexity in TorcsSound for replicating some code in the two
 * SoundInterface implementatins: OpenalSoundInterface and
 * PlibSoundInterface both take into account the individual
 * pecularities of each PlibTorcsSound and OpenalTorcsSound. However,
 * if it is deemed necessary later on, it is possible to make the
 * interface consistent.
 */
class TorcsSound {
protected:
	class SoundInterface* iface; ///< Handler to the interface
	int flags; ///< Flags relating to what effects are to be used.
	float MAX_VOL; ///< Maximum volume
	float volume; ///< Current volume
	float pitch; ///< Current pitch
	float lowpass; ///< Current low pass filter
	bool loop; ///< Whether it's a looping sound
public:
    /// Consruct a sound.
	TorcsSound(int flags = (ACTIVE_VOLUME|ACTIVE_PITCH))
	{
		this->flags = flags;
		
	}
    /// Destructor
	virtual ~TorcsSound() {}
	virtual void setVolume(float vol);
	virtual void setPitch(float pitch);
	virtual void setLPFilter(float lp);
	virtual void setSource(sgVec3 p, sgVec3 u) {}
	virtual float getVolume() {return volume;}
	virtual float getPitch() {return pitch;}
	virtual float getLPfilter() {return lowpass;}
	virtual void setReferenceDistance (float dist) {}
	virtual void getSource (sgVec3 p, sgVec3 u) {}
	//virtual void setListener(sgVec3 p, sgVec3 u) = 0;
	virtual void play() = 0;
	virtual void start() = 0;
	virtual void stop() = 0;
	virtual void resume() = 0;
	virtual void pause() = 0;
	virtual void update() = 0;
	virtual bool isPlaying() = 0;
	virtual bool isPaused() = 0;
};

/// PLIB-specific torcs sound.
class PlibTorcsSound : public TorcsSound {
protected:
	slSample* sample; ///< sample data
	slEnvelope* volume_env; ///< volume envelope
	slEnvelope* pitch_env; ///< pitch envelope
	slEnvelope* lowpass_env; ///< low pass filter envelope
	slScheduler* sched; ///< plib sl scheduler (see sl.h)
	bool playing; ///< Sound is playing
    bool paused; ///< sound is paused
public:
	PlibTorcsSound(slScheduler* sched,
				   const char* filename,
				   int flags = (ACTIVE_VOLUME|ACTIVE_PITCH),
				   bool loop = false);
	virtual ~PlibTorcsSound();
	virtual void setVolume(float vol);
	//virtual void setSource(sgVec3 p, sgVec3 u);
	//virtual void setListener (sgVec3 p, sgVec3 u);
	virtual void play();
	virtual void start();
	virtual void stop();
	virtual void resume();
	virtual void pause();
	virtual void update();
    /// True if the sound is playing.
	virtual bool isPlaying()
	{
		return playing;
	}
    /// Truye if the sound is paused.
	virtual bool isPaused() 
	{
		return paused;
	}
};

/** Sound source management.
 *
 * The sound source is different from the sound itself. It should
 * describe the position, speed, environment and other aspects of the
 * sound's source. Each sound source can emit many different actual
 * sounds.
 * 
 * Since sources can be coupled to particular listeners, it is in
 * principle possible to implement multiple listeners, something which
 * should be extremely useful for same-computer multiplayer games.
 */
class SoundSource {
public:
	sgVec3 p_lis; ///< listener position for this source
	sgVec3 u_lis; ///< listener velocity for this source
	sgVec3 p_src; ///< source position
	sgVec3 u_src; ///< source velocity;
	float a; ///< Environmental attenuation
    float f; ///< Environmental frequency shift
    float lp; ///< Environmental filtering
	SoundSource();
	void update();
	void setSource(sgVec3 p, sgVec3 u);
	void setListener (sgVec3 p, sgVec3 u);
};


/** Openal torcs sound */
class OpenalTorcsSound : public TorcsSound {
protected:
	ALuint buffer; ///< buffer id
	ALuint source; ///< source id
	ALfloat source_position[3]; ///< source position
	ALfloat source_velocity[3]; ///< source velocity
	ALfloat zeroes[3]; ///< just a vector of 0s
	ALfloat back[6]; ///< direction of back
	ALfloat front[6]; ///< direction of front 
	bool playing, paused;
	ALfloat MAX_DISTANCE; ///< maximum allowed distance
	ALfloat MAX_DISTANCE_LOW; ///< maximum allowed distance
	ALfloat REFERENCE_DISTANCE; ///< reference distance for sound
	ALfloat ROLLOFF_FACTOR; ///< how fast we need to roll off
	int poolindex; ///< which pool the sound is assigned to
	OpenalSoundInterface* itf; ///< Handle to the interface
	bool static_pool;	///< dynamic or static source assignment?
	bool is_enabled;	///< is it available at all?
public:
	OpenalTorcsSound(const char* filename,
					 OpenalSoundInterface* sitf,
					 int flags = (ACTIVE_VOLUME|ACTIVE_PITCH),
					 bool loop = false, bool static_pool = true);
	virtual ~OpenalTorcsSound();
	virtual void setVolume(float vol);
	virtual void setPitch(float pitch);
	virtual void setLPFilter(float lp);
	virtual void setSource(sgVec3 p, sgVec3 u);
	virtual void getSource (sgVec3 p, sgVec3 u);
	virtual void setReferenceDistance (float dist);
	//virtual void setListener (sgVec3 p, sgVec3 u);
	virtual void play();
	virtual void start();
	virtual void stop();
	virtual void resume();
	virtual void pause();
	virtual void update();
    /// Return true if playing
	virtual bool isPlaying()
	{
		return playing;
	}
    /// Return true if paused.
	virtual bool isPaused() 
	{
		return paused;
	}
};






#endif
