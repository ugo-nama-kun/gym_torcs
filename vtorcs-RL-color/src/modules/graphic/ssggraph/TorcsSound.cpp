// -*- Mode: c++ -*-
/***************************************************************************
    file                 : TorcsSound.cpp
    created              : Tue Apr 5 19:57:35 CEST 2005
    copyright            : (C) 2005 Christos Dimitrakakis, Bernhard Wymann
    email                : dimitrak@idiap.ch
    version              : $Id: TorcsSound.cpp,v 1.8 2005/11/18 00:20:32 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "TorcsSound.h"
#include "SoundInterface.h"


/// Set the volume \note effect not consistent across backends
void TorcsSound::setVolume(float vol)
{
	this->volume = vol;
}
/// Set the pitch \note Effect not consistent across backends
void TorcsSound::setPitch(float pitch)
{
	this->pitch = pitch;
}
/// Set the filter \note Effect not consistent across backends
void TorcsSound::setLPFilter(float lp)
{
	this->lowpass = lp;
}

/// Create a new PLib sound. It requires a scheduler to be set up
/// and a filename to read data from.
PlibTorcsSound::PlibTorcsSound(slScheduler* sched,
			       const char* filename,
			       int flags,
			       bool loop) : TorcsSound (flags)
{
	this->sched = sched;
	this->loop = loop;
	MAX_VOL = 1.0f;
	sample = new slSample (filename, sched);
	if (flags & ACTIVE_VOLUME) {
		volume_env = new slEnvelope(1, SL_SAMPLE_ONE_SHOT);
	}
	if (flags & ACTIVE_PITCH) {
		pitch_env = new slEnvelope(1, SL_SAMPLE_ONE_SHOT);
	}
	if (flags & ACTIVE_LP_FILTER) {
		lowpass_env = new slEnvelope(1, SL_SAMPLE_ONE_SHOT);
	}
	if (loop) {
		sched->loopSample (sample);
	}
	if (flags & ACTIVE_VOLUME) {
		sched->addSampleEnvelope (sample, 0, VOLUME_SLOT, volume_env,
					  SL_VOLUME_ENVELOPE);
	}
	if (flags & ACTIVE_PITCH) {
		sched->addSampleEnvelope (sample, 0, PITCH_SLOT, pitch_env,
					  SL_PITCH_ENVELOPE);
	}
	if (flags & ACTIVE_LP_FILTER) {
		sched->addSampleEnvelope(sample, 0, FILTER_SLOT, lowpass_env,
					 SL_FILTER_ENVELOPE);
	}
	if (flags & ACTIVE_VOLUME) {
		volume_env->setStep(0, 0.0f, 0.0f);
	}
	if (flags & ACTIVE_PITCH) {
		pitch_env->setStep(0, 0.0f, 1.0f);
	}
	if (flags & ACTIVE_LP_FILTER) {
		lowpass_env->setStep(0, 0.0, 1.0f);
	}
	volume = 0.0f;
	pitch = 1.0f;
	lowpass = 1.0f;
	playing = false;
	paused = false;
}


/// Destructor.
PlibTorcsSound::~PlibTorcsSound()
{
	sched->stopSample(sample);
	if (flags & ACTIVE_VOLUME) {
		sched->addSampleEnvelope(sample, 0, VOLUME_SLOT, NULL,
					 SL_NULL_ENVELOPE);
		delete volume_env;
	}
	if (flags & ACTIVE_PITCH) {
		sched->addSampleEnvelope(sample, 0, PITCH_SLOT, NULL,
					 SL_NULL_ENVELOPE);
		delete pitch_env;
	}
	if (flags & ACTIVE_LP_FILTER) {
		sched->addSampleEnvelope(sample, 0, FILTER_SLOT, NULL,
					 SL_NULL_ENVELOPE);
		delete lowpass_env;
	}
	delete sample;
}

/** Set the volume.  Since plib does not support envelopes for
 * one-shot samples, we pre-adjust their volume
 */
