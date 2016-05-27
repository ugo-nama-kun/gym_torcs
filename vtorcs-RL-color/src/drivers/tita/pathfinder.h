/***************************************************************************

    file                 : pathfinder.h
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2006 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: pathfinder.h,v 1.1.2.2 2008/11/09 17:50:21 berniw Exp $

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
	should (!) compute a good path
*/

#ifndef _PATHFINDER_H_
#define _PATHFINDER_H_

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

#include "trackdesc.h"
#include "mycar.h"
#include "spline.h"


// how far we look forward for overtaking, collisions, etc.
#define AHEAD	((int) (500.0/TRACKRES))
#define BACK	((int) (20.0/TRACKRES))

// how many segments can i pass per simulation step, depends on TRACKRES, simulation->_deltaTime and speed.
#define SEGRANGE ((int) (0.5+3.0/(50*RCM_MAX_DT_ROBOTS*TRACKRES)))
#define PATHBUF	(BACK+AHEAD+SEGRANGE)


class MyCar;
class OtherCar;


// holds data of opponents relative to my car.
typedef struct {
	double speedsqr;		// on track direction projected speed squared of opponent.
	double speed;			// same, but not squared.
	double time;			// estimate of time to catch up the car.
	double cosalpha;		// cos(alpha) from angle between my ond opponent.
	double disttomiddle;	// distance to middle (for prediction).
	int catchdist;			// roughly estimated distance to catch up with the opponent.
	int catchsegid;			// segment, where i expect (or better guess!) to catch opponent.
	double dist;			// #segments from me to the other car.
	OtherCar* collcar;		// pointer to the car.
	bool overtakee;			// is this the guy to overtake?
	double disttopath;		// distance to my path.
	double brakedist;		// distance needed for braking to the speed of this car.
	double mincorner;		// corner nearest to my car.
	double minorthdist;		// minimal distance relative to my car.
	double width;			// Width of the car in track direction.
} tOCar;


// holds data needed for let pass opponents.
typedef struct {
	double time;			// how long is the opponent "in range" to overlap me.
} tOverlapTimer;


class PathSegOpt
{
	public:
		PathSegOpt(int segments) {
			o = new vec2d[segments];
			tr = new vec2d[segments];
			length = new float[segments];
			disttomiddle = new float[segments];
			nextborderseg = new int[segments];
		}
		~PathSegOpt() { delete [] o; delete [] tr; delete [] length; delete [] disttomiddle; delete [] nextborderseg; }

		inline void setOptLoc(const vec2d* ip, int id) { o[id] = (*ip); }
		inline void setOptToRight(const vec2d* ip, int id) { tr[id] = (*ip); }
		inline void setOptLength(float l, int id) { length[id] = l; }
		inline void setOptToMiddle(float m, int id) { disttomiddle[id] = m; }

		inline const vec2d* getOptLoc(int id) { return &o[id]; }
		inline const vec2d* getOptToRight(int id) { return &tr[id]; }
		inline float getOptLength(int id) { return length[id]; }
		inline float getOptToMiddle(int id) { return disttomiddle[id]; }

	private:
		vec2d* o;				// Position in space, static trajectory.
		vec2d* tr;				// to right perpendicular to the path.
		float* length;			// dist to next pathseg.
		float* disttomiddle;	// Distance of path point to middle.
		int* nextborderseg;		// Index of the next segment right on the border.
};


class PathSegPit
{
	public:
		PathSegPit(int pitsegments, int segments, int start, int end, PathSegOpt* p) :
			path(p), startid(start), endid(end-1), npitsegments(pitsegments), nsegments(segments)
		{
			l = new vec2d[pitsegments];
		}
		~PathSegPit() { delete [] l; }

		inline void setPitLoc(vec2d* ip, int id) {
			if (isPitSeg(id)) {
				l[(id - startid + nsegments) % nsegments] = *ip;
			}
		}
		inline const vec2d* getPitLoc(int id) {
			return (isPitSeg(id)) ? &l[(id - startid + nsegments) % nsegments] : path->getOptLoc(id);
		}

	private:
		vec2d* l;			// Trajectory for pit lane (per car, constant).
		PathSegOpt* path;	// Optimal trajectory (TODO: static).
		int startid;		// Id of first pit segment.
		int endid;			// Id of last pit Segment.
		int npitsegments;	// Number of pit segments.
		int nsegments;		// # of all track segments (NOT pit segments!).

		inline bool isPitSeg(int id) {
			if (endid - startid >= 0) {
				if (id >= startid && id <= endid) {
					return true;
				}
			} else {
				if ((id >= 0 && id <= endid) || (id >= startid && id < nsegments)) {
					return true;
				}
			}
			return false;
		}
};


