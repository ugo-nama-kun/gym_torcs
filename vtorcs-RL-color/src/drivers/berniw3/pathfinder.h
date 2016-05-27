/***************************************************************************

    file                 : pathfinder.h
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: pathfinder.h,v 1.1.2.1 2008/11/09 17:50:19 berniw Exp $

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
#include "trackdesc.h"
#include "mycar.h"
#include "spline.h"
#include "linalg.h"

#define FNPF "drivers/berniw/parameter.dat"
#define FNIS "drivers/berniw/intsinsqr.dat"
#define FNIC "drivers/berniw/intcossqr.dat"

/* how far we look forward for overtaking, collisions, etc */
#define AHEAD 500

/* how many segments can i pass per simulation step, depends on TRACKRES, simulation->_deltaTime and speed */
#define SEGRANGE 3

/* choose old path */
//#define PATH_BERNIW

/* choose k1999 path */
#define PATH_K1999

class MyCar;
class OtherCar;


/* holds a point of datafile from clothoid */
typedef struct {
	double x;
	double pd;
	double is;
	double ic;
} tParam;


/* holds data relative to my car */
typedef struct {
	double speedsqr;		/* on track direction projected speed squared of opponent */
	double speed;			/* same, but not squared */
	double time;			/* estimate of time to catch up the car */
	double cosalpha;		/* cos(alpha) from angle between my ond opponent */
	double disttomiddle;	/* distance to middle (for prediction) */
	int catchdist;
	int catchsegid;			/* segment, where i expect (or better guess!) to catch opponent */
	double dist;			/* #segments from me to the other car */
	OtherCar* collcar;		/* pointers to the cars */
	bool overtakee;			/* is this the guy to overtake? */
	double disttopath;		/* distance to my path */
	double brakedist;		/* distance needed for braking to the speed of this car */
	double mincorner;		/* corner nearest to my car */
	double minorthdist;		/* minimal distance relative to my car */
} tOCar;


/* holds data needed for let pass opponents */
typedef struct {
	double time;			/* how long is the opponent "in range" to overlap me */
} tOverlapTimer;



class PathSeg
{
	public:
		void set(tdble ispeedsqr, tdble ilength, v3d* ip, v3d* id);
		void set(tdble ispeedsqr, tdble ilength, v3d* id);
		inline void setLoc(v3d* ip) { p = (*ip); }
		inline void setOptLoc(v3d* ip) { o = (*ip); }
		inline void setPitLoc(v3d* ip) { l = ip; }

		inline void setSpeedsqr(tdble spsqr) { speedsqr = spsqr; }
		inline void setWeight(tdble w) { weight = w; }
		inline void setRadius(tdble r) { radius = r; }

		inline tdble getSpeedsqr() { return speedsqr; }
		inline tdble getLength() { return length; }
		inline tdble getWeight() { return weight; }
		inline tdble getRadius() { return radius; }

		inline v3d* getOptLoc() { return &o; }
		inline v3d* getPitLoc() { return l; }
		inline v3d* getLoc() { return &p; }
		inline v3d* getDir() { return &d; }

	private:
		tdble speedsqr;	/* max possible speed sqared (speed ist therefore sqrt(speedsqr) */
		tdble length;	/* dist to the next pathseg */
		tdble weight;	/* weight function value for superposition */
		tdble radius;	/* radius of current segment */
		v3d p;			/* position in space, dynamic trajectory */
		v3d o;			/* position in space, static trajectory */
		v3d d;			/* direction vector of dynamic trajectory */
		v3d* l;			/* trajectory for pit lane */
};


class Pathfinder
{
	public:
		Pathfinder(TrackDesc* itrack, tCarElt* car, tSituation *situation);
		~Pathfinder();
		void plan(int trackSegId, tCarElt* car, tSituation* situation, MyCar* myc, OtherCar* ocar);
		void plan(MyCar* myc);

		void initPit(tCarElt* car);
		inline bool isPitAvailable() { return pit; }
		inline int getPitSegId() { return pitSegId; }
		void setPitStop(bool p, int id);
		inline bool getPitStop() { return pitStop; }
		int segmentsToPit(int id);
		void plotPitStopPath(char* filename);
		void plotPath(char* filename);

		inline double sqr(double a) { return a*a; };
		inline double dist(v3d* a, v3d* b) { return sqrt(sqr(a->x-b->x) + sqr(a->y-b->y) + sqr(a->z-b->z)); }
		inline double dist2D(v3d* a, v3d* b) { return sqrt(sqr(a->x-b->x) + sqr(a->y-b->y)); }
		inline PathSeg* getPathSeg(int pathSegId) { return &ps[pathSegId]; }
		int getCurrentSegment(tCarElt* car);
		int getCurrentSegment(tCarElt* car, int range);
		inline int getnPathSeg() { return nPathSeg; }
		inline double getPitSpeedSqrLimit() { return pitspeedsqrlimit; }
		double distToPath(int trackSegId, v3d* p);

