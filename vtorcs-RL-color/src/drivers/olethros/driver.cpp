
/***************************************************************************

    file                 : driver.cpp
    created              : Thu Dec 20 01:21:49 CET 2002
    copyright            : (C) 2002-2004 Bernhard Wymann
                           (C) 2004-2005 Christos Dimitrakakis
    email                : berniw@bluewin.ch
                           dimitrak@idiap.ch
    version              : $Id: driver.cpp,v 1.17.2.1 2008/05/30 22:18:32 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
   \file driver.cpp
   
   \brief High-level driver implementation.

   This file is the same as Bernhard's driver.cpp for the BT robot,
   with added bits for learning, plus some modifications to make driving
   smoother.
*/
#include "driver.h"
#include <learning/ann_policy.h>
#include <learning/MathFunctions.h>
#include "geometry.h"
#include <math.h>
#include <portability.h>
#include "TrackData.h"
#include "Trajectory.h"

#ifdef WIN32
#include <float.h>
#define isnan _isnan
#endif

#undef TEST_PITSTOPS

#ifdef USE_OLETHROS_NAMESPACE
namespace olethros {
#endif

    const float Driver::MAX_UNSTUCK_ANGLE = 15.0/180.0*PI;		///< [radians] If the angle of the car on the track is smaller, we assume we are not stuck.
    const float Driver::UNSTUCK_TIME_LIMIT = 2.0f;				///< [s] We try to get unstuck after this time.
    const float Driver::MAX_UNSTUCK_SPEED = 5.0f;				///< [m/s] Below this speed we consider being stuck.
    const float Driver::MIN_UNSTUCK_DIST = 2.0f;					///< [m] If we are closer to the middle we assume to be not stuck.
    const float Driver::G = 9.81f;								///< [m/(s*s)] Welcome on Earth.
    const float Driver::FULL_ACCEL_MARGIN = 3.0f;				///< [m/s] Margin reduce oscillation of brake/acceleration.
    const float Driver::SHIFT = 0.98f;							///< [-] (% of rpmredline) When do we like to shift gears.
    const float Driver::SHIFT_MARGIN = 4.0f;						///< [m/s] Avoid oscillating gear changes.
    const float Driver::ABS_SLIP = 2.0f;							///< [m/s] range [0..10]
    const float Driver::ABS_RANGE = 5.0f;						///< [m/s] range [0..10]
    const float Driver::ABS_MINSPEED = 3.0f;						///< [m/s] Below this speed the ABS is disabled (numeric, division by small numbers).
    const float Driver::TCL_SLIP = 2.0f;							///< [m/s] range [0..10]
    const float Driver::TCL_RANGE = 10.0f;						///< [m/s] range [0..10]
    const float Driver::LOOKAHEAD_CONST = 17.0f;					///< [m]
    const float Driver::LOOKAHEAD_FACTOR = 0.33f;				///< [-]
    const float Driver::WIDTHDIV = 3.0f;							///< [-] Defines the percentage of the track to use (2/WIDTHDIV).
    const float Driver::SIDECOLL_MARGIN = 3.0f;					///< [m] Distance between car centers to avoid side collisions.
    const float Driver::BORDER_OVERTAKE_MARGIN = 0.5f;			///< [m]
    const float Driver::OVERTAKE_OFFSET_SPEED = 2.0f;			///< [m/s] Offset change speed.
    const float Driver::OVERTAKE_TIME = 2.0f;			        ///< [s] Approach time to consider overtaking.
    const float Driver::PIT_LOOKAHEAD = 6.0f;					///< [m] Lookahead to stop in the pit.
    const float Driver::PIT_BRAKE_AHEAD = 200.0f;				///< [m] Workaround for "broken" pitentries.
    const float Driver::PIT_MU = 0.4f;							///< [-] Friction of pit concrete.
    const float Driver::MAX_SPEED = 84.0f;						///< [m/s] Speed to compute the percentage of brake to apply.
    const float Driver::MAX_FUEL_PER_METER = 0.0008f;			///< [liter/m] fuel consumtion.
    const float Driver::CLUTCH_SPEED = 5.0f;						///< [m/s]
    const float Driver::CENTERDIV = 0.1f;						///< [-] (factor) [0.01..0.6].
    const float Driver::DISTCUTOFF = 200.0f;						///< [m] How far to look, terminate while loops.
    const float Driver::MAX_INC_FACTOR = 5.0f;					///< [m] Increment faster if speed is slow [1.0..10.0].
    const float Driver::CATCH_FACTOR = 10.0f;					///< [-] select MIN(catchdist, dist*CATCH_FACTOR) to overtake.
    const float Driver::CLUTCH_FULL_MAX_TIME = 1.0f;				///< [s] Time to apply full clutch.
    const float Driver::USE_LEARNED_OFFSET_RANGE = 0.2f;			///< [m] if offset < this use the learned stuff
    const float Driver::ACCELERATOR_LETGO_TIME = 0.1f;           ///< [s] Time to let go of accelerator before braking.
    const float Driver::MIN_BRAKE_FOLLOW_DISTANCE = 1.0f;        ///< [m] Predicted minimum distance from next opponent to apply full brake
    const float Driver::MAX_BRAKE_FOLLOW_DISTANCE = 2.0f;        ///< [m] Predicted minimum distance from next opponent to start applying brake.
    const float Driver::STEER_DIRECTION_GAIN = 1.0f; ///< [-] Gain to apply for basic steerin
    const float Driver::STEER_DRIFT_GAIN = 0.01f; ///< [-] Gain for drift compensation
    const float Driver::STEER_PREDICT_GAIN = 0.1f; ///< [-] Predictive gain, multiplied with yaw rate.
    const float Driver::STEER_AVOIDANCE_GAIN = 0.2f; ///< [lograd/m] Gain for border avoidance steering
    const float Driver::STEER_EMERGENCY_GAIN = 0.01f; ///< [-] Gain for emergency steering
    const float Driver::FILTER_STEER_FEEDBACK = 0.8f;        ///< [-] Feedback from steering to steer filter.
    const float Driver::FILTER_PREDICT_FEEDBACK = 0.0f;        ///< [-] Feedback from prediction error to steer filter.
    const float Driver::FILTER_TARGET_FEEDBACK = 0.2f;        ///< [-] Feedback from target error to steer filter.
    const bool Driver::USE_NEW_ALPHA=false;  ///< Use actual trajectory..

    Cardata *Driver::cardata = NULL;
    double Driver::currentsimtime;

    static const int SBSIZE = 1024;

    Driver::Driver(int index)
    {
	INDEX = index;
	seg_alpha = NULL;
	seg_alpha_new = NULL;
	max_speed_list = NULL;
	u_toleft = 0.0f;
	u_toright = 0.0f;
	prev_toleft = 0.0f;
	prev_toright = 0.0f;
	prev_steer = 0.0f;
	speed_factor = 1.0f;
	dt = 0.0f;
	pit_exit_timer = 1.0f;
    }


    Driver::~Driver()
    {
	
	// Do not save learnt stuff from race itself.

#if 1 // Do not write on users HD for release ?

	// We save only for practice and qualifying
	if (race_type!=RM_TYPE_RACE) {
            // We save only for practice
            //if (race_type==RM_TYPE_PRACTICE) {
            char fname[SBSIZE];
            snprintf(fname, SBSIZE, "%s%s%d/%s.brain", GetLocalDir(),
                     "drivers/olethros/", INDEX, track->internalname);
            
            char path[SBSIZE];
            snprintf(path, SBSIZE, "%s%s%d", GetLocalDir(), "drivers/olethros/", INDEX);
            
            if (GfCreateDir(path) == GF_DIR_CREATED) {
                learn->saveParameters (fname);
            }
	}
#endif


#if 0
	printf ("# BEGIN ~Driver() dump\n");
	for (int i=0; i<track->nseg; i++, seg=seg->next) {
            printf ("%d %.1f %.1f %1.f %2.f\n", seg->id,
                    radius[seg->id],
                    ideal_radius[seg->id],
                    learn->getRadius(seg),
                    learn->predictedAccel(seg));
	}
	printf ("# END ~Driver() dump\n");
#endif

	delete opponents;
	delete pit;
	delete [] radius;
	delete [] ideal_radius;
	delete [] seg_alpha;
	delete [] seg_alpha_new;
	delete [] max_speed_list;
	delete learn;
	delete strategy;

	if (cardata != NULL) {
            delete cardata;
            cardata = NULL;
	}
    }


    /// Called for every track change or new race.
    void Driver::initTrack(tTrack* t, void *carHandle, void **carParmHandle, tSituation *s)
    {
	track = t;

	const int BUFSIZE = 256;
	char buffer[BUFSIZE];
	// Load a custom setup if one is available.
	// Get a pointer to the first char of the track filename.
	char* trackname = strrchr(track->filename, '/') + 1;

	switch (s->_raceType) {
        case RM_TYPE_PRACTICE:
            snprintf(buffer, BUFSIZE, "drivers/olethros/%d/practice/%s", INDEX, trackname);
            break;
        case RM_TYPE_QUALIF:
            snprintf(buffer, BUFSIZE, "drivers/olethros/%d/qualifying/%s", INDEX, trackname);
            break;
        case RM_TYPE_RACE:
            snprintf(buffer, BUFSIZE, "drivers/olethros/%d/race/%s", INDEX, trackname);
            break;
        default:
            break;
	}

	//printf ("Trying %s\n", buffer);
	*carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);

	if (*carParmHandle == NULL) {
            snprintf(buffer, BUFSIZE, "drivers/olethros/%d/default.xml", INDEX);
            //printf ("LOADING: %s\n", buffer);
            *carParmHandle = GfParmReadFile(buffer, GFPARM_RMODE_STD);
        } else {
            //printf ("LOADING: %s\n", buffer);
	}

	// Create a pit stop strategy object.
	if (s->_raceType == RM_TYPE_RACE) {
            strategy = new ManagedStrategy();
	} else {
            strategy = new SimpleStrategy();
	}

	// Init fuel.
	strategy->setFuelAtRaceStart(t, carParmHandle, s);