class PathSeg
{
	public:
		PathSeg(int segments, int tracksegments) {
			ps = new pseg[segments];
			size = segments;
			nseg = tracksegments;
			baseid = baseval = 0;
		}
		~PathSeg() { delete [] ps; }

		void set(tdble ispeedsqr, tdble ilength, vec2d* ip, vec2d* id, int segid);
		void set(tdble ispeedsqr, tdble ilength, vec2d* id, int segid);
		inline void setLoc(const vec2d* ip, int segid) { ps[getid(segid)].p = (*ip); }

		inline void setSpeedsqr(tdble spsqr, int segid) { ps[getid(segid)].speedsqr = spsqr; }
		inline void setRadius(tdble r, int segid) { ps[getid(segid)].radius = r; }

		inline tdble getSpeedsqr(int segid) { return ps[getid(segid)].speedsqr; }
		inline tdble getLength(int segid) { return ps[getid(segid)].length; }
		inline tdble getRadius(int segid) { return ps[getid(segid)].radius; }

		inline vec2d* getLoc(int segid) { return &ps[getid(segid)].p; }
		inline vec2d* getDir(int segid) { return &ps[getid(segid)].d; }

		// Set id of ringbuffer start and its value.
		inline void setBase(int segid) {
			if (segid == baseval) {
				return;
			}
			if (segid - baseval < -3) {
				baseid = (baseid + segid - baseval + nseg) % size;
			} else {
				baseid = (baseid + segid - baseval) % size;
			}
			baseval = segid;
		}

	private:
		class pseg {
			public:
				tdble speedsqr;	// Max possible speed sqared (speed ist therefore sqrt(speedsqr).
				tdble length;	// Dist to the next pathseg.
				tdble radius;	// Radius of current segment.
				vec2d p;		// Position in space, dynamic trajectory.
				vec2d d;		// Direction vector of dynamic trajectory.
		};
		pseg* ps;				// Dynamic path, ringbuffer.
		int size;				// # of ps segments
		int nseg;				// Number of track segments.
		int baseval;			// Tracksegid of current buffer start element.
		int baseid;				// Index of first element.

		int getid(int id) {
			int adr;
			if (id < baseval) {
				adr = id - baseval + nseg;
			} else {
				adr = id - baseval;
			}
			return (adr + baseid) % size;
		}
};


class Pathfinder
{
	public:
		Pathfinder(TrackDesc* itrack, tCarElt* car, tSituation *situation);
		~Pathfinder();
		void plan(int trackSegId, tCarElt* car, tSituation* situation, MyCar* myc, OtherCar* ocar);
		void plan(MyCar* myc, int currentsegid);

		void initPit(tCarElt* car);
		inline bool isPitAvailable() { return pit; }
		inline int getPitSegId() { return pitSegId; }
		void setPitStop(bool p, int id);
		inline bool getPitStop() { return pitStop; }
		int segmentsToPit(int id);
		void plotPitStopPath(char* filename);
		void plotPath(char* filename);

		inline double sqr(const double a) const { return a*a; };
		inline double dist(const vec2d* a, const vec2d* b) const { return sqrt(sqr(a->x-b->x) + sqr(a->y-b->y)); }
		inline double dist2D(vec2d* a, vec2d* b) { return sqrt(sqr(a->x-b->x) + sqr(a->y-b->y)); }
		inline PathSeg* getPath() { return psdyn; }

		int getCurrentSegment(tCarElt* car);
		int getCurrentSegment(tCarElt* car, int range);
		inline int getnPathSeg() { return nPathSeg; }
		inline double getPitSpeedSqrLimit() { return pitspeedsqrlimit; }
		double distToPath(int trackSegId, vec2d* p);
		double distToOptPath(int trackSegId, vec2d* p);

	private:
		static const double COLLDIST;	// up to this distance do we consider other cars as dangerous.
		enum { PITPOINTS = 7 };			// # points for pit spline.

		TrackDesc* track;		// pointer to track data.
		int lastId;				// segment id of the last call.
		int nPathSeg;			// # of PathSeg's.
		int lastPlan;			// start of the last plan.
		int lastPlanRange;		// range of the last plan.
		bool pitStop;			// pitstop ?
		bool inPit;				// internal pit state.

		int s1, s3;				// pitentrystart, pitentryend.
		int e1, e3;				// pitexitstart, pitexitend.

		vec2d pitLoc;			// location of pit.
		int pitSegId;			// segment id of pit.
		bool pit;
		int changed;
		double pitspeedsqrlimit;		// speed limit for pit lane squared.

