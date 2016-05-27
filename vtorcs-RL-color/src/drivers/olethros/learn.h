// -*- Mode: c++ -*-
/***************************************************************************

    file                 : learn.h
    created              : Wed Aug 28 16:36:00 CET 2004
    copyright            : (C) 2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: learn.h,v 1.10.2.1 2008/11/09 17:50:20 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _SEGLEARN_H_
#define _SEGLEARN_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <math.h>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robottools.h>
#include <robot.h>

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros
{
#endif

/// A class for learning various things about segments.
class SegLearn {
public:
	SegLearn(tTrack* t);
	~SegLearn();
	/// Ge the inferred radius of segment \c s
	float getRadius(tTrackSeg *s) { return radius[s->id]; }
	/// Update track radius offset
	void update(tSituation *s, tTrack *t, tCarElt *car, int alone, float offset, float outside, float *r, float alpha, float speed, float limit);
	/// Update acceleration and steering prediction
	float updateAccel (tSituation* s, tCarElt* car, float taccel, float derr, float dtm);
	/// Get predicted acceleration error
	float predictedError (tCarElt* car);
	/// Get predicted acceleration error
	float predictedAccel (tTrackSeg* seg);
	/// Adjust friction model coefficients
	void AdjustFriction (tTrackSeg* s, float G, float mass_, float CA_, float CW_, float u_, float brake_, float learning_rate = 1.0);
	/// Get friction coefficient
	float GetFrictionDm(tTrackSeg* s) {return (dm + segdm[s->id]);}
	/// Get friction coefficient
	float GetFrictionDm2(tTrackSeg* s) {return (dm2 + segdm2[s->id]);}
	/// Get friction coefficient
	float GetFrictionDm3(tTrackSeg* s) {return (dm3 + segdm3[s->id]);}
	/// Load
	void loadParameters (char* fname);
	/// Save
	void saveParameters (char* fname);

	void SetSafetyThreshold (float st) {safety_threshold = st;}
private:
	float tand(float x)
	{
		if (x>1.0) {
			return 1.0;
		} else if (x<-1.0) {
			return -1.0;
		}
		return x;
	}

	float tand_der(float x)
	{
		if (x>1.0) {
			return 0.0;
		} else if (x<-1.0) {
			return 0.0;
		}
		return 1.0;
	}
	float safety_threshold;
	bool LoadParameter (float* p, int n, FILE* f);

	void PropagateUpdateBackwards (tTrackSeg* pseg, float d, float beta, float max_length);
	float time_since_accident;
	/// Class for computing averages of measured values
	class Averages {
	public:
		Averages ()
		{
			k=0;
			taccel = derr = dtm = 0.0;
		}
		int k; ///< number of measurements
		float taccel; ///< acceleration
		float derr; ///< steering error
		float dtm; ///< path error
		/// Add measurements to list.
		void Measure(float taccel_, float derr_, float dtm_)
		{
			float r = (float) k;
			float d = 1.0f/(1.0f + (float) k);
			taccel = d *(taccel * r + taccel_);
			derr = d *(derr * r + derr_);
			dtm = d *(dtm * r + dtm_);
			k++;
		}
	};
	Averages averages; ///< holds average measurements
	tTrack* track; ///< A pointer to the current track
	int n_quantums; ///< number of quantums the track is split for learning
	int segments_per_quantum; ///< number of segments each quantum contains
	int prev_quantum; ///< id of the previous quantum
	float prev_accel; ///< previous acceleration
	double previous_time; ///< previous time
	int segQuantum (int segid); ///< The quantum ID of a given segment ID
	float *radius; ///< Array holding the radi of segments
	int *updateid; ///< Array holding the segments to be update
	float* accel; ///< Accelerations for each quantum
	float* derror; ///< Steering control for each quantum
	float* elig; ///< Eligibility of previously visited quantums for updates

	// estimates for friction
	float* segdm; ///< friction coefficient 1
	float* segdm2; ///< friction coefficient 2
	float* segdm3; ///< friction coefficient 3
	int prevsegid; ///< id of previous segment
	float W_brake;
	float W_accel;
	float dm; ///< global friction coefficient 1
	float dm2; ///< gloabal friction coefficient 2
	float dm3; ///< global friction coefficient 3
	float mu; ///< friction constant
	float mass; ///< mass
	float CA; ///< downforce constant
	float CW; ///< drag constant
	float u; ///< speed
	float brake; ///< brake input
	double prev_time; ///< time of previous friction estimate
	double delta_time; ///< dt for friction estimate
	double time_since_left_turn;
	double time_since_right_turn;
	bool check; ///< whether we should update radius
	float rmin; ///< current estimated minimum radius
	int lastturn; ///< type of last turn entered
	int prevtype;  ///< type of previous segment
	int n_seg; ///< total number of track segments
	bool new_lap; ///< we made a new lap
	int remaining_laps; ///< remaining_laps
    int lap; ///< current lap
};



#ifdef USE_OLETHROS_NAMESPACE
}
#endif

#endif //_SEGLEARN_H_