void PlibTorcsSound::setVolume(float vol)
{
	if (vol > MAX_VOL) {
		vol = MAX_VOL;
	}
	this->volume = vol;

    if (loop==false) {
        sample->adjustVolume (vol);
    }
}
/// Start the sample
void PlibTorcsSound::play()
{
	start();
}
/// Start the sample
void PlibTorcsSound::start()
{
	// TODO: consistency check?
	if (loop) {
		if (playing == false) {
			playing = true;
			sched->loopSample (sample);
		}
	} else {
		playing = true;
		sched->playSample (sample);
	}
}
/// Stop the sample
void PlibTorcsSound::stop()
{
	if (playing == true) {
		playing = false;
		sched->stopSample (sample);
	}
}
/// Resume a paused sample.
void PlibTorcsSound::resume()
{
	sched->resumeSample (sample);
	paused = false;
}
/// Pause a sample
void PlibTorcsSound::pause()
{
	sched->pauseSample (sample);
	paused = true;
}



/** Update the plib sounds.
 * This should be called as often as possible from the main sound code,
 * probably by looping through all the sounds used.
 */
void PlibTorcsSound::update()
{
	if (flags & ACTIVE_VOLUME) {
		volume_env->setStep(0, 0.0f, volume);
	}
	if (flags & ACTIVE_PITCH) {
		pitch_env->setStep(0, 0.0f, pitch);

	}
	if (flags & ACTIVE_LP_FILTER) {
		lowpass_env->setStep(0, 0.0f, lowpass);
	}
}

/// Create a sound source
SoundSource::SoundSource()
{
	a = 0.0;
	f = 1.0;
	lp = 1.0;
}

/** Calculate environmental parameters for current situation.
 *
 * At the moment this
 */

void SoundSource::update()
{
	// Get relative speed/position vector
	sgVec3 u;
	sgVec3 p;
	float u_rel = 0.0f;
	float u_rel_src = 0.0f;
	float u_rel_lis = 0.0f;
	float p_rel = 0.0f;
	int i;
	for (i=0; i<3; i++) {
		u[i] = u_src[i] - u_lis[i];
		p[i] = p_src[i] -  p_lis[i];
		p_rel += p[i]*p[i];
	}
    
	a = 1.0;
	f = 1.0f;
	lp = 1.0f;

	// Only the vector component on the LOV is significant
	//    u_rel = sqrt(u_rel);
	p_rel = 0.01f + sqrt(p_rel);
	float p_cosx = p[0]/p_rel;
	float p_cosy = p[1]/p_rel;
	float p_cosz = p[2]/p_rel;
	float p_x_comp = u[0]*p_cosx;
	float p_y_comp = u[1]*p_cosy;
	float p_z_comp = u[2]*p_cosz;
	float p_x_src = u_src[0]*p_cosx;
	float p_y_src = u_src[1]*p_cosy;
	float p_z_src = u_src[2]*p_cosz;
	float p_x_lis = u_lis[0]*p_cosx;
	float p_y_lis = u_lis[1]*p_cosy;
	float p_z_lis = u_lis[2]*p_cosz;
	u_rel = (p_y_comp + p_x_comp + p_z_comp);
	u_rel_src = (p_y_src + p_x_src + p_z_src);
	u_rel_lis = (p_y_lis + p_x_lis + p_z_lis);
	if (fabs(u_rel)>=0.9f*SPEED_OF_SOUND) {
		// Cut-off sound when relative speed approaches speed of sound.
		a = 0.0f;
		f = 1.0f;
		lp = 1.0f;
	} else {
		// attenuate and filter sound with distance
		// and shift pitch with speed
		float ref = 5.0f;
		float rolloff = 0.5f;
		float atten = ref / ( ref + rolloff * (p_rel - ref));
		//f = SPEED_OF_SOUND/(SPEED_OF_SOUND+u_rel);
		f = (SPEED_OF_SOUND - u_rel_src)/(SPEED_OF_SOUND - u_rel_lis);
		a = atten;
		float atten_filter = MIN (atten, 1.0f);
		lp = exp(atten_filter - 1.0f);
	}

}


