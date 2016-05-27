/***************************************************************************

    file                 : learn.cpp
    created              : Wed Aug 28 16:36:00 CET 2004
    copyright            : (C) 2004 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: learn.cpp,v 1.17.2.1 2008/05/30 22:18:32 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "learn.h"
#include <learning/ANN.h>
#include <learning/string_utils.h>
#include <iostream>

#ifdef WIN32
#include <float.h>
#define finite _finite
#endif

#ifdef USE_OLETHROS_NAMESPACE
//namespace olethros
//{
#endif

/// Check that tags match
inline bool CheckMatchingToken (const char* tag, StringBuffer* buf, FILE* f)
{
    int l = 1+strlen(tag);
    buf = SetStringBufferLength (buf, l);
    if (buf==NULL) {
        return false;
    }
    fread(buf->c, sizeof(char), l, f);

    if (strcmp(tag,buf->c)) {
        fprintf (stderr, "Expected tag <%s>, found <%s>.\n", tag, buf->c);
        return false;
    }
    return true;
}

/// Write a token
inline void WriteToken (const char* tag, FILE* f)
{
    fwrite (tag, sizeof(char), 1+strlen(tag), f);
}

SegLearn::SegLearn(tTrack* t)
{
    int i;
    track = t;
    n_seg = t->nseg;
    segments_per_quantum = 1;
    n_quantums = 1 + n_seg / segments_per_quantum;
    prev_quantum = n_quantums-1;

    radius = new float[t->nseg];
    updateid = new int[t->nseg];
    accel = new float[n_quantums];
    derror = new float[n_quantums];
    elig = new float[n_quantums];
	
    segdm = new float[t->nseg];
    segdm2 = new float[t->nseg];
    segdm3 = new float[t->nseg];
    dm = 0.0f;
    dm2 = 0.0f;
    dm3 = 0.0f;
    W_brake = -1.0;
    W_accel = 1.0;
    lap = 0;

    tTrackSeg *seg = t->seg;

    // Switch seg to seg 0 for sure.
    while (seg->id != 0) {
        seg = seg->prev;
    }

    for (i=0; i<n_quantums; i++) {
        accel[i] = 0.0;
        derror[i] = 0.0;
        elig[i] = 0.0;
    }

    for (i = 0; i < t->nseg; i++) {
        segdm[i] = 0.0;
        segdm2[i] = 0.0;
        segdm3[i] = 0.0;
        radius[i] = 0.0;
        updateid[i] = i;
        // Search the last turn in case of a straight.
        if (seg->type == TR_STR) {
            tTrackSeg *cs = seg;
            while (cs->type == TR_STR) {
                cs = cs->prev;
            }
            updateid[seg->id] = cs->id;
        }
        seg = seg->next;
    }

    check = false;
    rmin = t->width/2.0;
    prevtype = lastturn = TR_STR;
    mu = 1.0;
    mass = 1000.0;
    CA =0.5;
    CW = 0.5;
    u = 0.0;
    brake = 0.0;
    delta_time = 0.0f;
    previous_time = 0;
    prev_time = 0;
    prevsegid = 0;
    time_since_accident = 0.0f;
    time_since_left_turn = 10.0;
    time_since_right_turn = 10.0;
    new_lap = false;
    lap = 0;
    remaining_laps = 1;
}


SegLearn::~SegLearn()
{
    delete [] radius;
    delete [] updateid;
    delete [] accel;
    delete [] derror;
    delete [] elig;
    delete [] segdm;
    delete [] segdm2;
    delete [] segdm3;

}


void SegLearn::update(tSituation *s, tTrack *t, tCarElt *car, int alone, float offset, float outside, float *r, float alpha, float speed, float limit)
{
    bool local_update = true;
    float risk_factor = 100.0f;

    remaining_laps = car->_remainingLaps;

    if (car->_laps != lap) {
        lap = car->_laps;
        new_lap = true;
    } else {
        new_lap = false;
    }


    // Still on the same segment, alone, offset near 0, check.
    tTrackSeg *seg = car->_trkPos.seg;
    if (prev_time!=s->currentTime) {
        delta_time = s->currentTime - prev_time;
        prev_time = s->currentTime;
    }

    if (time_since_accident < 0.5f) {
        time_since_accident += delta_time;
        return;
    }

    tTrackSeg *prev_seg = seg;
    bool term = false;
    float dist = 0.0f;
    while (term == false) {
        prev_seg = prev_seg->prev;
        dist += prev_seg->length;
        if ((prev_seg->type != seg->type)
            || (dist > 100.0f)) {
            term = true;
        }
    }

    switch(seg->type) {
    case TR_LFT:
        time_since_left_turn = 0.0f;
        break;
    case TR_RGT:
        time_since_right_turn = 0.0f;
        break;
    }

    if (time_since_left_turn < 10.0f) {
        time_since_left_turn += delta_time;
    }
    if (time_since_right_turn < 10.0f) {
        time_since_right_turn += delta_time;
    }

    if (seg->type == lastturn || seg->type == TR_STR) {
        float max_out_factor = 1.0f;
        float mid_out_factor = 0.75f;
        float min_out_factor = 0.5f;
        if (fabs(offset) < 0.2 &&
            check == true 
            && alone > 0
            ) {

            float dr = 0.0;
            float tomiddle = car->_trkPos.toMiddle;
            // adjusted tomiddle

            float beta = 1.0;

            float target_toLeft = seg->width * (1-alpha);
            float target_toRight = seg->width * (alpha);

            // if negative then we are to the right of our target.
            // if positive then we are to the left of our target.
            // theta is a simple threshold
            float target_error = fabs(target_toLeft - car->_trkPos.toLeft);
            float theta = 0.5*seg->width - 0.5*car->_dimension_y;
            float dtheta = theta - target_error;

            if (dtheta < 0) dtheta -= 1;
            if (lastturn == TR_RGT) {
                if (target_toRight > car->_trkPos.toRight) {
                    if (time_since_left_turn < 1.0f) {
                        float drisk = car->_trkPos.toRight - car->_dimension_y;
                        if (drisk < 0.0f) {
                            if (dtheta>0) {
                                dtheta = 2.0f * drisk - 1.0f;
                            } else {
                                dtheta += 2.0f * drisk - 1.0f;
                            }
                        }
                    } else {
                        dtheta += 1.0;//theta;
                    }
                }
                if (car->_trkPos.toLeft < max_out_factor*car->_dimension_y
                    && dtheta > 0) {
                    float a = 1.5*car->_dimension_y - car->_trkPos.toLeft;
                    dtheta = (1-a) * dtheta;
                }
                if (car->_trkPos.toLeft - mid_out_factor*car->_dimension_y<0) {
                    float d = risk_factor * (car->_trkPos.toLeft - car->_dimension_y);// - 1;
                    if (d<dtheta) {
                        dtheta = d;
                    }
                }
                if ((car->_trkPos.toLeft - min_out_factor*car->_dimension_y<0)
                    || (car->_speed_x < 0)) {
                    dtheta = - risk_factor;
                    PropagateUpdateBackwards (seg->prev, -0.1f, 0.01f, 200.0f);
                    time_since_accident = 0.0f;
                    //printf ("DTH %d\n ", seg->id);
                }

                dr = (1-beta) * (outside - tomiddle)
                    + beta * (dtheta);
            } else if (lastturn == TR_LFT) {

                //if (prev_seg->type == TR_RGT || time_since_right_turn < 1.0f) {
                if (time_since_right_turn < 1.0f) {
                    if (target_toLeft > car->_trkPos.toLeft) {
                        float drisk = car->_trkPos.toLeft - car->_dimension_y;
                        if (drisk < 0.0f) {
                            if (dtheta>0) {
                                dtheta = 2.0f * drisk - 1.0f;
                            } else {
                                dtheta += 2.0f * drisk - 1.0f;
                            }
                        }
                    }
                } else {
                    dtheta += 1.0;//theta;
                }
                if (car->_trkPos.toRight < max_out_factor*car->_dimension_y
                    && dtheta > 0) {
                    float a = 1.5*car->_dimension_y - car->_trkPos.toRight;
                    dtheta = (1-a) * dtheta;
                }
                if (car->_trkPos.toRight - mid_out_factor*car->_dimension_y<0) {
                    float d = risk_factor *(car->_trkPos.toRight - car->_dimension_y);// - 1;
                    if (d<dtheta) {
                        dtheta = d;
                    }
                }
                if ((car->_trkPos.toRight - min_out_factor*car->_dimension_y < 0)
                    || (car->_speed_x < 0)) {
                    dtheta = - risk_factor;
                    PropagateUpdateBackwards (seg->prev, -0.1f, 0.01f, 200.0f);
                    time_since_accident = 0.0f;
                    //printf ("DTH %d\n", seg->id);
                }
                dr = (1-beta) * (outside + tomiddle)
                    + beta * (dtheta);
            }
            if (local_update) {
                //radius[updateid[seg->id]] += 0.01*dr;
                if (dr<0) {
                    PropagateUpdateBackwards (seg->prev, 0.01f*dr, 0.002f, 400.0f);
                } else {
                    PropagateUpdateBackwards (seg, 0.01f*dr, 0.05f, 100.0f);
                }

            }
            //printf ("%f %f %f %f \n", target_error, dr, rmin, r[seg->id]);
            if (dr < rmin) {
                rmin = dr;
            }
        } else {
            check = false;
        }
    }

    if (seg->type != prevtype) {
        prevtype = seg->type;
        if (seg->type != TR_STR) {
            if (check == true) {
                tTrackSeg *cs = seg->prev;
                // Skip straights.
                while (cs->type == TR_STR) {
                    cs = cs->prev;
                }

                while (cs->type == lastturn) {
                    if (radius[updateid[cs->id]] + rmin < 0.0) {
                        rmin = MAX(cs->radius - r[cs->id], rmin);
                    }
                    if (local_update == false) {
                        radius[updateid[cs->id]] += rmin;
                        radius[updateid[cs->id]] = MIN(radius[updateid[cs->id]], 1000.0);
                    }
                    cs = cs->prev;
                }
            }
            check = true;
            rmin = MIN(seg->width/2.0, seg->radius/10.0);
            lastturn = seg->type;
        }
    }
}

void SegLearn::PropagateUpdateBackwards (tTrackSeg* pseg, float d, float beta, float max_length)
{

    //printf ("back: %d->", pseg->id);
    float length;
    for (length = 0; length<max_length;) {
        length += pseg->length;
        pseg = pseg->prev;
        radius[updateid[pseg->id]] += d*exp(-beta*length);
    }
    //printf ("%d (%f->%f)\n", pseg->id, length, exp(-beta*length));
}

float SegLearn::updateAccel (tSituation* s, tCarElt* car, float taccel, float derr, float dtm)
{
    float alpha = 0.05f;
    float beta = 1.0f;
    float lambda = 1.0f;
    tTrackSeg* seg = car->_trkPos.seg;
	
    float in_track = 1.0;
    if (car->_trkPos.toRight - car->_dimension_y<0) {
        in_track = 1-fabs(tanh(0.5*(car->_trkPos.toRight - car->_dimension_y)));
        dtm = 2.0*(car->_trkPos.toRight - car->_dimension_y);
        //printf ("R:%f %f\n", car->_steerCmd, dtm);
    }
    if (car->_trkPos.toLeft - car->_dimension_y<0) {
        in_track = 1-fabs(tanh(0.5*(car->_trkPos.toLeft - car->_dimension_y)));
        dtm = -2.0*(car->_trkPos.toLeft - car->_dimension_y);
        //printf ("L:%f %f\n", car->_steerCmd, dtm);
    }
    if (car->_speed_x < 0) {
        in_track = 0;
        taccel = -1;
    }
    int segid = (seg->id);
    int quantum = segQuantum(segid);
    //float prev_quantum_accel = accel[prev_quantum];
    if (quantum != prev_quantum) {
        float dt = s->currentTime - previous_time;
        previous_time = s->currentTime;
        float gamma =exp(-dt*beta);
        float lg = lambda*gamma;

        //printf ("%f g%f\n", dt, gamma);
        elig[prev_quantum] = 1.0;

        float da = alpha * (taccel - accel[prev_quantum]);
        float ds = alpha * in_track * (dtm + gamma*derror[quantum] - derror[prev_quantum]);
        for (int i=0; i<n_quantums; i++) {
            accel[i] += elig[i] * da;
            derror[i] += elig[i] * ds;
            elig[i] *= lg;
        }
        prev_quantum = quantum;
        prev_accel = taccel;
        averages.k = 0;
        //printf ("%f %f\n", accel[segid], derror[segid]);
    } 
		
    averages.Measure(taccel, derr, dtm);
	
    return  0.0;//accel[quantum];//+ prev_quantum_accel);
}

float SegLearn::predictedError (tCarElt* car)
{
    tTrackSeg* seg = car->_trkPos.seg;
    int segid = seg->id;
    return derror[segid];
}

float SegLearn::predictedAccel (tTrackSeg* seg)
{
    //int segid = seg->id;
    return 0.0f;//derror[segid];
}

int SegLearn::segQuantum (int segid)
{
    int q = segid/segments_per_quantum;
    return q;
}

/**
   Model the system
   \f[
   \frac{du}{dt}=
   s_{brake} (
   \mu G + w_1 +
   \frac{(C_A \mu + C_W + w_2) u^2}{m} 
   )
   \f]
*/
void SegLearn::AdjustFriction (tTrackSeg* s, float G, float mass_, float CA_, float CW_, float u_, float brake_, float learning_rate)
{

    if (delta_time<=0.0) {
        delta_time = 0.02f;
    }
    float mu_ = s->surface->kFriction;
    float du = (u_ - u)/delta_time;//delta_time;
    float x_brake = 0.0f;
    float x_accel = 0.0f;
    if (brake<0) {
        x_accel = -brake;
    } else {
        x_brake = brake;
    }
    float brake_factor = W_brake*x_brake;
    float accel_factor = W_accel*x_accel;
    if (fabs(u)>10.0f) {
        accel_factor /= fabs(u); 
    } else {
        accel_factor /= 10.0f; 
    }
    float car_factor = tand (brake_factor + accel_factor);
    float car_factor_der = tand_der (brake_factor + accel_factor);
    //mu = 1.0;
    float N = G;// +(.5*CA*u*u/mass);
    //printf ("%f %f\n", G, N);
    float friction_factor = (mu + dm+segdm[prevsegid])*N;
    float friction_factor_der = N;
    float aero_factor = CW/mass*u*fabs(u);
    float pdu = car_factor * friction_factor - aero_factor;
    float delta_a = (du - pdu);
    float delta = learning_rate * delta_a;
    float der_dm = 0.05f*delta * friction_factor_der * car_factor;
    float der_acc = delta * car_factor_der * friction_factor;
    W_brake += der_acc * x_brake * car_factor_der * friction_factor;
    W_accel += der_acc * x_accel * car_factor_der * friction_factor;
    //float der_dm2 = -SIGN(u)*u*u*brake/mass;

    dm += 0.1*der_dm;
    segdm[prevsegid] += der_dm;

    mu=mu_; 
    mass=mass_;
    CA=CA_; 
    CW=CW_; 
    u=u_;
#if 0
    if ((new_lap) ||
        (remaining_laps==0 && prevsegid!=s->id)) {
        printf ("%f %f %f %f %f %f %f %f #DM\n", brake,
                dm, segdm[prevsegid],
                W_brake, W_accel,
                //dm3, segdm3[prevsegid],
                du, pdu, pdu-du);
    }
#endif
    brake=brake_;
    prevsegid = s->id;
}