	// Load and set parameters.
	MU_FACTOR = GfParmGetNum(*carParmHandle, OLETHROS_SECT_PRIV, OLETHROS_ATT_MUFACTOR, (char*)NULL, 0.69f);

    }


    /// Start a new race.
    void Driver::newRace(tCarElt* car, tSituation *s)
    {
	float deltaTime = (float) RCM_MAX_DT_ROBOTS;
	MAX_UNSTUCK_COUNT = int(UNSTUCK_TIME_LIMIT/deltaTime);
	OVERTAKE_OFFSET_INC = OVERTAKE_OFFSET_SPEED*deltaTime;
	stuck = 0;
	alone = 1;
	clutchtime = 0.0f;
	oldlookahead = 0.0f;
	u_toleft = 0.0f;
	u_toright = 0.0f;
	prev_toleft = 0.0f;
	prev_toright = 0.0f;
	prev_steer = 0.0f;
	alone_count = 0.0f;
	
	//printf ("INITIALISING ANOTHER CAR: ALONE_COUNT:%f\n", alone_count);
	overtaking = false;

	this->car = car;
	CARMASS = GfParmGetNum(car->_carHandle, SECT_CAR, PRM_MASS, NULL, 1000.0f);
	myoffset = 0.0f;
	initCa();
	initCw();
	initTireMu();
	initTCLfilter();
	mass = CARMASS + car->_fuel;

	//radius = new float[track->nseg];
	ideal_radius = new float[track->nseg];
	prepareTrack();
	//ShowPaths();

	// Create just one instance of cardata shared by all drivers.
	if (cardata == NULL) {
            cardata = new Cardata(s);
	}
	mycardata = cardata->findCar(car);
	currentsimtime = s->currentTime;

	// initialize the list of opponents.
	opponents = new Opponents(s, this, cardata);
	opponent = opponents->getOpponentPtr();

	// Initialize radius of segments.
	radius = new float[track->nseg];
	computeRadius(radius);

	learn = new SegLearn(track);

	// create the pit object.
	pit = new Pit(s, this);

	// save race type - to be eaten^H^H^H^H used later perhaps
	race_type = s->_raceType;

	// If we are not practicing, we read old parameters,
	// otherwise we start from scratch.
	if (race_type!=RM_TYPE_PRACTICE) {
            char fname[SBSIZE];
            snprintf(fname, SBSIZE, "%s%s%d/%s.brain", GetLocalDir(),
                     "drivers/olethros/", INDEX, track->internalname);
            learn->loadParameters (fname);
            learn->SetSafetyThreshold (0.0f);
	} else {
            learn->SetSafetyThreshold (0.5f);
	}
	 

	max_speed_list = new float[track->nseg];
	if (1) {
            tTrackSeg *s = track->seg;
            for (int i=0; i<track->nseg; i++, s=s->next) {
                float u = getAllowedSpeed(s);
                if (isnan(u) || u>10000.0) {
                    u = 10000.0;
                }
                max_speed_list[s->id] = u;
                //printf ("%d %f %f %f %f\n", s->id, max_speed_list[s->id], u, radius[s->id], 	learn->getRadius(s));
            }
	}
    }


    /// Drive during race.
    void Driver::drive(tSituation *s)
    {
	memset(&car->ctrl, 0, sizeof(tCarCtrl));
		
	update(s);
	if (race_type==RM_TYPE_PRACTICE) {
            if (car->_laps<200) {
                learn->SetSafetyThreshold (0.0f);
            } else {
                learn->SetSafetyThreshold (0.5f);
            }
	} else if (race_type==RM_TYPE_RACE) {
            learn->SetSafetyThreshold (0.5f);
	} else if (race_type==RM_TYPE_QUALIF) {
            learn->SetSafetyThreshold (0.5f);
        }

	if (0)
            {
		tTrackSeg* segment = car->_trkPos.seg;
		printf ("S:%1.f ", segment->radius);
		printf ("E:%1.f ", EstimateRadius2 (segment));
		printf ("R:%1.f ", radius[segment->id]);
		printf ("I:%1.f\n", ideal_radius[segment->id]);
            }
#ifdef TEST_PITSTOPS
	pit->setPitstop(true); // uncomment to test pitstops
#endif
	if (isStuck()) {
            car->_steerCmd = -mycardata->getCarAngle() / car->_steerLock;
            car->_gearCmd = -1;		// Reverse gear.
            car->_accelCmd = 1.0;	// 100% accelerator pedal.
            car->_brakeCmd = 0.0;	// No brakes.
            car->_clutchCmd = 0.0;	// Full clutch (gearbox connected with engine).
	} else {
            float steer = filterSColl(getSteer() 
                                      - 0.2f*learn->predictedError(car));
            //float gamma_steer = exp(- 0.1f * dt * fabs(getSpeed())); 
            //prev_steer = prev_steer * (1-gamma_steer) + gamma_steer * steer;
            car->_steerCmd = steer;
            car->_gearCmd = getGear();
            float fbrake = filterBrakeSpeed(getBrake());
            float faccel = getAccel();
            float accel = faccel - fbrake;
            float trk_accel;
            if (pit->getInPit()) {
                trk_accel = accel;
            } else {
                trk_accel = filterTrk(s, accel);
            }
            //printf ("fa:%f ta:%f ", faccel, trk_accel);
#if 0
            if (trk_accel > 0) {
                faccel = trk_accel;
                fbrake = 0;
            } else {
                fbrake = -trk_accel;
                faccel = 0;
            }
#endif
            fbrake = filterABS(filterBColl(filterBPit(fbrake)));
            faccel = filterTCL(filterAPit(faccel));
            if (fbrake > 0) {
                trk_accel = -fbrake;
            } else {
                trk_accel = faccel;
            }

            //trk_accel = faccel-fbrake;
		

            if (trk_accel >= 0) {
                car->_accelCmd = trk_accel;
                car->_brakeCmd = 0.0;
            } else {
                car->_accelCmd = 0.0;
                car->_brakeCmd = -trk_accel;
            }
            //printf ("fb:%f fa:%f ta:%f\n", fbrake, faccel, trk_accel);
            car->_clutchCmd = getClutch();

            //float mu = segptr->surface->kFriction;
            if (car->priv.collision) {

                //printf ("Collision, not adjusting (%d %f)\n",
                //car->_laps, s->currentTime);
                learn->AdjustFriction(car->_trkPos.seg, G, mass,CA, CW,getSpeed(), car->_brakeCmd, 0.0);
                car->priv.collision = 0;
            } else if (alone) {
                if (car->_accelCmd > 0) {
                    //float ratio =  0.0001*car->_engineMaxTq*car->_gearRatio[car->_gear + car->_gearOffset];
                    learn->AdjustFriction(car->_trkPos.seg, G, mass,CA, CW,getSpeed(), -car->_accelCmd, 0.001f);
                } else {
                    learn->AdjustFriction(car->_trkPos.seg, G, mass,CA, CW,getSpeed(), car->_brakeCmd, 0.001f);
                }
            } else {
                learn->AdjustFriction(car->_trkPos.seg, G, mass,CA, CW,getSpeed(), car->_brakeCmd, 0.0f);
            }
            //printf ("%f %f\n", car->_steerCmd, learn->predictedError(car));
            int segid = car->_trkPos.seg->id;
#if 0
            printf("S2:%1.f gS:%1.f sx:%1.f as:%1.f ms:%1.f\n",
                   3.6*sqrt(currentspeedsqr), 3.6*getSpeed(), 3.6*car->_speed_x,
                   3.6*current_allowed_speed,
                   3.6*max_speed_list[segid]);
#endif

            if (race_type == RM_TYPE_RACE) {
                if (alone) {
                    max_speed_list[segid] += 0.1 * dt * (car->_speed_x + 5.0f - max_speed_list[segid]);
                }
            } else {
                max_speed_list[segid] += 0.1 * dt * (car->_speed_x + 5.0f - max_speed_list[segid]);
            }

	}
    }


    /// Set pitstop commands.
    int Driver::pitCommand(tSituation *s)
    {
	car->_pitRepair = strategy->pitRepair(car, s);
	car->_pitFuel = strategy->pitRefuel(car, s);
	// This should be the only place where the pit stop is set to false!
	pit->setPitstop(false);
	return ROB_PIT_IM; // return immediately.
    }


    /// End of the current race.
    void Driver::endRace(tSituation *s)
    {
	// Nothing at the moment.
    }


    /***************************************************************************
     *
     * utility functions
     *
     ***************************************************************************/

    /// Pre-compute the radius of all curves
    void Driver::computeRadius(float *radius)
    {
	float lastturnarc = 0.0;
	int lastsegtype = TR_STR;

	tTrackSeg *currentseg, *startseg = track->seg;
	currentseg = startseg;

	do {
            
            //printf ("R: :%1.f %1.f %1.f\n", ideal_radius[currentseg->id], EstimateRadius2(currentseg), currentseg->radius);
            if (currentseg->type == TR_STR) {
                lastsegtype = TR_STR;
                if (isnan(ideal_radius[currentseg->id]) || 
                    ideal_radius[currentseg->id]>10000) {
                    ideal_radius[currentseg->id] = 10000;
                }
                //printf ("%d : %1.f\n", currentseg->id, ideal_radius[currentseg->id]);
                float R = EstimateRadius2 (currentseg);
                //printf ("R: %1.f %1.f\n", ideal_radius[currentseg->id], R);
                radius[currentseg->id] = MAX(R, ideal_radius[currentseg->id]);
                radius[currentseg->id] = ideal_radius[currentseg->id];
            } else {
                if (currentseg->type != lastsegtype) {
                    float arc = 0.0;
                    tTrackSeg *s = currentseg;
                    lastsegtype = currentseg->type;

                    while (s->type == lastsegtype && arc < PI/2.0) {
                        arc += s->arc;
                        s = s->next;
                    }
                    lastturnarc = arc/(PI/2.0);
                }
                radius[currentseg->id] = (currentseg->radius + currentseg->width/2.0)/lastturnarc;

                if (isnan(ideal_radius[currentseg->id]) || 
                    ideal_radius[currentseg->id]>10000.0f) {
                    ideal_radius[currentseg->id] = 10000.0f;
                }
                float R = EstimateRadius2 (currentseg);
                radius[currentseg->id] = MAX(radius[currentseg->id], ideal_radius[currentseg->id]);
                radius[currentseg->id] = MAX(radius[currentseg->id],R);
                radius[currentseg->id] = ideal_radius[currentseg->id];
            }
            currentseg = currentseg->next;
	} while (currentseg != startseg);

#if 0
	// smooth
	currentseg = startseg;
	do {
            float cr = radius[currentseg->id];
            float cn = MIN (cr, radius[currentseg->next->id]);
            float cp = MIN (cr, radius[currentseg->prev->id]);

            radius[currentseg->id] = 0.5f * (cr + 0.5f * (cn+cp));
            currentseg = currentseg->next;
	} while (currentseg != startseg);
#endif
    }


    /// Compute the allowed speed on a segment.
    float Driver::getAllowedSpeed(tTrackSeg *segment)
    {
	float mu = segment->surface->kFriction*TIREMU*MU_FACTOR;
	float r = radius[segment->id];
	float dr = learn->getRadius(segment);

	if ((alone > 0) && (fabs(myoffset) < USE_LEARNED_OFFSET_RANGE)) {
            float new_r = r+dr;
            float min_r = r;
            if (segment->type!=TR_STR) {
                min_r = MIN(min_r, segment->radiusr);
                min_r = MIN(min_r, segment->radiusl);
            }
            if (new_r > min_r && !pit->getInPit()) {
                r = new_r;
            }
	} else {
            //float calpha = seg_alpha[segment->id];
            //float normal_offset = (calpha - 0.5f) * segment->width;
            //float adj_offset = tanh(fabs(myoffset)) * (myoffset - normal_offset);
            float delta_offset = fabs (myoffset);

            if (dr >= -0.5f * r) {
                r += dr * (1.0f - tanh(delta_offset));
            }
            float alpha = 2.0f * delta_offset / segment->width;
            alpha = MIN (1, alpha);
            alpha = MAX (0, alpha);
            float segrad = segment->radius;
            if (segment->type == TR_STR) {
                segrad = 1000.0f;
            }
            //printf ("%f %f %f %f %f\n", delta_offset, normal_offset, adj_offset, alpha, r);

            r = alpha * segrad + (1-alpha) * r;
	}

	/// TODO - check range of predictedAccel!
            r *= exp(0.1*learn->predictedAccel (segment));
            float ay= 0.5*(segment->angle[TR_YL] + segment->angle[TR_YR]);
            float ny= 0.5*(segment->next->angle[TR_YL] + segment->next->angle[TR_YR]);
            float py= 0.5*(segment->prev->angle[TR_YL] + segment->prev->angle[TR_YR]);
            float l = segment->length;
            float derp = (sin(ay)-sin(py));
            float dern = (sin(ny)-sin(ay));
            float der = .5*(dern+derp)/l;
            float adj = 1 + tanh(0.1*car->_speed_x * der);

            float bank = 0.5 * (segment->angle[TR_XS] + segment->angle[TR_XE]);
#if 1
            if (segment->type==TR_STR) {
		adj *= cos(bank);
            } else if (segment->type==TR_LFT) {
		adj *= 1 + tanh(-bank);
            } else {
		adj *= 1 + tanh(bank);
            }
#endif
            mu *= adj;
            float n_speed =  sqrt((mu*G*r)/(1.0 - MIN(1.0, r*CA*mu/mass)));
            return n_speed;
    }


    // Compute the length to the end of the segment.
    float Driver::getDistToSegEnd()
    {
	if (car->_trkPos.seg->type == TR_STR) {
            return car->_trkPos.seg->length - car->_trkPos.toStart;
	} else {
            return (car->_trkPos.seg->arc - car->_trkPos.toStart)*car->_trkPos.seg->radius;
	}
    }


    /// Compute fitting acceleration.
    float Driver::getAccel()
    {
	if (car->_gear > 0) {
            float allowedspeed = getAllowedSpeed(car->_trkPos.seg);
            if (1)
		{
                    tTrackSeg *segptr = car->_trkPos.seg;
                    float mu = segptr->surface->kFriction;
                    float maxlookaheaddist = currentspeedsqr/(2.0*mu*G);
                    float lookaheaddist = getDistToSegEnd();
			
                    segptr = segptr->next;
                    while (lookaheaddist < maxlookaheaddist) {
                        float pallowedspeed = getAllowedSpeed(segptr);
                        float delta = (brakedist(pallowedspeed, mu) - lookaheaddist);
                        //printf("%f\n", delta);
                        float u = MAX(0.1f, getSpeed());
                        float t = ACCELERATOR_LETGO_TIME + (1.0f - speed_factor);
                        if (overtaking) {
                            t = -0.1f;
                        }
                        if (-delta/u<t) {
                            if (pallowedspeed < allowedspeed) {
                                allowedspeed = pallowedspeed;
                            }
                        }
                        lookaheaddist += segptr->length;
                        segptr = segptr->next;
                    }
		}
            float max_allowed;
            switch(race_type) {
            case RM_TYPE_PRACTICE:
                max_allowed = 1.2f*max_speed_list[car->_trkPos.seg->id]; 
                break;
            case RM_TYPE_QUALIF:
                max_allowed = 1.2f*max_speed_list[car->_trkPos.seg->id]; 
                break;
            default:
                max_allowed = 1.2f* max_speed_list[car->_trkPos.seg->id]; 
                break;
            }

            if (allowedspeed > max_allowed) {
                allowedspeed = max_allowed;
            }
            current_allowed_speed = allowedspeed;
            float total_speed = sqrt(car->_speed_x*car->_speed_x + car->_speed_y*car->_speed_y);
            float delta = allowedspeed - (total_speed + FULL_ACCEL_MARGIN);
            float alpha = 0.5;
            float lambda = 2.0;
            //printf ("%.1f\n", delta);
            if (delta>0) {
                if (delta<lambda) {
                    float acc = alpha + (1-alpha)*delta/lambda;
                    //printf ("ac:%f\n", acc);
                    return acc;
                }
                //printf ("ac:1\n");
                return 1.0;//tanh(delta);
            } else {
                float acc = alpha*(1+delta/FULL_ACCEL_MARGIN);
                if (acc<0) acc = 0;
                //printf ("at:%f\n", acc);
                return acc;
            }
	} else {
            return 1.0;
	}
    }


    /// If we get lapped reduce accelerator.
    float Driver::filterOverlap(float accel)
    {
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & OPP_LETPASS) {
                return MIN(accel, 0.5);
            }
	}
	return accel;
    }


    /// Compute initial brake value.
    float Driver::getBrake()
    {
	// Car drives backward?
	if (car->_speed_x < -MAX_UNSTUCK_SPEED) {
            // Yes, brake.
            return 1.0;
	} else {
            // We drive forward, normal braking.
            tTrackSeg *segptr = car->_trkPos.seg;
            float mu = segptr->surface->kFriction;
            float maxlookaheaddist = currentspeedsqr/(2.0*mu*G);
            float lookaheaddist = getDistToSegEnd();

            float allowedspeed = getAllowedSpeed(segptr);
            if (allowedspeed < car->_speed_x) {
                float delta = (0.5f*(car->_speed_x - allowedspeed)/FULL_ACCEL_MARGIN);
                return (tanh(delta));
            }

            segptr = segptr->next;
            while (lookaheaddist < maxlookaheaddist) {
                allowedspeed = getAllowedSpeed(segptr);
                if (allowedspeed < car->_speed_x) {
                    float delta = (brakedist(allowedspeed, mu) - lookaheaddist);
                    if (delta>0) {
                        return tanh(0.1f*delta);
                    }
                }
                lookaheaddist += segptr->length;
                segptr = segptr->next;
            }
            return 0.0f;
	}
    }


    /**
       \brief Compute gear.
   
       Gear is shifted upwards in two cases:

       1) When we almost reach the revs limiter.
       2) When the overall output torque by shifting upwards will be greater than the current overall output torque.

       Gear is shifted down when our revs by shifting down would be
       somewhat below the red line at the current speed.
   
    */
    int Driver::getGear()
    {
	if (car->_gear <= 0) {
            return 1;
	}
	float gr_up = car->_gearRatio[car->_gear + car->_gearOffset];
	float omega = car->_enginerpmRedLine/gr_up;
	float wr = car->_wheelRadius(2);
	//float rpm = car->_enginerpm;
	int next_gear = car->_gear + 1;
	if (next_gear>car->_gearNb) {
            next_gear = car->_gear;
	}
	
	float next_ratio = car->_gearRatio[next_gear + car->_gearOffset];
	float next_rpm = next_ratio * car->_speed_x / wr;
	float rpm = gr_up * car->_speed_x / wr;//car->_enginerpm;
	if (omega*wr*SHIFT < car->_speed_x) {
            return car->_gear + 1;
	} else if (EstimateTorque(next_rpm)*next_ratio > EstimateTorque(rpm)*gr_up) {
            return car->_gear + 1;
	} else {
            float gr_down = car->_gearRatio[car->_gear + car->_gearOffset - 1];
            float prev_rpm = gr_down * (car->_speed_x) / wr;

            if (prev_rpm < car->_enginerpmMaxPw * SHIFT
                && car->_gear > 1
                && (EstimateTorque (prev_rpm) * gr_down
                    > EstimateTorque (rpm) * gr_up)) {
                return car->_gear - 1;
            }
	}
	return car->_gear;
    }

    /**
       \brief Estimate torque from max torque and max power ratings.

       Uses a piecewise linear model to estimate torque, given the value
       of torque at the max torque and power points and assuming that the
       torque drops to 0 at 0 revs and that it decays linearly after the
       maximum power at a rate 4 times than that of the decay between
       maximum torque and maximum power.
    */
    float Driver::EstimateTorque (float rpm)
    {
#if 0
	float MaxPw = 350000;
	float rpmMaxPw = 7000.0f * (2.0f*M_PI/60.0f);
	float MaxTq = 536.0;
	float rpmMaxTq = 5000.0f * (2.0f*M_PI/60.0f);
#else
	float MaxPw = car->_engineMaxPw;
	float rpmMaxPw = car->_enginerpmMaxPw;
	float MaxTq = car->_engineMaxTq;
	float rpmMaxTq = car->_enginerpmMaxTq;
#endif
	float rpmMax = car->_enginerpmMax;
	float TqAtMaxPw = MaxPw/rpmMaxPw;
	float PwAtMaxRevs = .5*MaxPw;
	float TqAtMaxRevs = PwAtMaxRevs/rpmMax;
	//printf ("pw-estimated Tq at maxrevs: %f@%f\n", TqAtMaxRevs, car->_enginerpmMax);
	
	
	float t [] = {
            0.0,
            MaxTq,
            TqAtMaxPw,
            TqAtMaxRevs,
            0.0
	};
	float a [] = {		
            0.0,
            rpmMaxTq,
            rpmMaxPw,
            rpmMax,
            rpmMax*2.0
	};
	int N = 5;
	for (int i=0; i<N - 1; i++) {
            if (rpm>a[i] && rpm<=a[i+1]) {
                float d = (a[i+1]-a[i]);
                float dr = (rpm - a[i]);
                float D = (dr/d);
                float est_1 = t[i] * (1-D) + D * t[i+1];
                //float est_2 = (t[i]*a[i]*(1-D) + D*t[i+1]*a[i+1])/rpm; 
                return est_1;

            }
	}
	// if rpm < 0, or rpm >maxrpm
	return 0.0;
    }

    /// Compute steer value.
    float Driver::getSteer()
    {
	float avoidance = 0.0f;
	if (!pit->getInPit()) {
            if (car->_trkPos.toRight < car->_dimension_y) {
                avoidance = tanh(STEER_AVOIDANCE_GAIN*(car->_dimension_y - car->_trkPos.toRight));
            } else 	if (car->_trkPos.toLeft < car->_dimension_y) {
                avoidance = tanh(STEER_AVOIDANCE_GAIN*(car->_trkPos.toLeft - car->_dimension_y));
            }
	}

	v2d target = getTargetPoint();
	float targetAngle = atan2(target.y - car->_pos_Y, target.x - car->_pos_X);
	float steer_direction = STEER_DIRECTION_GAIN * (targetAngle - car->_yaw - STEER_PREDICT_GAIN * car->_yaw_rate);

	// over steer correction a la berniw
	tTrackSeg *seg = car->_trkPos.seg;
	float omega = 0.0;
	if (seg->type==TR_LFT) {
            omega = getSpeed()/radius[seg->id];
	} else if (seg->type == TR_RGT) {
            omega = -getSpeed()/radius[seg->id];
	}
	float correct_oversteer = 0.0f;//STEER_PREDICT_GAIN * (omega - car->_yaw_rate);


	float correct_drift = - STEER_DRIFT_GAIN * atan2(car->_speed_Y, car->_speed_X);

	NORM_PI_PI(steer_direction);
	return avoidance + correct_oversteer + correct_drift + steer_direction/car->_steerLock;
    }


    /// Compute the clutch value.
    float Driver::getClutch()
    {
	if (car->_gear > 1) {
            clutchtime = 0.0;
            return 0.0;
	} else {
            float drpm = car->_enginerpm - car->_enginerpmRedLine/2.0;
            clutchtime = MIN(CLUTCH_FULL_MAX_TIME, clutchtime);
            float clutcht = (CLUTCH_FULL_MAX_TIME - clutchtime)/CLUTCH_FULL_MAX_TIME;
            if (car->_gear == 1 && car->_accelCmd > 0.0) {
                clutchtime += (float) RCM_MAX_DT_ROBOTS;
            }

            if (drpm > 0) {
                float speedr;
                if (car->_gearCmd == 1) {
                    // Compute corresponding speed to engine rpm.
                    float omega = car->_enginerpmRedLine/car->_gearRatio[car->_gear + car->_gearOffset];
                    float wr = car->_wheelRadius(2);
                    speedr = (CLUTCH_SPEED + MAX(0.0, car->_speed_x))/fabs(wr*omega);
                    float clutchr = MAX(0.0, (1.0 - speedr*2.0*drpm/car->_enginerpmRedLine));
                    return MIN(clutcht, clutchr);
                } else {
                    // For the reverse gear.
                    clutchtime = 0.0;
                    return 0.0;
                }
            } else {
                return clutcht;
            }
	}
    }

    /// Compute target point for steering.
    v2d Driver::getTargetPoint()
    {
	tTrackSeg *seg = car->_trkPos.seg;
	tTrackSeg *init_seg = seg;
	float lookahead;
	float length = getDistToSegEnd();
	float offset = getOffset();
	if (pit->getInPit()) {
            // To stop in the pit we need special lookahead values.
            if (currentspeedsqr > pit->getSpeedlimitSqr()) {
                lookahead = PIT_LOOKAHEAD + car->_speed_x*LOOKAHEAD_FACTOR;
            } else {
                lookahead = PIT_LOOKAHEAD;
            }
	} else {
            // Usual lookahead.
            lookahead = LOOKAHEAD_CONST + car->_speed_x*LOOKAHEAD_FACTOR;
            // Prevent "snap back" of lookahead on harsh braking.
            float cmplookahead = oldlookahead - car->_speed_x*RCM_MAX_DT_ROBOTS;
            if (lookahead < cmplookahead) {
                lookahead = cmplookahead;
            }
	}

	oldlookahead = lookahead;

	// Search for the segment containing the target point.
	while (length < lookahead) {
            seg = seg->next;
            length += seg->length;
	}

	length = lookahead - length + seg->length;
	float fromstart = seg->lgfromstart;
	fromstart += length;


	float alpha, nalpha;
	if (USE_NEW_ALPHA) {
            alpha = seg_alpha_new[seg->id];
            nalpha = seg_alpha_new[seg->next->id];
	} else {
            alpha = seg_alpha[seg->id];
            nalpha = seg_alpha[seg->next->id];
	}

	float beta = length/seg->length;
	//printf ("%f %f %f\n", beta, length, seg->length);

	//if (pit->getInPit()) {
	//if (0) {//pit_exit_timer < 0.2f) {
	if (pit->getState() == IN_LANE) {
            alpha = 0.5;
            nalpha = 0.5;
            beta  = 0.0;
            //printf ("t: %f\n", pit_exit_timer);
	}

	// Compute the target point.
	offset = myoffset = pit->getPitOffset(offset, fromstart);

	v2d s;

	if (1) {
            s.x = (alpha*seg->vertex[TR_SL].x
                   + (1-alpha)*seg->vertex[TR_SR].x);
		
            s.y = (alpha*seg->vertex[TR_SL].y
                   +(1-alpha)*seg->vertex[TR_SR].y);
	} else if (1) {
            s.x =(1-beta)*(alpha*seg->vertex[TR_SL].x
                           +(1-alpha)*seg->vertex[TR_SR].x)
                +beta*(nalpha*seg->vertex[TR_EL].x
                       +(1-nalpha)*seg->vertex[TR_ER].x);
		
            s.y =(1-beta)*(alpha*seg->vertex[TR_SL].y
                           +(1-alpha)*seg->vertex[TR_SR].y)
                + beta*(nalpha*seg->vertex[TR_EL].y
                        +(1-nalpha)*seg->vertex[TR_ER].y);
	} else {
            s.x = 0.0;
            s.y = 0.0;
            //float sum = 0.0;
            float cnt = 0.0;
            while (init_seg != seg->next) {
                float alpha2 = seg_alpha[init_seg->id];
                float nalpha2 = seg_alpha[init_seg->next->id];
                s.x *= .25;
                s.y *= .25;
                cnt *= .25;
                s.x +=(1-beta)*(alpha2*init_seg->vertex[TR_SL].x
                                +(1-alpha2)*init_seg->vertex[TR_SR].x)
                    +beta*(nalpha2*init_seg->vertex[TR_EL].x
                           +(1-nalpha2)*init_seg->vertex[TR_ER].x);
			
                s.y +=(1-beta)*(alpha2*init_seg->vertex[TR_SL].y
                                +(1-alpha2)*init_seg->vertex[TR_SR].y)
                    + beta*(nalpha2*init_seg->vertex[TR_EL].y
                            +(1-nalpha2)*init_seg->vertex[TR_ER].y);
                cnt+=1.0;
                init_seg = init_seg->next;
            }
            s.x /= (cnt);
            s.y /= (cnt);
	}

	if ( seg->type == TR_STR) {
            float adj_offset = offset;
            v2d d, n;
            n.x = (seg->vertex[TR_EL].x - seg->vertex[TR_ER].x)/seg->length;
            n.y = (seg->vertex[TR_EL].y - seg->vertex[TR_ER].y)/seg->length;
            n.normalize();
            tTrackSeg* pseg = seg->prev;
            tTrackSeg* nseg = seg->next;
            float  nalpha;
            float  calpha;
            if (USE_NEW_ALPHA) {
                nalpha = seg_alpha_new[nseg->id];
                calpha = seg_alpha_new[seg->id];
            } else {
                nalpha = seg_alpha[nseg->id];
                calpha = seg_alpha[seg->id];
            }
            if (0){// !pit->getInPit()) { // or use '!' ?
                d.x = (seg->vertex[TR_EL].x - pseg->vertex[TR_SL].x)/seg->length;
                d.y = (seg->vertex[TR_EL].y - pseg->vertex[TR_SL].y)/seg->length;
            } else {
                // re-adjust offset
                adj_offset =  tanh(fabs(offset)) * ((0.5*seg->width + offset)-calpha*seg->width);
                if (pit->getInPit()) {
                    adj_offset = offset;
                }

#if 1
                //adj_offset += (1-tanh(fabs(offset))) * (calpha-0.5)*seg->width;
                float startx = calpha * seg->vertex[TR_SL].x
                    + (1-calpha) * seg->vertex[TR_SR].x;
                float starty = calpha * seg->vertex[TR_SL].y
                    + (1-calpha) * seg->vertex[TR_SR].y;
                float endx = nalpha * seg->vertex[TR_EL].x
                    + (1-nalpha) * seg->vertex[TR_ER].x;
                float endy = nalpha * seg->vertex[TR_EL].y
                    + (1-nalpha) * seg->vertex[TR_ER].y;


                d.x = (endx - startx)/seg->length;
                d.y = (endy - starty)/seg->length;
#else
                d.x = (-calpha*seg->vertex[TR_SL].x
                       + nalpha * seg->vertex[TR_EL].x
                       - (1-calpha) * seg->vertex[TR_SR].x
                       + (1-nalpha) * seg->vertex[TR_ER].x)/seg->length;
                d.y = (-calpha * seg->vertex[TR_SL].y
                       + nalpha * seg->vertex[TR_EL].y
                       - (1-calpha) * seg->vertex[TR_SR].y
                       + (1-nalpha) * seg->vertex[TR_ER].y)/seg->length;
#endif
            }
            return s + d*length + adj_offset*n;
	} else {
            float calpha;
            if (USE_NEW_ALPHA) {
                calpha = seg_alpha_new[seg->id];
            } else {
                calpha = seg_alpha[seg->id];
            }
            // negative offset = right
            // when tanh() -> 1, total offset is negating calpha offset! 
            float adj_offset =  tanh(fabs(offset)) * ((0.5*seg->width + offset)-calpha*seg->width);
            //adj_offset += (1-tanh(fabs(offset))) * (calpha-0.5)*seg->width;
            v2d c, n;
            c.x = seg->center.x;
            c.y = seg->center.y;
            float arc = length/seg->radius;
            float arcsign = (seg->type == TR_RGT) ? -1.0 : 1.0;
            arc = arc*arcsign;
            s = s.rotate(c, arc);

            n = c - s;
            n.normalize();
            return s + arcsign*adj_offset*n;
	}
    }


    /// Compute offset to normal target point for overtaking or let pass an opponent.
    float Driver::getOffset()
    {
	int i;
	float catchdist, mincatchdist = FLT_MAX, mindist = -1000.0;
	Opponent *o = NULL;

	// Increment speed dependent.
	float incfactor = MAX_INC_FACTOR - MIN(fabs(car->_speed_x)/MAX_INC_FACTOR, (MAX_INC_FACTOR-1.0));

	// Let overlap.
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & OPP_LETPASS) {
                // Behind, larger distances are smaller ("more negative").
                if (opponent[i].getDistance() > mindist) {
                    mindist = opponent[i].getDistance();
                    o = &opponent[i];
                }
            }
	}

	//myoffset = -0.2f * car->_trkPos.seg->width;
	overtaking = false;
	if (o != NULL) {
            tCarElt *ocar = o->getCarPtr();
            float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
            float w = car->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
            if (side > 0.0) {
                if (myoffset < w) {
                    myoffset += OVERTAKE_OFFSET_INC*incfactor;
                }
            } else {
                if (myoffset > -w) {
                    myoffset -= OVERTAKE_OFFSET_INC*incfactor;
                }
            }
            //printf ("let overtake: %f\n", myoffset);
            return myoffset;
	}


	// Overtake.
	float time_to_overtake = OVERTAKE_TIME;
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & OPP_FRONT) {
                catchdist = opponent[i].getCatchDist();//MIN(opponent[i].getCatchDist(), opponent[i].getDistance()*CATCH_FACTOR);
                if (getSpeed() > 0) {
                    time_to_overtake = catchdist/getSpeed();
                    if (time_to_overtake < OVERTAKE_TIME) {
                        // consider overtaking if we approach in OVERTAKE_TIME
                        //printf ("t_overtake: %f\n", time_to_overtake);
                        if (catchdist < mincatchdist) {
                            mincatchdist = catchdist;
                            o = &opponent[i];					}
				 
                    } else if (opponent[i].getBrakeOvertake()>0.1f) {
                        // or if we recently were about to crash
                        //printf ("b_overtake: %f\n", opponent[i].getBrakeOvertake());
                        catchdist = opponent[i].getDistance();
                        if (catchdist < mincatchdist) {
                            mincatchdist = catchdist;
                            o = &opponent[i];
                        }
                    }
                }
            }
	}

	if (o != NULL) {
            overtaking = true;
            // Compute the width around the middle which we can use for overtaking.
            float w = o->getCarPtr()->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
            // Compute the opponents distance to the middle.
            float otm = o->getCarPtr()->_trkPos.toMiddle;
            // Define the with of the middle range.
            float wm = o->getCarPtr()->_trkPos.seg->width*CENTERDIV;

            if (time_to_overtake >0.0f) { 
                incfactor *= (1.0f + OVERTAKE_TIME) / (1.0f + time_to_overtake);
            } else {
                incfactor *= 2.0f;
            }

            if (otm > wm && myoffset > -w) {
                myoffset -= OVERTAKE_OFFSET_INC*incfactor;
            } else if (otm < -wm && myoffset < w) {
                myoffset += OVERTAKE_OFFSET_INC*incfactor;
            } else {
                // If the opponent is near the middle we try to move the offset toward
                // the inside of the expected turn.
                // Try to find out the characteristic of the track up to catchdist.
                tTrackSeg *seg = car->_trkPos.seg;
                float length = getDistToSegEnd();
                float oldlen, seglen = length;
                float lenright = 0.0, lenleft = 0.0;
                mincatchdist = MIN(mincatchdist, DISTCUTOFF);

                do {
#if 0
                    switch (seg->type) {
                    case TR_LFT:
                        lenleft += seglen;
                        break;
                    case TR_RGT:
                        lenright += seglen;
                        break;
                    default:
                        // Do nothing.
                        break;
                    }
#else
                    lenleft += seg_alpha[seg->id] * seglen;
                    lenright += (1.0f - seg_alpha[seg->id]) * seglen;
#endif
                    seg = seg->next;
                    seglen = seg->length;
                    oldlen = length;
                    length += seglen;
                } while (oldlen < mincatchdist);

                // If we are on a straight look for the next turn.
                if (lenleft == 0.0 && lenright == 0.0) {
                    while (seg->type == TR_STR) {
                        lenleft += 0.1f * seg_alpha[seg->id] * seg->length;
                        lenright += 0.1f * (1.0f - seg_alpha[seg->id]) * seg->length;
                        seg = seg->next;
                    }
                    // Assume: left or right if not straight.
                    if (seg->type == TR_LFT) {
                        lenleft += seg->length;
                    } else {
                        lenright += seg->length;
                    }
                }

                // Because we are inside we can go to the border.
                float maxoff = (o->getCarPtr()->_trkPos.seg->width - car->_dimension_y)/2.0-BORDER_OVERTAKE_MARGIN;
                if (lenleft > lenright) {
                    if (myoffset < maxoff) {
                        myoffset += OVERTAKE_OFFSET_INC*incfactor;
                    }
                } else {
                    if (myoffset > -maxoff) {
                        myoffset -= OVERTAKE_OFFSET_INC*incfactor;
                    }
                }
            }
	} else {
            // There is no opponent to overtake, so the offset goes slowly back to zero.
            float a = 1.0;
            if (myoffset > a*OVERTAKE_OFFSET_INC) {
                myoffset -= a*OVERTAKE_OFFSET_INC;
                //myoffset= 0.0;
            } else if (myoffset < -a*OVERTAKE_OFFSET_INC) {
                myoffset += a*OVERTAKE_OFFSET_INC;
                //myoffset= 0.0;
            } else {
                myoffset = 0.0;
            }
	}

	return myoffset;
    }


    /// Update my private data every timestep.
    void Driver::update(tSituation *s)
    {
	// Update global car data (shared by all instances) just once per timestep.
	if (currentsimtime != s->currentTime) {
            dt = s->currentTime - currentsimtime;
            if (dt<0.0) dt = 0.0;
            currentsimtime = s->currentTime;
            cardata->update();
	}

	// Update the local data rest.
	speedangle = mycardata->getTrackangle() - atan2(car->_speed_Y, car->_speed_X);
	NORM_PI_PI(speedangle);
	mass = CARMASS + car->_fuel;
	currentspeedsqr = car->_speed_x*car->_speed_x;
	opponents->update(s, this);
	strategy->update(car, s);
	speed_factor = strategy->getSpeedFactor (car, s, opponents);
	if (!pit->getPitstop()) {
            pit->setPitstop(strategy->needPitstop(car, s, opponents));
	}
	pit->update();
	if (pit->getInPit()) {
            pit_exit_timer = 0.0f;
	} else {
            pit_exit_timer += dt;
	}

	alone = isAlone();

	if (race_type != RM_TYPE_RACE) {
            learn->update(s, track, car, alone, myoffset,
                          car->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN,
                          radius, seg_alpha[car->_trkPos.seg->id],
                          car->_speed_x, current_allowed_speed);
	}
    }

    /// Check if we are alone
    int Driver::isAlone()
    {
	int i;
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & (OPP_COLL | OPP_LETPASS)) {
                alone_count = 0.0f;
            }
	}

	if (alone_count >= 2.0f) {
            return 1;	// Alone.
	} else {
            alone_count += dt;
            return 0;
	}
    }


    /// Check if I'm stuck.
    bool Driver::isStuck()
    {
	if (fabs(mycardata->getCarAngle()) > MAX_UNSTUCK_ANGLE &&
            car->_speed_x < MAX_UNSTUCK_SPEED &&
            fabs(car->_trkPos.toMiddle) > MIN_UNSTUCK_DIST) {
            if (stuck > MAX_UNSTUCK_COUNT && car->_trkPos.toMiddle*mycardata->getCarAngle() < 0.0) {
                return true;
            } else {
                stuck++;
                return false;
            }
	} else {
            stuck = 0;
            return false;
	}
    }


    /// Compute aerodynamic downforce coefficient CA.
    void Driver::initCa()
    {
	const char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	float rearwingarea = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGAREA, (char*) NULL, 0.0);
	float rearwingangle = GfParmGetNum(car->_carHandle, SECT_REARWING, PRM_WINGANGLE, (char*) NULL, 0.0);
	float wingca = 1.23*rearwingarea*sin(rearwingangle);

	float cl = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FCL, (char*) NULL, 0.0) +
            GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_RCL, (char*) NULL, 0.0);
	float h = 0.0;
	int i;
	for (i = 0; i < 4; i++)
            h += GfParmGetNum(car->_carHandle, WheelSect[i], PRM_RIDEHEIGHT, (char*) NULL, 0.20f);
	h*= 1.5; h = h*h; h = h*h; h = 2.0 * exp(-3.0*h);
	CA = h*cl + 4.0*wingca;
    }


    /// Compute aerodynamic drag coefficient CW.
    void Driver::initCw()
    {
	float cx = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_CX, (char*) NULL, 0.0);
	float frontarea = GfParmGetNum(car->_carHandle, SECT_AERODYNAMICS, PRM_FRNTAREA, (char*) NULL, 0.0);
	CW = 0.645*cx*frontarea;
    }


    /// Init the friction coefficient of the the tires.
    void Driver::initTireMu()
    {
	const char *WheelSect[4] = {SECT_FRNTRGTWHEEL, SECT_FRNTLFTWHEEL, SECT_REARRGTWHEEL, SECT_REARLFTWHEEL};
	float tm = FLT_MAX;
	int i;

	for (i = 0; i < 4; i++) {
            tm = MIN(tm, GfParmGetNum(car->_carHandle, WheelSect[i], PRM_MU, (char*) NULL, 1.0));
	}
	TIREMU = tm;
    }


    /// Reduces the brake value such that it fits the speed (more downforce -> more braking).
    float Driver::filterBrakeSpeed(float brake)
    {
	float weight = (CARMASS + car->_fuel)*G;
	float maxForce = weight + CA*MAX_SPEED*MAX_SPEED;
	float force = weight + CA*currentspeedsqr;
	return brake*force/maxForce;
    }

    /// Reduced
    float Driver::filterAPit(float accel)
    {
	if (pit->getPitstop() && !pit->getInPit()) {
            float dl, dw;
            RtDistToPit(car, track, &dl, &dw);
            if (dl < PIT_BRAKE_AHEAD) {
                float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
                accel = tanh(0.1*(dl-brakedist(0.0, mu)));
                if (accel < 0) {
                    accel = 0;
                }
#ifdef TEST_PITSTOPS
                printf ("apit(N):%f\n", accel);
#endif
            }
	}

	if (pit->getInPit()) {
            float s = pit->toSplineCoord(car->_distFromStartLine);
            if (pit->getPitstop() || s < pit->getNPitEnd()) { 
                float delta =  pit->getSpeedlimit() - getSpeed();
                if (delta>0) {
                    accel = tanh(delta);
                }
#ifdef TEST_PITSTOPS
                printf ("apit(Y):%f %f %f %f\n", accel, delta, pit->getSpeedlimit(), getSpeed());
#endif

            }
	}
	return accel;
    }

    // Brake filter for pit stop.
    float Driver::filterBPit(float brake)
    {
	if (pit->getPitstop() && !pit->getInPit()) {
            float dl, dw;
            RtDistToPit(car, track, &dl, &dw);
            if (dl < PIT_BRAKE_AHEAD) {
                pit->setState (APPROACHING);
                float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
                float bd = brakedist(0.0, mu);
                if (bd > dl) {
                    float br = tanh((bd-dl));
#ifdef TEST_PITSTOPS
                    printf ("Brakeahead:%f\n", br);
#endif
                    return br;
                }
            }
	}

	if (pit->getInPit()) {
            pit->setState (IN_LANE);
            float s = pit->toSplineCoord(car->_distFromStartLine);
            // Pit entry.
            if (pit->getPitstop()) {
                float mu = car->_trkPos.seg->surface->kFriction*TIREMU*PIT_MU;
                if (s < pit->getNPitStart()) {
                    // Brake to pit speed limit.
                    float dist = pit->getNPitStart() - s;
                    float b =  (brakedist(pit->getSpeedlimit(), mu) - dist);
                    if (b > 0) {
#ifdef TEST_PITSTOPS
                        printf ("pitstart:%f\n", tanh(b));
#endif
                        return tanh(b);

                    }
                } else {
                    // Hold speed limit.
                    if (currentspeedsqr > pit->getSpeedlimitSqr()) {
                        float br = pit->getSpeedLimitBrake(currentspeedsqr);
#ifdef TEST_PITSTOPS
                        printf ("limit:%f\n", br);
#endif
                        return br;
                    }
                }
                // Brake into pit (speed limit 0.0 to stop)
                float dist = pit->getNPitLoc() - s;
                if (pit->isTimeout(dist)) {
                    pit->setPitstop(false);
                    return 0.0;
                } else {
                    if (brakedist(0.0, mu) > dist) {
                        float bd = brakedist(0.0,mu)-dist;
#ifdef TEST_PITSTOPS
                        printf ("brakedist:%f\n", tanh(bd));
#endif
                        return tanh(bd);
                        //return 1.0;
                    } else if (s > pit->getNPitLoc()) {
                        // Stop in the pit.
                        return 1.0;
                    }
                }
            } else {
                pit->setState (PIT_EXIT);
                // Pit exit.
                if (s < pit->getNPitEnd()) {
                    // Pit speed limit.
                    if (currentspeedsqr > pit->getSpeedlimitSqr()) {
                        float br =  pit->getSpeedLimitBrake(currentspeedsqr);
#ifdef TEST_PITSTOPS
                        printf ("exit_limit:%f\n", br);
#endif
                        return br;
                    }
                }
            }
	}

	pit->setState (NONE);

	return brake;
    }


    /**
       Brake filter for collision avoidance.
   
       Given \f$u_0\f$, \f$u_t\f$, the velocities of our car and the
       opponent's, and the distance \f$d\f$ between each we calculate
       \f$s\f$, the minimum distance necessary to brake down to the
       opponent's velocity. It will take us \f$t\f$ seconds
       to perform this braking:
       \f[
       t = \frac{2s}{u_0+u_t}.
       \f]
       We also calculate the time to hit the other car if we do not brake.
       \f[
       t' = \frac{d}{u_0-u_t}
       \f]
   
       We then determine if we will be crashing or not by simply calculating
       \f[
       d_t = d + u_t t - s,
       \f]
       which is simply our current separation, plus the distance that will
       be travelled by the opponent, minus the distance that will be
       travelled by us. This quantity shows the minimum distance that we
       will ever have from the opponent if we hit the brakes now. 
   
       We define two constants related to \f$d_t\f$, \c
       MAX_BRAKE_FOLLOW_DISTANCE and \c MIN_BRAKE_FOLLOW_DISTANCE which
       determine the range we should keep from cars ahead of us.
   
    */

    float Driver::filterBColl(float brake)
    {
	float mu = car->_trkPos.seg->surface->kFriction;
	int i;
	float maxbrake = brake;
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & OPP_COLL) {
                float ut = opponent[i].getSpeed();
                float u0 = getSpeed();
                if (ut<u0 && u0 > 0.0f) {
                    float d = opponent[i].getDistance();
                    float s = brakedist(opponent[i].getSpeed(), mu);
                    float t = 2*s/(u0+ut);
                    float t_impact = d/(u0-ut);
                    float dt = d + ut*t - s;
                    float y = -1;
                    if (dt<2.0f * MAX_BRAKE_FOLLOW_DISTANCE || t_impact < OVERTAKE_TIME) {
                        //printf ("bcol: t_impact:%f dt:%f\n", t_impact, dt);
                        opponent[i].brakeForCollision();
                        if (dt<MAX_BRAKE_FOLLOW_DISTANCE) {
                            y = 1 - (dt-MIN_BRAKE_FOLLOW_DISTANCE)/(MAX_BRAKE_FOLLOW_DISTANCE-MIN_BRAKE_FOLLOW_DISTANCE);
                            if (y>0) {
                                maxbrake = SmoothMaxGamma (y, maxbrake, 0.5, 100.0);
                            } else {
                                // else we're going to crash anyway.. so..
                            }
                        }
                    }
                }
            }
	}
	//printf ("%f %f\n", brake, maxbrake);
	return maxbrake;
    }


    /// Steer filter for collision avoidance.
    float Driver::filterSColl(float steer)
    {
	int i;
	float sidedist = 0.0, fsidedist = 0.0, minsidedist = FLT_MAX;
	Opponent *o = NULL;

	// Get the index of the nearest car (o).
	for (i = 0; i < opponents->getNOpponents(); i++) {
            if (opponent[i].getState() & OPP_SIDE) {
                sidedist = opponent[i].getSideDist();
                fsidedist = fabs(sidedist);
                if (fsidedist < minsidedist) {
                    minsidedist = fsidedist;
                    o = &opponent[i];
                }
            }
	}

	// If there is another car handle the situation.
	if (o != NULL) {
            float d = fsidedist - o->getWidth();
            // Near, so we need to look at it.
            if (d < 2.0 * SIDECOLL_MARGIN) {
                /* compute angle between cars */
                tCarElt *ocar = o->getCarPtr();
                float diffangle = ocar->_yaw - car->_yaw;
                float diff = (SIDECOLL_MARGIN-d)/SIDECOLL_MARGIN;
                //printf ("side: %f angle:%f\n", d, diffangle);
                NORM_PI_PI(diffangle);
                // We are near and heading toward the car.
                // always turn..
                if (diffangle*o->getSideDist() < 0.0) {
                    const float c = SIDECOLL_MARGIN;//2.0;
                    d = d - c;
                    if (d < 0.0) {
                        d = 0.0;
                    }

                    float psteer = 0.01f * diffangle/car->_steerLock;
                    /**< Distance to middle of segment (+ to left - to right) */
                    float side = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
                    psteer = diff*tanh(0.1f*psteer+SIGN(side)*exp(-0.5* (fabs(o->getDistance()) + fabs(o->getSideDist()))));
                    myoffset = car->_trkPos.toMiddle;

                    float w = o->getCarPtr()->_trkPos.seg->width/WIDTHDIV-BORDER_OVERTAKE_MARGIN;
                    if (fabs(myoffset) > w) {
                        myoffset = (myoffset > 0.0) ? w : -w;
                    }

                    // Who is outside?
                    if (car->_trkPos.seg->type == TR_STR) {
                        if (fabs(car->_trkPos.toMiddle) > fabs(ocar->_trkPos.toMiddle)) {
                            psteer = steer*(d/c) + 1.5*psteer*(1.0-d/c);
                        } else {
                            psteer = steer*(d/c) + 2.0*psteer*(1.0-d/c);
                        }
                    } else {
                        float outside = car->_trkPos.toMiddle - ocar->_trkPos.toMiddle;
                        float sign = (car->_trkPos.seg->type == TR_RGT) ? 1.0 : -1.0;
                        if (outside*sign > 0.0) {
                            psteer = steer*(d/c) + 1.5*psteer*(1.0-d/c);
                        } else {
                            psteer = steer*(d/c) + 2.0*psteer*(1.0-d/c);
                        }
                    }

                    if (psteer*steer > 0.0 && fabs(steer) > fabs(psteer)) {
                        return steer;
                    } else {
                        return psteer;
                    }
                }
            }
	}
	return steer;
    }


    /// Antilocking filter for brakes.
    float Driver::filterABS(float brake)
    {
	if (car->_speed_x < ABS_MINSPEED) return brake;
	int i;
	float slip = 0.0;
	for (i = 0; i < 4; i++) {
            slip += car->_wheelSpinVel(i) * car->_wheelRadius(i);
	}
	slip = car->_speed_x - slip/4.0;
	if (slip > ABS_SLIP) {
            brake = brake - MIN(brake, (slip - ABS_SLIP)/ABS_RANGE);
	}
	return brake;
    }


    /// TCL filter for accelerator pedal.
    float Driver::filterTCL(float accel)
    {
	float slip = (this->*GET_DRIVEN_WHEEL_SPEED)() - car->_speed_x;
	TCL_status = 0.75 * TCL_status;
	if (TCL_status < 0.1) TCL_status = 0.0;
	if (slip > TCL_SLIP) {
            TCL_status +=  0.5 * (slip - TCL_SLIP)/TCL_RANGE;
	}
	accel = accel - MIN(accel, TCL_status);
	return accel;
    }


    /// Traction Control (TCL) setup.
    void Driver::initTCLfilter()
    {
	TCL_status = 0.0;
	char *traintype = GfParmGetStr(car->_carHandle, SECT_DRIVETRAIN, PRM_TYPE, VAL_TRANS_RWD);
	if (strcmp(traintype, VAL_TRANS_RWD) == 0) {
            GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_RWD;
	} else if (strcmp(traintype, VAL_TRANS_FWD) == 0) {
            GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_FWD;
	} else if (strcmp(traintype, VAL_TRANS_4WD) == 0) {
            GET_DRIVEN_WHEEL_SPEED = &Driver::filterTCL_4WD;
	}
    }


    /// TCL filter plugin for rear wheel driven cars.
    float Driver::filterTCL_RWD()
    {
	return (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
            car->_wheelRadius(REAR_LFT) / 2.0;
    }


    /// TCL filter plugin for front wheel driven cars.
    float Driver::filterTCL_FWD()
    {
	return (car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
            car->_wheelRadius(FRNT_LFT) / 2.0;
    }


    /// TCL filter plugin for all wheel driven cars.
    float Driver::filterTCL_4WD()
    {
	return ((car->_wheelSpinVel(FRNT_RGT) + car->_wheelSpinVel(FRNT_LFT)) *
                car->_wheelRadius(FRNT_LFT) +
                (car->_wheelSpinVel(REAR_RGT) + car->_wheelSpinVel(REAR_LFT)) *
                car->_wheelRadius(REAR_LFT)) / 4.0;
    }


    /** Hold car on the track.
	
    It has a bug
    */
    float Driver::filterTrk(tSituation* s, float accel)
    {
	tTrackSeg* seg = car->_trkPos.seg;
	int id = seg->id;
	float target_x;  // target toLeft
	if (USE_NEW_ALPHA) {
            target_x = seg_alpha_new[id];
	} else {
            target_x = seg_alpha[id];
	}
	float trackR = fabs(car->_trkPos.toRight);
	float trackL = fabs(car->_trkPos.toLeft);
	float actual_x = trackR/(trackL + trackR);

	seg_alpha_new[id] += 0.01*(actual_x-(seg_alpha_new[id]));
	//seg_alpha[id] -= 0.01*(actual_x-(seg_alpha_new[id]));

	if (USE_NEW_ALPHA) { // limit new alpha
            float margin  = 1.0;
            if (seg->type==TR_STR) {
                margin = 1.0;
            } else {
                margin = 2.0;
            }
            float lowlimit = margin*car->_dimension_y / track->width;
            float hilimit = 1-lowlimit;
            if (seg_alpha_new[id]<lowlimit) seg_alpha_new[id] = lowlimit;
            if (seg_alpha_new[id]>hilimit) seg_alpha_new[id] = hilimit;
	}
	float orig_steer = getSteer() + 0.1*(target_x-actual_x);


	// Steering filter definition: these values make a huge
	// difference.  Using for example 0.1, 0.0, 1.0 makes for slower
	// driving.. I am not sure why.  i.e. on e-track-3, mclarenf1, you
	// get times in 1:14-1:15 with the currently used values but
	// around 1:16 1:17 with the others.  the -0.1 term that subtracts
	// the predicted error is there to prevent the error from growing
	// too much.
	float dtm = -( 0.1 * orig_steer
                       - 0.1 * learn->predictedError(car)
                       + 0.2 * (target_x - actual_x));//(car->_trkPos.toMiddle);
	float tm = fabs(actual_x - target_x);
	//printf("%f %f %f %f %f\n", tm, actual_x - target_x, actual_x - seg_alpha_new[id], target_x, seg_alpha_new[id]);

	if (accel>0) {
            accel = filterTCL(accel);
	}
        float speed_to_inside = 0.0;
	if (car->_trkPos.toMiddle*speedangle > 0.0) {
            speed_to_inside = 0.5;
	}
	if (car->_speed_x < MAX_UNSTUCK_SPEED ||		// Too slow.
            pit->getInPit())// ||							// Pit stop.
            //car->_trkPos.toMiddle*speedangle > 0.0)		// Speedvector points to the inside of the turn.
            {
		float w = car->_dimension_y/seg->width;
		if (car->_speed_x < MAX_UNSTUCK_SPEED) {
                    learn->updateAccel (s, car, -1.0, tm-w,dtm);
		}
		return accel;
            }
	float accident = 0.0;
	float margin = fabs(car->_trkPos.toMiddle) - 0.5*seg->width;
	if (margin > 0.0) {
            if (margin > .5*car->_dimension_y) {
                accident = -1;
            }
            if (car->_trkPos.toRight<car->_dimension_y) {
                dtm -= 10.0*tanh((car->_dimension_y - car->_trkPos.toRight));
            } else 	if (car->_trkPos.toLeft<car->_dimension_y) {
                dtm -= 10.0*tanh((car->_trkPos.toLeft - car->_dimension_y));
            }
            //printf ("%f %f %f\n", -orig_steer, dtm, margin);
			
	}
	//	printf ("%.1f %.1f %.1f %.1f | ",
	//prev_toleft, prev_toright,
	//u_toleft, u_toright);
	float uleft = 0; 

	float uright = 0;
	if (dt > 0.001) {
            uleft = (car->_trkPos.toLeft - prev_toleft)/dt;
            uright = (car->_trkPos.toRight - prev_toright)/dt;
	}
	u_toleft = .9*u_toleft + 0.1*uleft;
	u_toright = .9*u_toright + 0.1*uright;
	prev_toleft = car->_trkPos.toLeft;
	prev_toright = car->_trkPos.toRight;
	float danger = 1000.0; // larger, less danger
	float steer_adjust = 0.0;
			
	if ((seg->type == TR_RGT) ||(car->_steerCmd < 0.1f)) {
            //printf ("RIGHT: %f\n", car->_steerCmd);
            if (u_toleft<0) {
                danger = -car->_trkPos.toLeft/u_toleft;
                steer_adjust = -1.0/(1.0 + fabs(danger));
                //printf ("LCR: %fs %d\n", danger, seg->id);
            } else if (u_toright>0) {
                danger = -2.0*car->_trkPos.toRight/u_toright;
                steer_adjust = 0.1f;//(1.0 + fabs(danger));
                //printf ("RCR: %fs %d\n", danger, seg->id);
            }
	}
	if ((seg->type == TR_LFT)||(car->_steerCmd > 0.1f)) {
            //printf ("LEFT: %f\n", car->_steerCmd);
            if (u_toright<0) {
                danger = -car->_trkPos.toRight/u_toright;
                steer_adjust = 1.0f/(1.0f+fabs(danger));
                //printf ("RCL: %fs %d\n", danger, seg->id);
            } else if (u_toleft<0) {
                danger = -2.0f*car->_trkPos.toLeft/u_toleft;
                steer_adjust = -0.1f;//(1.0+fabs(danger));
                //printf ("LCL: %fs %d\n", danger, seg->id);
            }
	}
	float danger_accel = 0.0;
	if (danger>0) {
            if (danger<0.5) {
                danger_accel = -0.5 - 2*(0.5-danger);
                car->_steerCmd += STEER_EMERGENCY_GAIN * steer_adjust;
            } else if (danger<1.0) {
                danger_accel = 0.5*(danger-1);
                car->_steerCmd += STEER_EMERGENCY_GAIN * (danger-2) * steer_adjust;
            } 
	}
	float Kgamma = car->_pitch;
	{
            tTrackSeg* tseg = seg;
            for (float l=0.0f; l<50.0f; l+=tseg->length, tseg=tseg->next) {
                float ay= 0.5*(tseg->angle[TR_YL] + tseg->angle[TR_YR]);
                float ny= 0.5*(tseg->next->angle[TR_YL] + tseg->next->angle[TR_YR]);
                float py= 0.5*(tseg->prev->angle[TR_YL] + tseg->prev->angle[TR_YR]);
                float tmp = -0.25 * (2*ay + ny + py);
                if (tseg->type!=TR_STR) {
                    tmp *= 2.0;
                }
                Kgamma = MAX(Kgamma, tmp);
            }
	}
	float deltapitch = MAX(Kgamma - car->_pitch, 0.0);
	float flyspeed = MAX(50, getSpeed());
	float fly_mode = (1.0/(5.0 + flyspeed)- deltapitch);
	if (fly_mode < -0.1) {
            // only let go if we have to let go a lot
            float cut_accel =  2 * tanh(fly_mode);
            if (cut_accel < -1) {
                danger_accel += 2 * tanh(fly_mode);
            }
	}

	if (seg->type == TR_STR) {
            float w = car->_dimension_y/seg->width ;
            if (tm > w) {
                accel += learn->updateAccel (s, car, accident, tm-w,dtm);
            } else {
                accel += learn->updateAccel (s, car, accident, tm-w,dtm);
            }
            return accel + danger_accel;
	} else {
            float w = 1.0/WIDTHDIV;
            if (tm > w) {
                accel += learn->updateAccel (s, car, accident, tm-w, dtm);
                //accel = 0.0;
            } else {
                accel += learn->updateAccel (s, car, accident, tm-w,dtm);
            }
            return accel + danger_accel;
	}
    }


    /// Compute the needed distance to brake.
    float Driver::brakedist(float allowedspeed, float mu)
    {
	float c,d;
	if (pit->getInPit()) {
            c = mu*G;
            d = (CA*mu + CW)/mass;
            //float dm=learn->GetFrictionDm(car->_trkPos.seg);
            //float dm2=learn->GetFrictionDm2(car->_trkPos.seg);
	} else {
            //printf ("%f %f\n", learn->GetFrictionDm(car->_trkPos.seg)
            //, learn->GetFrictionDm2(car->_trkPos.seg));
            c = mu*G + learn->GetFrictionDm(car->_trkPos.seg);
            d = (CA*mu + CW)/mass;
            //+ learn->GetFrictionDm2(car->_trkPos.seg))/mass;
            //+ learn->GetFrictionDm3(car->_trkPos.seg))/mass;
	}
	float v1sqr = currentspeedsqr;
	float v2sqr = allowedspeed*allowedspeed;
	return -log((c + v2sqr*d)/(c + v1sqr*d))/(2.0*d);
    }



    /**
       \brief prepare an initial path through the track.

       This function uses steepest gradient descent to create a path with
       the minimum lateral acceleration. The cost function is not directly
       related to the lateral acceleration, unfortunately - I make some
       simplifying assumptions, so it might not work well for all tracks.

    */
    void Driver::prepareTrack()
    {
	int N=track->nseg;

	seg_alpha = new float [N];
	seg_alpha_new = new float [N];

	int i;
	for (i=0; i<N; i++) {
            seg_alpha[i] = 0.5;
            seg_alpha_new[i] = 0.5;
	}
    
        TrackData track_data;
        Trajectory trajectory;
        SegmentList segment_list;

        track_data.setStep(10.0f);
        track_data.setWidth(10.0f);
    
	tTrackSeg* seg = track->seg;
	seg = track->seg;
        float length_limit = 10.0;
        float current_length = length_limit;
        int j = 0;
	for (i=0; i<N; i++, seg=seg->next) {
            seg_alpha[seg->id] = 0.5;
            current_length += seg->length;
            if (current_length >= length_limit) {
                Point left(seg->vertex[TR_SL].x, seg->vertex[TR_SL].y);
                Point right(seg->vertex[TR_SR].x, seg->vertex[TR_SR].y);
                segment_list.Add(Segment(left, right));
                current_length = 0.0;
                j++;
            }
	}

        trajectory.Optimise(segment_list, 500, 0.02f, "/tmp/result");
        seg = track->seg;
        current_length = length_limit;

        {
            // copy optimised values
            tTrackSeg* seg = track->seg;
            Trajectory trajectory2;
            SegmentList segment_list2;
            trajectory2.w.resize(N);
            int i,j;
            j = 0;
            for (i=0; i<N; i++, seg=seg->next) {
                float w =  trajectory.w[j];
                seg_alpha[seg->id] = w;
                trajectory2.w[i] = w;
                Point left(seg->vertex[TR_SL].x, seg->vertex[TR_SL].y);
                Point right(seg->vertex[TR_SR].x, seg->vertex[TR_SR].y);
                segment_list2.Add(Segment(left, right));
                current_length += seg->length;
                if (current_length >= length_limit) {
                    j++;
                    if (j>=(int) trajectory.w.size()) {
                        j = 0;
                    }
                    current_length = 0.0;
                }
            }
            trajectory2.Optimise(segment_list2, 1000, 0.005f, "/tmp/result2", false);
            seg = track->seg;
            for (i=0; i<N; i++, seg=seg->next) {
                seg_alpha[seg->id] = trajectory2.w[i];
            }
            seg = track->seg;
            for (i=0; i<N; i++, seg = seg->next) {
                int j = seg->id;
                ideal_radius[j] = 1.0f / trajectory2.accel[i];
            }
        }




	if (0) {
            //printf ("Estimating least square error radius for smoothed points.\n");
            tTrackSeg* seg = track->seg;
            int prev_type = -track->seg->type;
            float prev_rad = seg->radius;
            int cnt=0;
            int max_cnt = 5;
            float max_length = 50.0f;
            tTrackSeg* next_eval = seg;
            for (int i=0; i<N; i++, seg=seg->next) {
                float drad = 0;
                //printf ("%f ", darc);
                //printf("%f\n", seg->arc);
                prev_rad = seg->radius;
                prev_type = seg->type;
                tTrackSeg* prev_seg = seg;
                // look back until we find a segment of different type or
                // radius or until the total length>max_length and we have looked
                // at more than max_cnt points
                float length = max_length;
                cnt = max_cnt;
                drad = 0.0f;
                while((length>0 || cnt>0)
                      && (prev_seg->type==seg->type)
                      && (drad < 0.01f || cnt>0)) {
                    prev_seg = prev_seg->prev;
                    length -= prev_seg->length;
                    drad = fabs(prev_seg->radius - seg->radius);
                    cnt--;
                }
                //printf ("%f<=0 %d<=0 %d!=%d %f<0.01 ?\n",
                //length, cnt, prev_seg->type, seg->type, drad);
                // look forward with same conditions as above
                tTrackSeg* next_seg = seg;
                length = max_length;
                cnt = max_cnt;
                drad = 0.0;
                while((length>0 || cnt>0)
                      && (next_seg->type==seg->type)
                      && (drad < 0.01 || cnt >0)) {
                    next_seg = next_seg->next;
                    length -= next_seg->length;
                    drad = fabs(next_seg->radius - seg->radius);
                    cnt--;
                }
			
                //printf ("%d %d %d\n", prev_seg->id, seg->id, next_seg->id);
                if (next_eval == seg) {
                    if (seg->type==TR_STR) {
                        for (tTrackSeg* s = prev_seg->next; s != next_seg; s=s->next) {
                            ideal_radius[s->id] = 10000;
                        }
                    } else {
                        float r = EstimateRadius (seg, prev_seg, next_seg->next);
                        for (tTrackSeg* s = prev_seg->next; s != next_seg; s=s->next) {
                            ideal_radius[s->id] = r;
                        }
                    }
                    next_eval = next_seg;
                }
                //printf ("%f %f\n", r, ideal_radius[seg->id]);
            }
            //printf ("Done\n");
	}
    }

    /// Show the paths
    void Driver::ShowPaths()
    {
	tTrackSeg* seg = track->seg;
	int N = track->nseg;
	
	FILE* fplan = fopen("/tmp/track_plan", "w");
	FILE* fpath = fopen("/tmp/track_path", "w");
	//FILE* fpathnew = fopen("/tmp/track_path_new", "w");
	seg = track->seg;
	for (int i=0; i<N; i++, seg=seg->next) {
            int id = seg->id;
            //float xle = seg->vertex[TR_EL].x;
            //float yle = seg->vertex[TR_EL].y;
            //float xre = seg->vertex[TR_ER].x;
            //float yre = seg->vertex[TR_ER].y;
            float xls = seg->vertex[TR_SL].x;
            float yls = seg->vertex[TR_SL].y;
            float xrs = seg->vertex[TR_SR].x;
            float yrs = seg->vertex[TR_SR].y;
            //float xlm = 0.5*(xls+xle);
            //float ylm = 0.5*(yls+yle);
            //float xrm = 0.5*(xrs+xre);
            //float yrm = 0.5*(yrs+yre);
            fprintf (fplan, "%f %f %f %f %d\n", xls, yls, xrs, yrs, id);
            //fprintf (fplan, "%f %f %f %f %d\n", xlm, ylm, xrm, yrm, id);
            //fprintf (fplan, "%f %f %f %f %d\n", xle, yle, xre, yre, id);
            float a = seg_alpha[seg->id];
            fprintf (fpath, "%f %f %d\n", xls*a+(1-a)*xrs, yls*a+(1-a)*yrs, id);
            //fprintf (fpath, "%f %f %d\n", xlm*a+(1-a)*xrm, ylm*a+(1-a)*yrm, id);
            //fprintf (fpath, "%f %f %d\n", xle*a+(1-a)*xre, yle*a+(1-a)*yre, id);
            a = seg_alpha_new[seg->id];
            //fprintf (fpathnew, "%f %f %d\n", xls*a+(1-a)*xrs, yls*a+(1-a)*yrs, id);
            //fprintf (fpathnew, "%f %f %d\n", xlm*a+(1-a)*xrm, ylm*a+(1-a)*yrm, id);
            //fprintf (fpathnew, "%f %f %d\n", xle*a+(1-a)*xre, yle*a+(1-a)*yre, id);
	}
	//fclose(fpathnew);
	fclose(fpath);
	fclose(fplan);
    }


    /**
     *  \brief Given a segment, estimate the radius of the optimal path through it.
     *
     *  This version tries to fit a sphere via mean-square error minimisation.
     */
    float Driver::EstimateRadius (tTrackSeg* seg, tTrackSeg* prev_seg, tTrackSeg* next_seg) 
    {

	std::vector<Vector> vmatrix;
	ParametricSphere curve(2);
	for (tTrackSeg* s = prev_seg; s!=next_seg; s=s->next) {
            Vector optimal(2);
            //int i = s->id;
            float alpha = seg_alpha[s->id];
            //alpha = 0.0;
            optimal[0] = s->vertex[TR_SL].x * alpha
                + s->vertex[TR_SR].x * (1 - alpha);
            optimal[1] = s->vertex[TR_SL].y * alpha
                + s->vertex[TR_SR].y * (1 - alpha);
            vmatrix.push_back(optimal);
	}
	curve.C->x[0] = seg->center.x;
	curve.C->x[1] = seg->center.y;
	//printf ("%f %f\n", curve.C->x[0], curve.C->x[1]);
	//printf ("%f %f\n", (*curve.C)[0], (*curve.C)[1]);
	curve.r = ideal_radius[seg->id];//seg->radius ;
	EstimateSphere (vmatrix, &curve);
	return curve.r;
    }


    /**
       \brief Given a segment, estimate the radius of the optimal path through it.
       This version attempts to fit a sphere exactly.
    */
    float Driver::EstimateRadius2 (tTrackSeg* seg) 
    {

	std::vector<Vector> vmatrix;
	seg=seg->prev;
	for (int i=0; i<3; i++, seg=seg->next->next) {
            Vector optimal(2);
            //int i = s->id;
            float alpha = seg_alpha[seg->id];
            //alpha = 0.0;
            optimal[0] = seg->vertex[TR_SL].x * alpha
                + seg->vertex[TR_SR].x * (1 - alpha);
            optimal[1] = seg->vertex[TR_SL].y * alpha
                + seg->vertex[TR_SR].y * (1 - alpha);
            vmatrix.push_back(optimal);
	}
	return CalculateRadiusPoints (vmatrix);
    }




    /// Given a circle and a segment, find target path
    float Driver::FindCurveTarget(tTrackSeg* seg, Vector* C, float rmax)
    {
	Vector inside(2);
	Vector outside(2);
	
	if (seg->type==TR_LFT) {
            inside.x[0] = seg->vertex[TR_SL].x;
            inside.x[1] = seg->vertex[TR_SL].y;
            outside.x[0] = seg->vertex[TR_SR].x;
            outside.x[1] = seg->vertex[TR_SR].y;
	} else {
            inside.x[0] = seg->vertex[TR_SR].x;
            inside.x[1] = seg->vertex[TR_SR].y;
            outside.x[0] = seg->vertex[TR_SL].x;
            outside.x[1] = seg->vertex[TR_SL].y;
	}

	ParametricLine R(&inside, &outside);
	Vector* intersect = IntersectSphereLine(&R, C, rmax);
	float target = .5;
	bool flag = false;
	//	if (intersect->N==0) {
	//		printf ("oops:");
	//	}

	for (int j=0; j<intersect->Size(); j++) {
            if ((intersect->x[j]>=0) && (intersect->x[j]<=1)) {
                flag = true;
                if (seg->type==TR_LFT) {
                    target = 1 -intersect->x[j];
                } else {
                    target = intersect->x[j];
                }
            } else {
                if (flag==false) {
                    float dxj=0.5;
                    if (intersect->x[j]<0) {
                        dxj=0;
                    }
                    if (intersect->x[j]>1) {
                        dxj=1;
                    }
                    if (seg->type==TR_LFT) {
                        target = 1 - dxj;
                    } else {
                        target = dxj;
                    }
                }
            }
	}



	delete intersect;
	return target;
    }

    /// Find circle from previous or next curve intersecting the straight.
    float Driver::FindStraightTarget(tTrackSeg* curve, tTrackSeg* seg, Vector* C, float rmax, bool& flag)
    {
	Vector inside(2);
	Vector outside(2);

	float target = .5;	
	if (curve->type==TR_RGT) {
            inside.x[0] = seg->vertex[TR_SL].x;
            inside.x[1] = seg->vertex[TR_SL].y;
            outside.x[0] = seg->vertex[TR_SR].x;
            outside.x[1] = seg->vertex[TR_SR].y;
            target = 0.0;
	} else {
            inside.x[0] = seg->vertex[TR_SR].x;
            inside.x[1] = seg->vertex[TR_SR].y;
            outside.x[0] = seg->vertex[TR_SL].x;
            outside.x[1] = seg->vertex[TR_SL].y;
            target = 1.0;
	}

	ParametricLine R(&inside, &outside);
	Vector* intersect = IntersectSphereLine(&R, C, rmax);

	flag = false;
	//	if (intersect->N==0) {
	//		printf ("oops:");
	//	}

	for (int j=0; j<intersect->Size(); j++) {
            if ((intersect->x[j]>=0) && (intersect->x[j]<=1)) {
                flag = true;
                if (seg->type==TR_LFT) {
                    target = 1 -intersect->x[j];
                } else {
                    target = intersect->x[j];
                }
            }
	}

	delete intersect;
	return target;
    }

    /**
       \brief Adjust influence weight on radi of consecutive curves.

       The radius of some segments of a curve is more influential than
       others when creating the path. For example if you have one wide
       curve followed by a tight followed by another wide one, you wish to
       treat the wide curves as if they were straight, especially close
       to the tight curve. This function implements a weighting scheme
       that can be used later. This way one can find optimal paths for
       each curve independently and then combine them using those weights.

       \c cs is the beginning of the segments and \c ce is the end of the
       segments for which we will calculate radi.

       \c radi is an array that will contain the weights; it should have a
       size at least equal to the maximum segment ID.
    */
    void Driver::AdjustRadi(tTrackSeg* cs, tTrackSeg* ce, float* radi)
    {
	float sum = 0.0;
	float max = 0.0;
	tTrackSeg* aseg;
	for (aseg = cs->next; 
             aseg != ce;
             aseg = aseg->next) {
            int id = aseg->id;
            radi[id] = 1/(aseg->radius);//exp(-0.01*aseg->radius);
            sum += radi[id];
            if (max<radi[id]) max = radi[id];
	}
	//ideal_radius[i] = seg->radius;// * radi[i]/sum;

	for (aseg = cs->next; 
             aseg != ce;
             aseg = aseg->next) {
            int id = aseg->id;
            // first compute radius at top of curve piece.
            radi[id] /= max;
            // see how far away we are from next and previous
            // pieces (meaning with different radi).
            bool same_curve = true;
            float dist_to_next = aseg->length / 2;
            float dist_to_prev = aseg->length / 2;
            tTrackSeg* prev = aseg;
            tTrackSeg* next = aseg;
            while (same_curve) {
                same_curve = false;
                if ((prev->prev->type == aseg->type)
                    && (fabs(prev->prev->radius - aseg->radius) < 1)) {
                    prev = prev->prev;
                    dist_to_prev += prev->length;
                    same_curve = true;
                }
                if ((next->next->type == aseg->type)
                    && (fabs(next->next->radius - aseg->radius) < 1)) {
                    next = next->next;
                    dist_to_next += next->length;
                    same_curve = true;
                }
            }
            // calculate weight such that it is 0 at the center of the piece
            // and 1 at the ends.
            float total_length = dist_to_next + dist_to_prev;
            float weight = fabs(dist_to_prev - dist_to_next)/total_length;
            radi[id] = (1 - weight) + weight * radi[id];
	} // for (aseg)

    }

#ifdef USE_OLETHROS_NAMESPACE
}
#endif