/** Set source position and velocity.
 */
void SoundSource::setSource(sgVec3 p, sgVec3 u)
{
	for (int i=0; i<3; i++) {
		p_src[i] = p[i];
		u_src[i] = u[i];
	}
}

/** Set listener position and velocity.
 */
void SoundSource::setListener (sgVec3 p, sgVec3 u)
{
	for (int i=0; i<3; i++) {
		p_lis[i] = p[i];
		u_lis[i] = u[i];
	}
}


/** Create a new torcs sound
 * 
 *
 */
OpenalTorcsSound::OpenalTorcsSound(const char* filename, OpenalSoundInterface* sitf, int flags, bool loop, bool static_pool)
{

	this->loop = loop;
	this->flags = flags;
	this->static_pool = static_pool;
	volume = 0.0f;
	pitch = 1.0f;
	lowpass = 1.0f;
	poolindex = -1;
	itf = sitf;

	MAX_DISTANCE = 10000.0f;
	MAX_DISTANCE_LOW = 5.0f;
	REFERENCE_DISTANCE = 5.0f;
	ROLLOFF_FACTOR = 0.5f;

	playing = false;
	paused = false;

	int i;
	for (i = 0; i<3; i++) {
		source_position[i] = 0.0f;
		source_velocity[i] = 0.0f;
		zeroes[i] = 0.0f;
	}

	//printf("SOUND, create source: %s -> %s\n", filename, (static_pool == true) ? "static" : "dynamic");

	int error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("Uncatched OpenAL Error on entry: %d with file %s\n", error, filename);
	}
	
	alGenBuffers (1, &buffer);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alGenBuffers failed %s\n", error, filename);
		is_enabled = false;
		return;
	}

	ALvoid *wave = NULL;
	ALsizei size;
	ALsizei freq;
	ALenum format;
	ALboolean srcloop;

	alutLoadWAVFile((ALbyte *) filename, &format, &wave, &size, &freq, &srcloop);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, could not load %s\n", error, filename);
		if (alIsBuffer(buffer)) {
			alDeleteBuffers(1, &buffer);
			alGetError();
		}
		is_enabled = false;
		return;
	}
	
	alBufferData (buffer, format, wave, size, freq);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alBufferData %s\n", error, filename);
		if (alIsBuffer(buffer)) {
			alDeleteBuffers(1, &buffer);
			alGetError();
		}
		is_enabled = false;
		return;
	}

	alutUnloadWAV(format, wave, size, freq);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alutUnloadWAV %s\n", error, filename);
	}
	
	if (!static_pool) {
		is_enabled = true;
		return;
	}
	
	if (!sitf->getStaticSource(&source)) {
		is_enabled = false;
		printf("    No static sources left: %s\n", filename);
		if (alIsBuffer(buffer)) {
			alDeleteBuffers(1, &buffer);
			alGetError();
		}
		return;
	} else {
		is_enabled = true;	
	}

	alSourcefv (source, AL_POSITION, source_position);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcefv AL_POSITION %s\n", error, filename);
	}

	alSourcefv (source, AL_VELOCITY, source_velocity);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcefv AL_VELOCITY %s\n", error, filename);
	}

	alSourcei (source, AL_BUFFER, buffer);
	error = alGetError();
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcei AL_BUFFER %s\n", error, filename);
	}

	alSourcei (source, AL_LOOPING, loop);
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcei AL_LOOPING %s\n", error, filename);
	}

	alSourcef (source, AL_MAX_DISTANCE, MAX_DISTANCE);
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcef AL_MAX_DISTANCE %s\n", error, filename);
	}

	alSourcef (source, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcef AL_REFERENCE_DISTANCE %s\n", error, filename);
	}

	alSourcef (source, AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR);
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcef AL_ROLLOFF_FACTOR %s\n", error, filename);
	}

	alSourcef (source, AL_GAIN, 0.0f);
	if (error != AL_NO_ERROR) {
		printf("OpenAL Error: %d, alSourcef AL_GAIN %s\n", error, filename);
	}
}