bool SegLearn::LoadParameter (float* p, int n, FILE* f)
{
    fread (p, sizeof(float), n, f);
    bool flag = false;
    int i;
    for (i=0; i<n; i++) {
        if (!finite (p[i])) {
            p[i] = 0.0f;
            flag = true;
        }
    }
    if (flag) {
        fprintf (stderr, "warning: olethros/learn.cpp: infinite parameters, setting to 0.");
    }
    return flag;
}

void SegLearn::loadParameters (char* fname)
{
    //std::cout << "Maybe load parameters from " << fname << std::endl;
    FILE* f = fopen(fname,"rb");
    if (!f) { // no error here.
        return;
    }
	
    StringBuffer* rtag = NewStringBuffer (256);
    CheckMatchingToken("OLETHROS_LEARN", rtag, f);
    int local_n_quantums;
    fread (&local_n_quantums, sizeof(int), 1, f);
    if (local_n_quantums!=n_quantums) {
        std::cerr << "Number of quantums " << local_n_quantums << " does not agree with current (" << n_quantums << "). Aborting read.\n";
        fclose(f);
        return;
    }
    CheckMatchingToken("RADI", rtag, f);
    fread (radius, n_seg, sizeof(float), f);

    CheckMatchingToken("DM FRICTION", rtag, f);
    LoadParameter (segdm, n_seg,  f);
    LoadParameter (segdm2,  n_seg, f);
    LoadParameter (segdm3,  n_seg, f);
    LoadParameter (&dm, 1, f);
    LoadParameter (&dm2, 1, f);
    LoadParameter (&dm3, 1, f);

    CheckMatchingToken("PRED ACCEL", rtag, f);
    LoadParameter (accel, n_quantums,  f);
    CheckMatchingToken("PRED STEER", rtag, f);
    LoadParameter (derror, n_quantums,  f);
	
    CheckMatchingToken("END",rtag, f);
    FreeStringBuffer(&rtag);
    fclose(f);
    //std::cout << "Parameters loaded\n";
}

