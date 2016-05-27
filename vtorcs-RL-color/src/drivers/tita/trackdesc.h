/***************************************************************************

    file                 : trackdesc.h
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2006 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: trackdesc.h,v 1.1.2.2 2008/11/09 17:50:21 berniw Exp $

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
#include <tmath/linalg_t.h>

// TODO: test runs with different resolutions.
#define TRACKRES 1.0
#define RMAX 10000.0

// Magic radius for bumpy roads.
#define RREL 180.0

class TrackSegment
{
	public:
		TrackSegment();
		~TrackSegment();
		void init(const tTrackSeg* s, const vec3d* l, const vec3d* m, const vec3d* r);
		inline void setLength(double len) { length = len; }
		inline void setKbeta(double b) { kbeta = b; }
		inline void setKgamma(double c) { kgamma = c; }

		inline int getType() const { return pTrackSeg->type; }
		inline unsigned int getRaceType() const { return pTrackSeg->raceInfo; }
		inline tdble getRadius() const { return radius; }

		inline tdble getKfriction() const { return pTrackSeg->surface->kFriction; }
		inline tdble getKrollres() const { return pTrackSeg->surface->kRollRes; }
		inline tdble getKroughness() const { return pTrackSeg->surface->kRoughness; }
		inline tdble getKroughwavelen() const { return pTrackSeg->surface->kRoughWaveLen; }

		inline tdble getWidth() const { return width; }
		inline tdble getKalpha() const { return kalpha; }
		inline tdble getKbeta() const { return kbeta; }
		inline tdble getKgamma() const { return kgamma; }
		inline tdble getLength() const { return length; }

		inline const vec3d* getLeftBorder() const { return &l; }
		inline const vec3d* getRightBorder() const { return &r; }
		inline const vec3d* getMiddle() const { return &m; }
		inline const vec3d* getToRight() const { return &tr; }

		inline double sqr(double a) const { return a*a; }
		inline double distToMiddle2D(double x, double y) const { return sqrt(sqr(x-m.x) + sqr(y-m.y)); }

		inline double distToRight3D(const vec3d* p) const { return sqrt(sqr(p->x-r.x) + sqr(p->y-r.y) + sqr(p->z-r.z)); }
		inline double distToLeft3D(const vec3d* p) const { return sqrt(sqr(p->x-l.x) + sqr(p->y-l.y) + sqr(p->z-l.z)); }
		inline double distToMiddle3D(double x, double y, double z) const { return sqrt(sqr(x-m.x) + sqr(y-m.y) + sqr(z-m.z)); }
		inline double distToMiddleSqr3D(double x, double y, double z) const { return sqr(x-m.x) + sqr(y-m.y) + sqr(z-m.z); }
		inline double distToMiddle3D(const vec3d* p) const { return sqrt(sqr(p->x-m.x) + sqr(p->y-m.y) + sqr(p->z-m.z)); }

		inline tTrackSeg* getSegPtr() const { return pTrackSeg;}

	private:
		tTrackSeg* pTrackSeg;	// Id of the corresponding segment.
		vec3d l, m, r;			// right, middle and left segment (road) border.
		vec3d tr;				// normalized direction vector to the right side.
		tdble radius;			// Radius.
		tdble width;			// Width of the track segment.
		tdble kalpha;			// (roll) factor for the angle (like michigan).
		tdble kbeta;			// (curvature)factor for bumps (e-track-3).
		tdble kgamma;			// (pitch) factor of road.
		tdble length;			// Distance to the next segment (2-D, not 3-D!).
};


class TrackSegment2D
{
	public:
		TrackSegment2D();
		~TrackSegment2D();
		void init(const TrackSegment* s);
		inline tTrackSeg* getTorcsTrackSeg() const { return pTrackSeg; }

		inline int getType() const { return pTrackSeg->type; }
		inline unsigned int getRaceType() const { return pTrackSeg->raceInfo; }
		inline tdble getRadius() const { return radius; }

		inline tdble getKfriction() const { return pTrackSeg->surface->kFriction; }
		inline tdble getKrollres() const { return pTrackSeg->surface->kRollRes; }
		inline tdble getKroughness() const { return pTrackSeg->surface->kRoughness; }
		inline tdble getKroughwavelen() const { return pTrackSeg->surface->kRoughWaveLen; }

		inline tdble getWidth() const { return width; }
		inline tdble getKalpha() const { return kalpha; }
		inline tdble getKbeta() const { return kbeta; }
		inline tdble getKgamma() const { return kgamma; }

		inline const vec2d* getLeftBorder() const { return &l; }
		inline const vec2d* getRightBorder() const { return &r; }
		inline const vec2d* getMiddle() const { return &m; }
		inline const vec2d* getToRight() const { return &tr; }

		inline double sqr(double a) const { return a*a; }
		inline double distToMiddle(double x, double y) const { return sqrt(sqr(x-m.x) + sqr(y-m.y)); }
		inline double distToRight(const vec2d* p) const { return sqrt(sqr(p->x-r.x) + sqr(p->y-r.y)); }
		inline double distToLeft(const vec2d* p) const{ return sqrt(sqr(p->x-l.x) + sqr(p->y-l.y)); }
		inline double distToMiddleSqr(double x, double y, double z) const { return sqr(x-m.x) + sqr(y-m.y); }
		inline double distToMiddle(const vec2d* p) const { return sqrt(sqr(p->x-m.x) + sqr(p->y-m.y)); }

	private:
		tTrackSeg* pTrackSeg;	// Id of the corresponding segment.
		vec2d l, m, r;			// right, middle and left segment (road) border.
		vec2d tr;				// normalized direction vector to the right side.
		tdble radius;			// Radius.
		tdble width;			// Width of the track segment.
		tdble kalpha;			// (roll) factor for the angle (like michigan).
		tdble kbeta;			// (curvature)factor for bumps (e-track-3).
		tdble kgamma;			// (pitch) factor of road.
};


class TrackDesc
{
	public:
		TrackDesc(const tTrack* track);
		~TrackDesc();
		/* output track data to gnuplot readable file */
		void plot(char* filename);

		inline tTrack* getTorcsTrack() { return torcstrack; }
		//inline TrackSegment* getSegmentPtr(int index) { return &ts[index]; }
		inline TrackSegment* getSegmentPtr3D(int index) { return &ts[index]; }
		inline TrackSegment2D* getSegmentPtr(int index) { return &ts2d[index]; }
		inline int getnTrackSegments() { return nTrackSegments; }
		int getCurrentSegment(tCarElt* car, int lastId, int range);
		int getCurrentSegment(tCarElt* car);
		int getNearestId(vec2d* p);

		inline int getPitEntryStartId() { return nPitEntryStart; }
		inline int getPitExitEndId() { return nPitExitEnd; }
		inline int getPitType() { return torcstrack->pits.type; }

		static inline void dirVector2D(const vec3d* a, const vec3d* b, vec3d* r) { r->x = a->x - b->x; r->y = a->y - b->y; r->z = 0.0; }
		static inline void dirVector2D(const vec2d* a, const vec2d* b, vec2d* r) { r->x = a->x - b->x; r->y = a->y - b->y; }
		static inline double cosalpha(vec3d* a, vec3d* b) { return (*a)*(*b)/(a->len()*b->len()); }
		double distGFromPoint(vec2d* r1, vec2d* rdir, vec2d* p);

		/* returns distance to middle with p on the toright vector: value > 0 is right, value < 0 is left */
		inline double distToMiddle(int id, const vec2d* p) { return (*p - *ts2d[id].getMiddle())*(*ts2d[id].getToRight()); }
		bool isBetween(int start, int end, int id);
		int diffSegId(int id1, int id2);

	private:
		tTrack *torcstrack;		// Pointer to original TORCS  track.
		TrackSegment* ts;		// Pointer to the array of tracksegments in 3D
		TrackSegment2D* ts2d;	// Pointer to the array of tracksegments in 2D
		int nTrackSegments;		// # of track segments
		int nPitEntryStart;
		int nPitExitEnd;
};