		static PathSegOpt* psopt;		// Precomuted path.
		static bool optpathinitialized;	// Precomuted path is shared between instances, is it initialized?
		PathSegPit* pspit;				// Pit path.
		PathSeg* psdyn;					// Dynamic trajectory.

		int collcars;
		tOCar* o;
		tOverlapTimer* overlaptimer;

		tCarElt* teammate;				// Teammate, TODO: support multiple teammates.

		void initPitStopPath(void);
		void getPitPoint(int j, int k, double slope, double dist, vec2d* r);
		int collision(int trackSegId, tCarElt* mycar, tSituation *s, MyCar* myc, OtherCar* ocar);
		int overtake(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar);
		double curvature(double xp, double yp, double x, double y, double xn, double yn);
		void adjustRadius(int s, int p, int e, double c, double carwidth);
		void stepInterpolate(int iMin, int iMax, int Step);
		void interpolate(int Step);
		void smooth(int Step);

		int correctPath(int id, tCarElt* car, MyCar* myc);
		void smooth(int s, int e, int p, double w);
		int updateOCar(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o);
		void updateOverlapTimer(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o, tOverlapTimer* ov);
        int letoverlap(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOverlapTimer* ov);
		double pathOptSlope(int id);
		double pathDynSlope(int id);
		int countSegments(int from, int to);
};


inline double Pathfinder::distToPath(int trackSegId, vec2d* p)
{
	vec2d *pd = psdyn->getDir(trackSegId);	// Dir is already normalized, length == 1.0.
	vec2d *pl = psdyn->getLoc(trackSegId);
	vec2d rq = *p - *pl;
	return pd->y*rq.x - pd->x*rq.y;
}


inline double Pathfinder::distToOptPath(int trackSegId, vec2d* p)
{
	const vec2d *pd = psopt->getOptToRight(trackSegId);	// Dir is already normalized, length == 1.0.
	const vec2d *pl = psopt->getOptLoc(trackSegId);
	vec2d rq = *p - *pl;
	return pd->x*rq.x + pd->y*rq.y;
}


inline void PathSeg::set(tdble ispeedsqr, tdble ilength, vec2d* ip, vec2d* id, int segid) {
	int sid = getid(segid);
	ps[sid].speedsqr = ispeedsqr;
	ps[sid].length = ilength;
	ps[sid].p = (*ip);
	ps[sid].d = (*id);
}


inline void PathSeg::set(tdble ispeedsqr, tdble ilength, vec2d* id, int segid) {
	int sid = getid(segid);
	ps[sid].speedsqr = ispeedsqr;
	ps[sid].length = ilength;
	ps[sid].d = (*id);
}


inline void Pathfinder::setPitStop(bool p, int id) {
	if (isPitAvailable() && track->isBetween(e3, (s1 - AHEAD + nPathSeg) % nPathSeg, id) && p) {
		pitStop = true ;
	} else {
		pitStop = false;
	}
}


inline int Pathfinder::segmentsToPit(int id) {
	if (id <= pitSegId) {
		return (pitSegId - id);
	} else {
		return (track->getnTrackSegments() - id + pitSegId);
	}
}


inline double Pathfinder::pathOptSlope(int id) {
	int nextid = (id + 1) % nPathSeg;
	vec2d dir = *psopt->getOptLoc(nextid) - *psopt->getOptLoc(id);
	double dp = dir*(*track->getSegmentPtr(id)->getToRight())/dir.len();
	double alpha = PI/2.0 - acos(dp);
	return tan(alpha);
}


inline double Pathfinder::pathDynSlope(int id) {
	int nextid = (id + 1) % nPathSeg;
	vec2d dir = *psdyn->getLoc(nextid) - *psdyn->getLoc(id);
	double dp = dir*(*track->getSegmentPtr(id)->getToRight())/dir.len();
	double alpha = PI/2.0 - acos(dp);
	return tan(alpha);
}


// get the segment on which the car is, searching ALL the segments.
inline int Pathfinder::getCurrentSegment(tCarElt* car)
{
	lastId = track->getCurrentSegment(car);
	return lastId;
}


// get the segment on which the car is, searching from the position of the last call within range.
inline int Pathfinder::getCurrentSegment(tCarElt* car, int range)
{
	lastId = track->getCurrentSegment(car, lastId, range);
	return lastId;
}


inline int Pathfinder::countSegments(int from, int to)
{
	if ( to >= from) {
		return to - from;
	} else {
		return nPathSeg - from + to;
	}
}

#endif // _PATHFINDER_H_