/// Save
void SegLearn::saveParameters (char* fname)
{
    FILE* f = fopen(fname,"wb");
    //std::cout << "Maybe save parameters to " << fname << std::endl;
    if (!f) {
        std::cerr << "Could not open " << fname << " for writing. Check permissions\n";
        return;
    }

    //StringBuffer* rtag = NewStringBuffer (256);
    WriteToken("OLETHROS_LEARN", f);
    fwrite (&n_quantums, sizeof(int), 1, f);

    WriteToken("RADI", f);
    fwrite (radius, n_seg, sizeof(float), f);

    WriteToken("DM FRICTION", f);
    fwrite (segdm, sizeof(float), n_seg,  f);
    fwrite (segdm2, sizeof(float), n_seg, f);
    fwrite (segdm3, sizeof(float), n_seg, f);
    fwrite (&dm, sizeof(float), 1, f);
    fwrite (&dm2, sizeof(float), 1, f);
    fwrite (&dm3, sizeof(float), 1, f);

    WriteToken("PRED ACCEL", f);
    fwrite (accel, sizeof(float), n_quantums,  f);

    WriteToken("PRED STEER", f);
    fwrite (derror, sizeof(float), n_quantums,  f);

    WriteToken("END", f);
    //FreeStringBuffer(&rtag);
    fclose(f);
    //std::cout << "Parameters saved\n";
}

#ifdef USE_OLETHROS_NAMESPACE
//}
#endif