inline double TrackDesc::distGFromPoint(vec2d* r1, vec2d* rdir, vec2d* p) {
	vec2d dv = *p - *r1;
	return fabs(rdir->x*dv.y - rdir->y*dv.x)/rdir->len();
}


// Check if a segment is within a certain range INCLUDING the borders.
inline bool TrackDesc::isBetween(int start, int end, int id) {
	if (start <= end) {
		if (id >= start && id <= end) {
			return true;
		}
	} else {
		if ((id >= 0 && id <= end) || (id >= start && id < nTrackSegments)) {
			return true;
		}
	}
	return false;
}


inline int TrackDesc::diffSegId(int id1, int id2) {
	int t;
	if (id1 > id2) { t = id1; id1 = id2; id2 = t; }
	t = (nTrackSegments - id2 + id1) % nTrackSegments;
	return MIN(id2 - id1, t);
}


// get the segment on which the car is, searching from the position of the last call within range.
inline int TrackDesc::getCurrentSegment(tCarElt* car, int lastId, int range)
{
	int start = -(range / 4);
	int end = range * 3 / 4;
 	double d, min = FLT_MAX;
	TrackSegment* ts;
	int minindex = 0;

	for (int i = start; i < end; i++) {
 		int j = (lastId+i+getnTrackSegments()) % getnTrackSegments();
		ts = getSegmentPtr3D(j);
		d = ts->distToMiddleSqr3D(car->_pos_X, car->_pos_Y, car->_pos_Z);
		if (d < min) {
			min = d;
			minindex = j;
		}
	}
	return minindex;
}

#endif // _TRACKDESC_H_


