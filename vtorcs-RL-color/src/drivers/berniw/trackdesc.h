/***************************************************************************

    file                 : trackdesc.h
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: trackdesc.h,v 1.15.2.1 2008/11/09 17:50:19 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	class TrackSegment:
	describes the properties of approx 1 m on the track, like where the borders are, pysical stuff
	(eg. friction).
	class TrackDesc:
	describes the whole track, the data is stored in an array of Tracksegments.
*/

#ifndef _TRACKDESC_H_
#define _TRACKDESC_H_

#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#include <tgf.h>
#include <track.h>
#include <car.h>
#include <raceman.h>
#include <robot.h>
#include <robottools.h>
#include <math.h>
#include "linalg.h"

#define TRACKRES 1.0
#define RMAX 10000.0

/* magic radius for bumpy roads */
#define RREL 180.0

class TrackSegment
{
	public:
		TrackSegment();
		~TrackSegment();
		void init(int id, const tTrackSeg* s, const v3d* l, const v3d* m, const v3d* r);
		inline void setLength(double len) { length = len; }
		inline void setKbeta(double b) { kbeta = b; }
		inline void setKgamma(double c) { kgamma = c; }

		inline int getType() { return type; }
		inline unsigned int getRaceType() { return raceType; }
		inline tdble getRadius() { return radius; }

		inline tdble getKfriction() { return pTrackSeg->surface->kFriction; }
		inline tdble getKrollres() { return pTrackSeg->surface->kRollRes; }
		inline tdble getKroughness() { return pTrackSeg->surface->kRoughness; }
		inline tdble getKroughwavelen() { return pTrackSeg->surface->kRoughWaveLen; }

		inline tdble getWidth() { return width; }
		inline tdble getKalpha() { return kalpha; }
		inline tdble getKbeta() { return kbeta; }
		inline tdble getKgamma() { return kgamma; }
		inline tdble getLength() { return length; }

		inline v3d* getLeftBorder() { return &l; }
		inline v3d* getRightBorder() { return &r; }
		inline v3d* getMiddle() { return &m; }
		inline v3d* getToRight() { return &tr; }

		inline double sqr(double a) { return a*a; }
		inline double distToMiddle2D(double x, double y) { return sqrt(sqr(x-m.x) + sqr(y-m.y)); }

		inline double distToRight3D(v3d* p) { return sqrt(sqr(p->x-r.x) + sqr(p->y-r.y) + sqr(p->z-r.z)); }
		inline double distToLeft3D(v3d* p) { return sqrt(sqr(p->x-l.x) + sqr(p->y-l.y) + sqr(p->z-l.z)); }
		inline double distToMiddle3D(double x, double y, double z) { return sqrt(sqr(x-m.x) + sqr(y-m.y) + sqr(z-m.z)); }
		inline double distToMiddleSqr3D(double x, double y, double z) { return sqr(x-m.x) + sqr(y-m.y) + sqr(z-m.z); }
		inline double distToMiddle3D(v3d* p) { return sqrt(sqr(p->x-m.x) + sqr(p->y-m.y) + sqr(p->z-m.z)); }

	private:
		tTrackSeg* pTrackSeg;	/* id of the corresponding segment */
		int type;				/* physical type (eg. straight, left or right) */
		unsigned int raceType;	/* race type (eg. pitlane, speedlimit, ...) */
		v3d l, m, r;			/* right, middle and left segment (road) border */
		v3d tr;					/* normalized direction vector to the right side */
		tdble radius;			/* radius */
		tdble width;			/* width of the track segment*/
		tdble kalpha;			/* (roll)factor for the angle (like michigan) */
		tdble kbeta;			/* (curvature)factor for bumps (e-track-3) */
		tdble kgamma;			/* (pitch)factor of road */
		tdble length;			/* distance to the next segment (2-D, not 3-D!) */
};

class TrackDesc
{
	public:
		TrackDesc(const tTrack* track);
		~TrackDesc();
		/* output track data to gnuplot readable file */
		void plot(char* filename);

		inline tTrack* getTorcsTrack() { return torcstrack; }
		inline TrackSegment* getSegmentPtr(int index) { return &ts[index]; }
		inline int getnTrackSegments() { return nTrackSegments; }
		int getCurrentSegment(tCarElt* car, int lastId, int range);
		int getCurrentSegment(tCarElt* car);
		int getNearestId(v3d* p);

		inline int getPitEntryStartId() { return nPitEntryStart; }
		inline int getPitExitEndId() { return nPitExitEnd; }
		inline int getPitType() { return torcstrack->pits.type; }

		static inline void dirVector2D(v3d* a, v3d* b, v3d* r) { r->x = a->x - b->x; r->y = a->y - b->y; r->z = 0.0; }
		static inline double cosalpha(v3d* a, v3d* b) { return (*a)*(*b)/(a->len()*b->len()); }
		double distGFromPoint(v3d* r1, v3d* rdir, v3d* p);

		/* returns distance to middle with p on the toright vector: value > 0 is right, value < 0 is left */
		inline double distToMiddle(int id, v3d* p) { return (*p - *ts[id].getMiddle())*(*ts[id].getToRight()); }
		bool isBetween(int start, int end, int id);
		int diffSegId(int id1, int id2);

	private:
		tTrack *torcstrack;
		TrackSegment* ts;		/* pointer to the array of tracksegments */
		int nTrackSegments;		/* # of track segments */
		int nPitEntryStart;
		int nPitExitEnd;
};


inline double TrackDesc::distGFromPoint(v3d* r1, v3d* rdir, v3d* p) {
	v3d t, s;
	p->dirVector(r1, &t);
	rdir->crossProduct(&t, &s);
	return s.len()/rdir->len();
}


inline bool TrackDesc::isBetween(int start, int end, int id) {
	if (start <= end) {
		if (id >= start && id <= end) { return true; } else { return false; }
	} else {
		if ((id >= 0 && id <= end) || (id >= start && id < nTrackSegments)) { return true; } else { return false; }
	}
}


inline int TrackDesc::diffSegId(int id1, int id2) {
	int t;
	if (id1 > id2) { t = id1; id1 = id2; id2 = t; }
	t = (nTrackSegments - id2 + id1) % nTrackSegments;
	return MIN(id2 - id1, t);
}


/* get the segment on which the car is, searching from the position of the last call within range */
inline int TrackDesc::getCurrentSegment(tCarElt* car, int lastId, int range)
{
	int start = -(range / 4);
	int end = range * 3 / 4;
 	double d, min = FLT_MAX;
	TrackSegment* ts;
	int minindex = 0;

	for (int i = start; i < end; i++) {
 		int j = (lastId+i+getnTrackSegments()) % getnTrackSegments();
		ts = getSegmentPtr(j);
		d = ts->distToMiddleSqr3D(car->_pos_X, car->_pos_Y, car->_pos_Z);
		if (d < min) {
			min = d;
			minindex = j;
		}
	}
	return minindex;
}

#endif // _TRACKDESC_H_