OpenalTorcsSound::~OpenalTorcsSound()
{
	if (alIsSource(source)) {
		alSourceStop (source);
		alDeleteSources(1, &source);
	}
	if (alIsBuffer(buffer)) {
		alDeleteBuffers(1, &buffer);
	}
}

void OpenalTorcsSound::setVolume (float vol)
{
	this->volume = vol;
}

void OpenalTorcsSound::setPitch(float pitch)
{
	this->pitch = pitch;
}

void OpenalTorcsSound::setLPFilter(float lp)
{
	this->lowpass = lp;
}


void OpenalTorcsSound::setReferenceDistance(float dist)
{
	if (static_pool) {
		if (is_enabled) {
			alSourcef (source, AL_REFERENCE_DISTANCE, dist);
		}
	} else {
		if (itf->getSourcePool()->isSourceActive(this, &poolindex)) {
			alSourcef (source, AL_REFERENCE_DISTANCE, dist);		
			REFERENCE_DISTANCE = dist;
		}
	}
}


void OpenalTorcsSound::setSource (sgVec3 p, sgVec3 u)
{
	for (int i=0; i<3; i++) {
		source_position[i] = p[i];
		source_velocity[i] = u[i];
	}
}


void OpenalTorcsSound::getSource(sgVec3 p, sgVec3 u)
{
	for (int i=0; i<3; i++) {
		p[i] = source_position[i];
		u[i] = source_velocity[i];
	}
}


void OpenalTorcsSound::play()
{
	start();
}

void OpenalTorcsSound::start()
{
	if (static_pool) {
		if (is_enabled) {
			if (playing==false) {
				if (loop) {
					playing = true;
				}
				alSourcePlay (source);
			}
		}
	} else {
		// shared source.
		bool needs_init;
		if (itf->getSourcePool()->getSource(this, &source, &needs_init, &poolindex)) {
			if (needs_init) {
				// Setup source.
				alSourcefv (source, AL_POSITION, source_position);
				alSourcefv (source, AL_VELOCITY, source_velocity);
				alSourcei (source, AL_BUFFER, buffer);
				alSourcei (source, AL_LOOPING, loop);
				alSourcef (source, AL_MAX_DISTANCE, MAX_DISTANCE);
				alSourcef (source, AL_REFERENCE_DISTANCE, REFERENCE_DISTANCE);
				alSourcef (source, AL_ROLLOFF_FACTOR, ROLLOFF_FACTOR);
				alSourcef (source, AL_GAIN, 0.0f);
			}

			// play
			if (playing==false) {
				if (loop) {
					playing = true;
				}
				alSourcePlay (source);
			}
		}
	}
}


void OpenalTorcsSound::stop()
{
	if (static_pool) {
		if (is_enabled) {
			if (playing==true) {
				playing = false;
				alSourceStop (source);
			}
		}
	} else {
		// Shared source.	
		if (itf->getSourcePool()->releaseSource(this, &poolindex)) {
			if (playing==true) {
				playing = false;
				alSourceStop (source);
			}
		}
	}
}

void OpenalTorcsSound::resume()
{
	if (paused==true) {
		paused = false;
	}
}


void OpenalTorcsSound::pause()
{
	if (paused==false) {
		paused = true;
	}
}

void OpenalTorcsSound::update ()
{
    ALfloat zero_velocity[3] = {0.0f, 0.0f, 0.0f};
	if (static_pool) {
		if (is_enabled) {
			alSourcefv (source, AL_POSITION, source_position);
			alSourcefv (source, AL_VELOCITY, zero_velocity);
			alSourcef (source, AL_PITCH, pitch);
			alSourcef (source, AL_GAIN, volume);
		}
	} else {
		if (itf->getSourcePool()->isSourceActive(this, &poolindex)) {
			alSourcefv (source, AL_POSITION, source_position);
			alSourcefv (source, AL_VELOCITY, zero_velocity);
			alSourcef (source, AL_PITCH, pitch);
			alSourcef (source, AL_GAIN, volume);		
		}
	}
}