	private:
		static const double COLLDIST;	/* up to this distance do we consider other cars as dangerous */
		static const double TPRES;		/* resolution of the steps */
		enum { PITPOINTS = 7 };			/* # points for pit spline */
		enum { NTPARAMS = 1001 };		/* # entries in dat files */
		tParam cp[NTPARAMS];			/* holds values needed for clothiod */

		TrackDesc* track;		/* pointer to track data */
		int lastId;				/* segment id of the last call */
		PathSeg* ps;			/* array with the plan */
		int nPathSeg;			/* # of PathSeg's */
		int lastPlan;			/* start of the last plan */
		int lastPlanRange;		/* range of the last plan */
		bool pitStop;			/* pitstop ? */
		bool inPit;				/* internal pit state */

		int s1, s3;				/* pitentrystart, pitentryend */
		int e1, e3;				/* pitexitstart, pitexitend */

		v3d pitLoc;				/* location of pit */
		int pitSegId;			/* segment id of pit */
		bool pit;
		int changed;
		double pitspeedsqrlimit;	/* speed limit for pit lane squared */

		int collcars;
		tOCar* o;
		tOverlapTimer* overlaptimer;
		v3d* pitcord;

		void initPitStopPath(void);
		void getPitPoint(int j, int k, double slope, double dist, v3d* r);
		int collision(int trackSegId, tCarElt* mycar, tSituation *s, MyCar* myc, OtherCar* ocar);
		int overtake(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar);
		double curvature(double xp, double yp, double x, double y, double xn, double yn);
		void adjustRadius(int s, int p, int e, double c, double carwidth);
		void stepInterpolate(int iMin, int iMax, int Step);
		void interpolate(int Step);
		void smooth(int Step);

		int correctPath(int id, tCarElt* car, MyCar* myc);

		bool loadClothoidParams(tParam* p);
		double intsinsqr(double alpha);
		double intcossqr(double alpha);
		double clothparam(double alpha);
		double clothsigma(double beta, double y);
		double clothlength(double beta, double y);

		int findStartSegId(int id);
		int findEndSegId(int id);
		int initStraight(int id, double w);
		int initLeft(int id, double w);
		int initRight(int id, double w);
		double computeWeight(double x, double len);
		void setLocWeighted(int id, double newweight, v3d* newp);
		void smooth(int s, int e, int p, double w);
		void smooth(int id, double delta, double w);
		void optimize(int start, int range, double w);
		void optimize2(int start, int range, double w);
		void optimize3(int start, int range, double w);
		int updateOCar(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o);
		void updateOverlapTimer(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o, tOverlapTimer* ov);
        int letoverlap(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOverlapTimer* ov);
		double pathSlope(int id);
		int countSegments(int from, int to);
};


inline double Pathfinder::distToPath(int trackSegId, v3d* p)
{
	v3d *toright = track->getSegmentPtr(trackSegId)->getToRight();
	v3d *pathdir = ps[trackSegId].getDir();
	v3d n1, torightpath;
	toright->crossProduct(pathdir, &n1);
	pathdir->crossProduct(&n1, &torightpath);
	return ((*p - *ps[trackSegId].getLoc())*torightpath)/torightpath.len();
}


inline void PathSeg::set(tdble ispeedsqr, tdble ilength, v3d* ip, v3d* id) {
	speedsqr = ispeedsqr;
	length = ilength;
	p = (*ip);
	d = (*id);
}


inline void PathSeg::set(tdble ispeedsqr, tdble ilength, v3d* id) {
	speedsqr = ispeedsqr;
	length = ilength;
	d = (*id);
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


inline double Pathfinder::pathSlope(int id) {
	int nextid = (id + 1) % nPathSeg;
	v3d dir = *ps[nextid].getLoc() - *ps[id].getLoc();
	double dp = dir*(*track->getSegmentPtr(id)->getToRight())/dir.len();
	double alpha = PI/2.0 - acos(dp);
	return tan(alpha);
}


/* get the segment on which the car is, searching ALL the segments */
inline int Pathfinder::getCurrentSegment(tCarElt* car)
{
	lastId = track->getCurrentSegment(car);
	return lastId;
}


/* get the segment on which the car is, searching from the position of the last call within range */
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

