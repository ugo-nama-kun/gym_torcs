/***************************************************************************

    file                 : pathfinder.cpp
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2006 by Bernhard Wymann, some Code from Remi Coulom, K1999.cpp
    email                : berniw@bluewin.ch
    version              : $Id: pathfinder.cpp,v 1.1.2.1 2008/05/27 22:33:17 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "pathfinder.h"
#include "berniw.h"

const double Pathfinder::COLLDIST = 150.0;
PathSegOpt* Pathfinder::psopt = NULL;
bool Pathfinder::optpathinitialized = false;


Pathfinder::Pathfinder(TrackDesc* itrack, tCarElt* car, tSituation *s)
{
	int i;
	track = itrack;
	tTrack* t = track->getTorcsTrack();
	o = new tOCar[s->_ncars];

	// Set team mate, TODO: support multiple teammates.
	teammate = NULL;
	char *teammatename = GfParmGetStr(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_TEAMMATE, NULL);
	// Teammate defined in XML setup file?
	if (teammatename != NULL) {
		// Teammate as well in the race?
		for (i = 0; i < s->_ncars; i++) {
			if (strcmp(s->cars[i]->_name, teammatename) == 0 && car != s->cars[i]) {
				teammate = s->cars[i];
				break;
			}
		}
	}

	overlaptimer = new tOverlapTimer[s->_ncars];

	for (i = 0; i < s->_ncars; i++) {
		overlaptimer[i].time = 0.0;
	}

	// The optimal path has to have one point per tracksegment.
	nPathSeg = track->getnTrackSegments();

	// Get memory for the optimal path, just once shared by all instances.
	if (psopt == NULL) {
		psopt = new PathSegOpt(nPathSeg);
	}

	// Get memory for dynamic trajectory.
	psdyn = new PathSeg(PATHBUF, nPathSeg);
	changed = lastPlan = lastPlanRange = 0;
	inPit = pitStop = false;

	// Check if there is a pit type we can use and if for this car is a pit available.
	pit = false;
	if (t->pits.type == TR_PIT_ON_TRACK_SIDE && car->_pit != NULL) {
		pit = true;
	}
	
	s1 = e3 = 0;
	if (isPitAvailable()) {
		initPit(car);
		// The values in the setup file must be for trackres == 1.0.
		s1 = track->getPitEntryStartId();
		s1 = (int) (GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_PITENTRY, (char*)NULL, s1*TRACKRES)/TRACKRES);
		e3 = track->getPitExitEndId();
		e3 = (int) (GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_PITEXIT, (char*)NULL, e3*TRACKRES)/TRACKRES);
		pitspeedsqrlimit = t->pits.speedLimit - 0.5;
		pitspeedsqrlimit *= pitspeedsqrlimit;
		pspit = new PathSegPit(countSegments(s1, e3), nPathSeg, s1, e3, psopt);
	}
}


Pathfinder::~Pathfinder()
{
	delete psdyn;
	if (psopt != NULL) {
		delete psopt;
		psopt = NULL;
		optpathinitialized = false;
	}
	if (isPitAvailable()) {
		delete pspit;
	}
	delete [] o;
	delete [] overlaptimer;
}


// Compute where the pit is, etc.
void Pathfinder::initPit(tCarElt* car) {
	tTrack* t = track->getTorcsTrack();

	if (t->pits.driversPits != NULL && car != NULL) {
		if (isPitAvailable()) {
			tTrackSeg* pitSeg = car->_pit->pos.seg;
			if (pitSeg->type == TR_STR) {
				vec2d v1, v2, v3;
				// v1 points in the direction of the segment.
				v1.x = pitSeg->vertex[TR_EL].x - pitSeg->vertex[TR_SL].x;
				v1.y = pitSeg->vertex[TR_EL].y - pitSeg->vertex[TR_SL].y;
				v1.normalize();

				// v2 points to the side of the segment.
				double s = (t->pits.side == TR_LFT) ? -1.0 : 1.0 ;
				v2.x = s*(pitSeg->vertex[TR_SR].x - pitSeg->vertex[TR_SL].x);
				v2.y = s*(pitSeg->vertex[TR_SR].y - pitSeg->vertex[TR_SL].y);
				v2.normalize();

				// Loading starting point of segment.
				pitLoc.x = (pitSeg->vertex[TR_SR].x + pitSeg->vertex[TR_SL].x) / 2.0;
				pitLoc.y = (pitSeg->vertex[TR_SR].y + pitSeg->vertex[TR_SL].y) / 2.0;

				// Going along the track.
				pitLoc = pitLoc + (double)(car->_pit->pos.toStart)*v1;
				pitSegId = track->getNearestId(&pitLoc);

				// Going sideways, minus because of opposite sign of v2 and the value toMiddle.
				double m = fabs(t->pits.driversPits->pos.toMiddle);
				v3 = pitLoc + m*v2;

				tTrackSeg* tmpSeg = t->pits.pitStart;	
				v2.x = (tmpSeg->vertex[TR_SR].x + tmpSeg->vertex[TR_SL].x) / 2.0;
				v2.y = (tmpSeg->vertex[TR_SR].y + tmpSeg->vertex[TR_SL].y) / 2.0;

				// FIXME: this could cause problems, integrate bt pit stops.
				double dist = (v2-pitLoc).len() - 2.0/TRACKRES;
				if (dist < t->pits.len) {
					v2 = pitLoc - v1*(t->pits.len + 2.0/TRACKRES);
				}
				s3 = track->getNearestId(&v2);

				tmpSeg = t->pits.pitEnd;
				v2.x = (tmpSeg->vertex[TR_ER].x + tmpSeg->vertex[TR_EL].x) / 2.0;
				v2.y = (tmpSeg->vertex[TR_ER].y + tmpSeg->vertex[TR_EL].y) / 2.0;
				// FIXME: this could cause problems, integrate bt pit stops.
				dist = (pitLoc-v2).len() - 2.0/TRACKRES;
				if (dist < t->pits.len) {
					v2 = pitLoc + v1*(t->pits.len + 2.0/TRACKRES);
				}				
				e1 = track->getNearestId(&v2);
			

				pitLoc = v3;
			} else {
				pit = false;
			}
		}
	}
}


// Call this after you computed a static racing path with plan().
void Pathfinder::initPitStopPath(void)
{
	tTrack* t = track->getTorcsTrack();
	vec2d p, q;
	const vec2d *pp, *pmypitseg = track->getSegmentPtr(pitSegId)->getMiddle();
	double d, dp, sgn;
	double delta = t->pits.width;
	int i;
	double ypit[PITPOINTS], yspit[PITPOINTS], spit[PITPOINTS];
	int snpit[PITPOINTS];

	// Set up point 0 on the track (s1).
	ypit[0] = track->distToMiddle(s1, psopt->getOptLoc(s1));
	snpit[0] = s1;

	// Set up point 1 pit lane entry (s3).
	track->dirVector2D(&pitLoc, pmypitseg, &p);
	dp = p.len();
	d = dp - delta;

	sgn = (t->pits.side == TR_LFT) ? -1.0 : 1.0 ;
	ypit[1] = d*sgn;
	snpit[1] = s3;

	// Set up point 2 before we turn into the pit.
    // FIXME: this could cause problems, integrate bt pit stops.
	i = (pitSegId - (int) (t->pits.len/TRACKRES) + nPathSeg) % nPathSeg;
	ypit[2] = d*sgn;
	snpit[2] = i;

	// Set up point 3, the pit, we know this already.
	ypit[3] = dp*sgn;
	snpit[3] = pitSegId;

	// Compute point 4, go from pit back to pit lane.
	i = (pitSegId + (int) (t->pits.len/TRACKRES) + nPathSeg) % nPathSeg;
	ypit[4] = d*sgn;
	snpit[4] = i;

	// compute point 5, drive to end of pit lane (e1).
	ypit[5] = d*sgn;
	snpit[5] = e1;

	// Compute point 6, back on the track.
	ypit[6] = track->distToMiddle(e3, psopt->getOptLoc(e3));
	snpit[6] = e3;

	// Compute spit array.
	spit[0] = 0.0;
	for (i = 1; i < PITPOINTS; i++) {
		d = 0.0;
		for (int j = snpit[i-1]; (j + 1) % nPathSeg !=  snpit[i]; j++) {
			if (snpit[i] > snpit[i-1]) {
				d = (double) (snpit[i] - snpit[i-1]);
			} else {
				d = (double) (nPathSeg - snpit[i-1] + snpit[i]);
			}
		}
		spit[i] = spit[i-1] + d;
	}

	// Scale for track resolution.
	for (i = 1; i < PITPOINTS; i++) {
		spit[i] = TRACKRES*spit[i];
	}

	// Set up slopes.
	yspit[0] = pathOptSlope(s1);
	yspit[6] = pathOptSlope(e3);

	for (i = 1; i < PITPOINTS-1; i++) {
		yspit[i] = 0.0;
	}

	// Compute path to pit.
	double l = 0.0;
	for (i = s1; (i + nPathSeg) % nPathSeg != e3; i++) {
		int j = (i + nPathSeg) % nPathSeg;
		d = spline(PITPOINTS, l, spit, ypit, yspit);

		pp = track->getSegmentPtr(j)->getMiddle();
		p = *track->getSegmentPtr(j)->getToRight();;
		q = *pp + p*d;
		pspit->setPitLoc(&q, j);
		l += TRACKRES;
	}
}


// Plots pit path to file for gnuplot.
// gnuplot command: plot 'filename'
void Pathfinder::plotPitStopPath(char* filename)
{
	FILE* fd = fopen(filename, "w");

	// Plot pit path.
	for (int i = 0; i < nPathSeg; i++) {
		fprintf(fd, "%f\t%f\n", pspit->getPitLoc(i)->x, pspit->getPitLoc(i)->y);
	}
	fclose(fd);
}


// Plots the optimal path.
void Pathfinder::plotPath(char* filename)
{
	FILE* fd = fopen(filename, "w");

	// Plot path.
	for (int i = 0; i < nPathSeg; i++) {
		fprintf(fd, "%f\t%f\n", psopt->getOptLoc(i)->x, psopt->getOptLoc(i)->y);
	}
	fclose(fd);
}


// Plans a static trajectory ignoring current situation.
void Pathfinder::plan(MyCar* myc, int currentsegid)
{
	double r, length, speedsqr;
	int u, v, w;
	vec2d dir;
	int i,j;

	// Compute optimal path if not already done.
	if (optpathinitialized == false) {
		// Basic initialisation with track middle.
		for (i = 0; i < nPathSeg; i++) {
			psopt->setOptLoc(track->getSegmentPtr(i)->getMiddle(), i);
		}

		// Compute optimal path.
		for (int step = 128; (step /= 2) > 0;) {
			for (int i = 100 * int(sqrt(float(step))); --i >= 0;) smooth(step);
			interpolate(step);
		}
		optpathinitialized = true;

		// Now init the other fields of the optimal path.
		// Compute the tangent as slopes of the spline connecting the path.
		double* sp_x = new double[nPathSeg+1];
		double* sp_y = new double[nPathSeg+1];
		double* sp_xs = new double[nPathSeg+1];
		double* sp_ys = new double[nPathSeg+1];
		double* sp_s = new double[nPathSeg+1];
		
		for (i = 0; i < nPathSeg; i++) {
			sp_x[i] = psopt->getOptLoc(i)->x;
			sp_y[i] = psopt->getOptLoc(i)->y;
		}
		sp_x[nPathSeg] = sp_x[0];
		sp_y[nPathSeg] = sp_y[0];

		// Slopes for closed spline.
		parametricslopesp(nPathSeg+1, sp_x, sp_y, sp_xs, sp_ys, sp_s);

		for (i = 0; i < nPathSeg; i++) {
			// We want not the tangent, we want the perpendicular vector to right, so swap sx, sy and
			// fudge the signs (direction).
			vec2d slope(sp_ys[i], -sp_xs[i]);
			slope.normalize();
			psopt->setOptToRight(&slope, i);
			// Set the length.
			psopt->setOptLength((*psopt->getOptLoc((i+1) % nPathSeg) - *psopt->getOptLoc(i)).len(), i);
		}

		for (i = 0; i < nPathSeg; i++) {
			float f = ((*psopt->getOptLoc(i)) - (*track->getSegmentPtr(i)->getMiddle()))*(*track->getSegmentPtr(i)->getToRight());
			psopt->setOptToMiddle(f, i);
		}

		delete [] sp_x;
		delete [] sp_y;
		delete [] sp_xs;
		delete [] sp_ys;
		delete [] sp_s;
	}

	// Init pit ond optimal path.
	int bufstartseg = (currentsegid - BACK + nPathSeg) % nPathSeg;

	psdyn->setBase(bufstartseg);
	for (i = bufstartseg; i < bufstartseg + PATHBUF; i++) {
		j = i % nPathSeg;
		psdyn->setLoc(psopt->getOptLoc(j), j);
	}

	// Compute possible speeds, direction vector and length of trajectoies.
	u = (nPathSeg + bufstartseg - 1) % nPathSeg; v = bufstartseg; w = (bufstartseg + 1) % nPathSeg;

	for (j = bufstartseg; j < bufstartseg + PATHBUF; j++) {
		i = j % nPathSeg;
		r = radius(psopt->getOptLoc(u)->x, psopt->getOptLoc(u)->y,
			psopt->getOptLoc(v)->x, psopt->getOptLoc(v)->y, psopt->getOptLoc(w)->x, psopt->getOptLoc(w)->y);
		psdyn->setRadius(r, i);
		r = fabs(r);

		length = dist(psopt->getOptLoc(v), psopt->getOptLoc(w));

		tdble mu = track->getSegmentPtr(i)->getKfriction()*myc->CFRICTION*track->getSegmentPtr(i)->getKalpha();
		tdble b = track->getSegmentPtr(i)->getKbeta();
		speedsqr = myc->SPEEDSQRFACTOR*r*g*mu/(1.0 - MIN(1.0, (mu*myc->ca*r/myc->mass)) + mu*r*b);

		dir = (*psopt->getOptLoc(w)) - (*psopt->getOptLoc(u));
		dir.normalize();

		psdyn->set(speedsqr, length, &dir, i);

		u = v; v = w; w = (w + 1 + nPathSeg) % nPathSeg;
	}

	// Add path to pit if a pit is available.
	if (isPitAvailable()) {
		initPitStopPath();
	}
}


// Plans a trajectory according to the situation.
void Pathfinder::plan(int trackSegId, tCarElt* car, tSituation *situation, MyCar* myc, OtherCar* ocar)
{
	double r, length, speedsqr;
	int u, v, w;
	vec2d dir;

	int bufstartseg = (trackSegId - BACK + nPathSeg) % nPathSeg;
	psdyn->setBase(bufstartseg);

	int i, start;

	if (myc->derror > myc->PATHERR*myc->PATHERRFACTOR) {
		start = trackSegId;
	} else {
		start = lastPlan+lastPlanRange;
	}

	if (track->isBetween(e3, s1, trackSegId)) {
		inPit = false;
	}
	// Relies on that pitstop dosen't get enabled between s1, e3.
	if (track->isBetween(s1, e3, trackSegId) && (pitStop)) {
		inPit = true;
	}

	// Load precomputed trajectory.
	if (!pitStop && !inPit) {
		for (i = start; i < trackSegId+AHEAD+SEGRANGE; i++) {
			int j = (i+nPathSeg) % nPathSeg;
			psdyn->setLoc(psopt->getOptLoc(j), j);
		}
	} else {
		for (i = start; i < trackSegId+AHEAD+SEGRANGE; i++) {
			int j = (i+nPathSeg) % nPathSeg;
			psdyn->setLoc(pspit->getPitLoc(j), j);
		}
	}

	// Update data about the opponents relative to me.
	collcars = updateOCar(trackSegId, situation, myc, ocar, o);
	updateOverlapTimer(trackSegId, situation, myc, ocar, o, overlaptimer);

	if (!inPit && (!pitStop || track->isBetween(e3, (s1 - AHEAD + nPathSeg) % nPathSeg, trackSegId))) {
		// Are we on the trajectory or do I need a correction.
		if ((myc->derror > myc->PATHERR*myc->PATHERRFACTOR ||
			(myc->getDeltaPitch() > myc->MAXALLOWEDPITCH && myc->getSpeed() > myc->FLYSPEED)))
		{
			changed += correctPath(trackSegId, car, myc);
		}

		// Overtaking.
		if (changed == 0) {
			changed += overtake(trackSegId, situation, myc, ocar);
		}

		// If we have nothing better to, let opponents overlap.
		if (changed == 0) {
			changed = letoverlap(trackSegId, situation, myc, ocar, overlaptimer);
		}
	}

	// Recompute speed and direction of new trajectory.
	if (changed > 0 || (psdyn->getSpeedsqr(trackSegId) < 5.0)) {
		start = trackSegId;
	}

	u = start - 1; v = start; w = start+1;
	int u2 = (start - 3 + nPathSeg) % nPathSeg;
	int w2 = (start + 3 + nPathSeg) % nPathSeg;
	u = (u + nPathSeg) % nPathSeg;
	v = (v + nPathSeg) % nPathSeg;
	w = (w + nPathSeg) % nPathSeg;

	for (i = start; i < trackSegId+AHEAD+SEGRANGE; i++) {
		int j = (i+nPathSeg) % nPathSeg;
		/* taking 2 radiuses to reduce "noise" */

		double r2 = radius(psdyn->getLoc(u)->x, psdyn->getLoc(u)->y,
			psdyn->getLoc(v)->x, psdyn->getLoc(v)->y, psdyn->getLoc(w)->x, psdyn->getLoc(w)->y);

		double r1 = radius(psdyn->getLoc(u2)->x, psdyn->getLoc(u2)->y,
			psdyn->getLoc(v)->x, psdyn->getLoc(v)->y, psdyn->getLoc(w2)->x, psdyn->getLoc(w2)->y);

		if (fabs(r1) > fabs(r2)) {
			psdyn->setRadius(r1, j);
			r = fabs(r1);
		} else {
			psdyn->setRadius(r2, j);
			r = fabs(r2);
		}

		length = dist(psdyn->getLoc(v), psdyn->getLoc(w));

		// Compute allowed speed squared.
		double mu = track->getSegmentPtr(j)->getKfriction()*myc->CFRICTION*track->getSegmentPtr(j)->getKalpha();
		double b = track->getSegmentPtr(j)->getKbeta();
		speedsqr = myc->SPEEDSQRFACTOR*r*g*mu/(1.0 - MIN(1.0, (mu*myc->ca*r/myc->mass)) + mu*r*b);
		if (pitStop && track->isBetween(s3, pitSegId, j)) {
			double speedsqrpit = ((double) segmentsToPit(j) * TRACKRES) *2.0*g*track->getSegmentPtr(j)->getKfriction()*myc->CFRICTION*myc->cgcorr_b;
			if (speedsqr > speedsqrpit) {
				speedsqr = speedsqrpit;
			}
		}
		if ((pitStop || inPit) && track->isBetween(s3, e1, j)) {
			if (speedsqr > getPitSpeedSqrLimit()) {
				speedsqr = getPitSpeedSqrLimit();
			}
		}

		dir = (*psdyn->getLoc(w)) - (*psdyn->getLoc(u));
		dir.normalize();

		psdyn->set(speedsqr, length, &dir, j);

		u = v; v = w; w = (w + 1 + nPathSeg) % nPathSeg;
		w2 = (w2 + 1 + nPathSeg) % nPathSeg;
		u2 = (u2 + 1 + nPathSeg) % nPathSeg;
	}

	changed = 0;

	/* set speed limits on the path, in case there is an obstacle (other car) */
	changed += collision(trackSegId, car, situation, myc, ocar);

	lastPlan = trackSegId; lastPlanRange = AHEAD;
}


void Pathfinder::smooth(int s, int p, int e, double w)
{
	TrackSegment2D* t = track->getSegmentPtr(p);
	const vec2d *rgh = t->getToRight();
	vec2d *rs = psdyn->getLoc(s), *rp = psdyn->getLoc(p), *re = psdyn->getLoc(e), n;

	double rgx = (re->x - rs->x), rgy = (re->y - rs->y);
	double m = ((rs->x - rp->x)*rgy + (rp->y - rs->y)*rgx ) / (rgy * rgh->x - rgx * rgh->y);
	n = (*rp) + (*rgh)*m;
	psdyn->setLoc(&n, p);
}


/* collision avoidence with braking */
int Pathfinder::collision(int trackSegId, tCarElt* mycar, tSituation* s, MyCar* myc, OtherCar* ocar)
{
	int end = (trackSegId + (int) (COLLDIST/TRACKRES) + nPathSeg) % nPathSeg;
	int didsomething = 0;
	int i, n = collcars;

	for (i = 0; i < n; i++) {
// TODO: magic number
// TODO: Move out of pathfinder to improve performance.
		if (o[i].overtakee == true || (o[i].time > myc->TIMETOCATCH-0.1 && o[i].collcar->getSpeed() < 10.0)) continue;
		int currentsegid = o[i].collcar->getCurrentSegId();
		if (track->isBetween(trackSegId, end, currentsegid) && (myc->getSpeed() > o[i].speed)) {
			int spsegid = (currentsegid - (int) ((myc->CARLEN + 1)/TRACKRES) + nPathSeg) % nPathSeg;

			// TODO: try to use relative speed.
			if (o[i].mincorner < myc->CARWIDTH/2.0 + (myc->DIST*MIN(1.0, o[i].collcar->getSpeed()/28.0))) {
				double cmpdist = o[i].dist - myc->CARLEN - myc->DIST;
				if ((o[i].brakedist >= cmpdist) && (psdyn->getSpeedsqr(spsegid) > o[i].speedsqr)) {
					int j;
					int adv = MAX(1, (int) (3.0/TRACKRES+0.5));
					for (j = spsegid - adv; j < spsegid + adv; j++) {
						psdyn->setSpeedsqr(o[i].speedsqr, (j + nPathSeg) % nPathSeg);
					}
					didsomething = 1;
				}
			}

			if (track->isBetween(trackSegId, end, o[i].catchsegid)) {
				double myd = track->distToMiddle(o[i].catchsegid, psdyn->getLoc(o[i].catchsegid));
				double sina = o[i].collcar->getDir()->fakeCrossProduct(myc->getDir());
				double otherd = o[i].disttomiddle + sina*o[i].collcar->getSpeed()*o[i].time;

				if (fabs(myd - otherd) < myc->CARWIDTH + myc->DIST*MIN(1.0, o[i].collcar->getSpeed()/28.0)) {
					if ((o[i].catchdist > 0.0) && (o[i].brakedist >= (o[i].catchdist - (myc->CARLEN + myc->DIST)))) {
						int catchsegid = ((o[i].catchsegid - (int) ((myc->CARLEN + 1)/TRACKRES) + nPathSeg) % nPathSeg);
						if (psdyn->getSpeedsqr(catchsegid) > o[i].speedsqr) {
							psdyn->setSpeedsqr(o[i].speedsqr, catchsegid);
							didsomething = 1;
						}
					}
				}
			}

		}
	}
	return didsomething;
}



inline double Pathfinder::curvature(double xp, double yp, double x, double y, double xn, double yn)
{
	return 1.0/radius(xp, yp, x, y, xn, yn);
}


// Optimize point p ala k1999 (curvature), Remi Coulom, K1999.cpp.
inline void Pathfinder::adjustRadius(int s, int p, int e, double c, double security) {
	const double sidedistext = 2.0;
	const double sidedistint = 1.2;

	TrackSegment2D* t = track->getSegmentPtr(p);
	const vec2d *rgh = t->getToRight();
	const vec2d *left = t->getLeftBorder();
	const vec2d *right = t->getRightBorder();
	const vec2d *rs = psopt->getOptLoc(s), *rp = psopt->getOptLoc(p), *re = psopt->getOptLoc(e);
	double oldlane = track->distToMiddle(p, rp)/t->getWidth() + 0.5;

	double rgx = (re->x - rs->x), rgy = (re->y - rs->y);
	double m = ((rs->x - rp->x)*rgy + (rp->y - rs->y)*rgx ) / (rgy * rgh->x - rgx * rgh->y);

	if (m < -t->getWidth()) {
		m = -t->getWidth();
	}
	if (m > t->getWidth()) {
		m = t->getWidth();
	}

	vec2d n = (*rp) +(*rgh)*m;
	psopt->setOptLoc(&n, p);
	double newlane = track->distToMiddle(p, rp)/t->getWidth() + 0.5;

	// Get an estimate how much the curvature changes by moving the point 1/10000 of track width.
	const double delta = 0.0001;
	double dx = delta * (right->x - left->x);
	double dy = delta * (right->y - left->y);
	double deltacurvature = curvature(rs->x, rs->y, rp->x + dx, rp->y + dy, re->x, re->y);

	if (deltacurvature > 0.000000001) {
		newlane += (delta / deltacurvature) * c;
		double ExtLane = (sidedistext + security) / t->getWidth();
		double IntLane = (sidedistint + security) / t->getWidth();

		if (ExtLane > 0.5) ExtLane = 0.5;
		if (IntLane > 0.5) IntLane = 0.5;

		if (c >= 0.0) {
			if (newlane < IntLane) newlane = IntLane;
			if (1 - newlane < ExtLane) {
    			if (1 - oldlane < ExtLane) newlane = MIN(oldlane, newlane);
    			else newlane = 1 - ExtLane;
			}
		} else {
			if (newlane < ExtLane) {
    			if (oldlane < ExtLane) newlane = MAX(oldlane, newlane);
    			else newlane = ExtLane;
			}
			if (1 - newlane < IntLane) newlane = 1 - IntLane;
		}

		double d = (newlane - 0.5) * t->getWidth();
		const vec2d* trackmiddle = t->getMiddle();

		n = (*trackmiddle) + (*rgh)*d;
		psopt->setOptLoc(&n, p);
	}
}


// Interpolation step from Remi Coulom, K1999.cpp.
void Pathfinder::stepInterpolate(int iMin, int iMax, int Step)
{
	int next = (iMax + Step) % nPathSeg;
	if (next > nPathSeg - Step) next = 0;

	int prev = (((nPathSeg + iMin - Step) % nPathSeg) / Step) * Step;
	if (prev > nPathSeg - Step)
	prev -= Step;

	const vec2d *pp = psopt->getOptLoc(prev);
	const vec2d *p = psopt->getOptLoc(iMin);
	const vec2d *pn = psopt->getOptLoc(iMax % nPathSeg);
	const vec2d *pnn = psopt->getOptLoc(next);

	double ir0 = curvature(pp->x, pp->y, p->x, p->y, pn->x, pn->y);
	double ir1 = curvature(p->x, p->y, pn->x, pn->y, pnn->x, pnn->y);

	for (int k = iMax; --k > iMin;) {
		double x = double(k - iMin) / double(iMax - iMin);
		double TargetRInverse = x * ir1 + (1 - x) * ir0;
		adjustRadius(iMin, k, iMax % nPathSeg, TargetRInverse, 0.0);
	}
}


// Interpolating from Remi Coulom, K1999.cpp.
void Pathfinder::interpolate(int step)
{
	if (step > 1) {
		int i;
		for (i = step; i <= nPathSeg - step; i += step) stepInterpolate(i - step, i, step);
		stepInterpolate(i - step, nPathSeg, step);
	}
}


// Smoothing from Remi Coulom, K1999.cpp.
void Pathfinder::smooth(int Step)
{
	int prev = ((nPathSeg - Step) / Step) * Step;
	int prevprev = prev - Step;
	int next = Step;
	int nextnext = next + Step;

	const vec2d *pp, *p, *n, *nn, *cp;

	for (int i = 0; i <= nPathSeg - Step; i += Step) {
		pp = psopt->getOptLoc(prevprev);
		p = psopt->getOptLoc(prev);
		cp = psopt->getOptLoc(i);
		n = psopt->getOptLoc(next);
		nn = psopt->getOptLoc(nextnext);

		double ir0 = curvature(pp->x, pp->y, p->x, p->y, cp->x, cp->y);
		double ir1 = curvature(cp->x, cp->y, n->x, n->y, nn->x, nn->y);
		double dx, dy;
		dx = cp->x - p->x; dy = cp->y - p->y;
		double lPrev = sqrt(dx*dx + dy*dy);
		dx = cp->x - n->x; dy = cp->y - n->y;
		double lNext = sqrt(dx*dx + dy*dy);

		double TargetRInverse = (lNext * ir0 + lPrev * ir1) / (lNext + lPrev);

		double Security = lPrev * lNext / (8.0 * 100.0);
		adjustRadius(prev, i, next, TargetRInverse, Security);

		prevprev = prev;
		prev = i;
		next = nextnext;
		nextnext = next + Step;
		if (nextnext > nPathSeg - Step) nextnext = 0;
	}
}


// Compute a path back to the planned path.
int Pathfinder::correctPath(int id, tCarElt* car, MyCar* myc)
{
	double s[2], y[2], ys[2];
	bool out;

	double d = track->distToMiddle(id, myc->getCurrentPos());
	double factor = MIN(MIN(myc->CORRLEN/TRACKRES*myc->derror, nPathSeg/2.0), AHEAD);
	int endid = (id + (int) (factor) + nPathSeg) % nPathSeg;

	// Are we outside the track or on the track?
	if (fabs(d) > (track->getSegmentPtr(id)->getWidth() - myc->CARWIDTH)/2.0) {
		d = sign(d)*((track->getSegmentPtr(id)->getWidth() - myc->CARWIDTH)/2.0 - myc->MARGIN);
		vec2d pathdir = *psdyn->getDir(id);
		vec2d pathtoright(pathdir.y, -pathdir.x);
		vec2d trackdir(-track->getSegmentPtr(id)->getToRight()->y, track->getSegmentPtr(id)->getToRight()->x);
		double alpha = PI/2.0 - acos(trackdir*pathtoright);
		ys[0] = tan(alpha);
		out = true;
	} else {
		vec2d pathdir = *psdyn->getDir(id);
		vec2d pathtoright(pathdir.y, -pathdir.x);
		double alpha = PI/2.0 - acos((*myc->getDir())*pathtoright);
		ys[0] = tan(alpha);
		out = false;
	}

	// Set up points for spline.
	y[0] = myc->getErrorSgn()*myc->derror;
	y[1] = 0.0;
	ys[1] = 0.0;

	s[0] = 0.0;

	int i, j;
	s[1] = 0.0;
	for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
		s[1] += psdyn->getLength(j);
	}

	// Modify path.
	double l = 0.0;
	vec2d q;
	const vec2d *pp, *qq;

	if (out == true) {
		// We are off the track, so we take the optimal trajectory as base and reject the current one.
		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			d = spline(2, l, s, y, ys);
			float d1 = ((*psdyn->getLoc(j)) - (*track->getSegmentPtr(j)->getMiddle()))*(*track->getSegmentPtr(j)->getToRight());
			float d2 = d1 + d;

			if (fabs(d2) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH)/2.0) {
				d = sign(d)*((track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH)/2.0 - myc->MARGIN - fabs(d1));
			}

			pp = psopt->getOptLoc(j);
			qq = psopt->getOptToRight(j);
			q = (*pp) + (*qq)*d;
			psdyn->setLoc(&q, j);
			l += psdyn->getLength(j);
		}

		// Reload optimal trajectory where needed.
		for (i = endid; (j = (i+nPathSeg) % nPathSeg) != (id+AHEAD) % nPathSeg; i++) {
			psdyn->setLoc(psopt->getOptLoc(j), j);
		}
	} else {
		// We are on the track, therefore we take the planned dynamic trajectory as base.
		double newdisttomiddle[AHEAD];
		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			d = spline(2, l, s, y, ys);
			float d2 = ((*psdyn->getLoc(j)) - (*track->getSegmentPtr(j)->getMiddle()))*(*track->getSegmentPtr(j)->getToRight()) + d;
			//printf("d2: %f, limit: %f\n", d2, (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - myc->MARGIN);
			if (fabs(d2) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - myc->MARGIN) {
				return 0;
			}
			newdisttomiddle[i - id] = d;
			l += psdyn->getLength(j);
		}

		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			pp = psdyn->getLoc(j);
			qq = psopt->getOptToRight(j);
			q = *pp + (*qq)*newdisttomiddle[i - id];
			psdyn->setLoc(&q, j);

		}

		// Reload of optimal trajectory here is not allowed, because we want connect to the dynamic trajectory.
	}

	// Align previous point for getting correct speedsqr in Pathfinder::plan(...).
	int p = (id - 1 + nPathSeg) % nPathSeg;
	int e = (id + 1 + nPathSeg) % nPathSeg;
	smooth(id, p, e, 1.0);

	return 1;
}


/* compute path for overtaking the "next colliding" car */
int Pathfinder::overtake(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar)
{
	if (collcars == 0) return 0;

	int i, j, m = 0;
	float freefrommiddle = -1000.0; // Invlaid, nothing free.

	if (collcars > 1) {
		int lanesize = (int) track->getSegmentPtr(trackSegId)->getWidth();
		int* freelane = new int[lanesize];

		for (i = 0; i < lanesize; i++) {
			freelane[i] = 0;
		}

		for (i = 0; i < collcars; i++) {
			if (o[i].speed > myc->getSpeed()) continue;
			int lid = (int) (o[i].disttomiddle + lanesize/2.0 + 0.5);
			if (lid > 2 && lid < lanesize-2) {
				freelane[lid] += 1;
				freelane[lid-1] += 1;
				freelane[lid+1] += 1;
				if (o[i].width/2.0 > 1.5) {
					freelane[lid-2] += 1;
					freelane[lid+2] += 1;
				}
			} else if (lid >= 0 && lid < lanesize){
				freelane[lid] += 2;
			}
		}

		// Search free lane.
		for (i = 0; i < lanesize-4; i++) {
			if (freelane[i] == 0 && freelane[i+1] == 0 &&
				freelane[i+2] == 0 && freelane[i+3] == 0
			) {
				float l = (i+2) - track->getSegmentPtr(trackSegId)->getWidth()/2.0;
				if (fabs(l) < fabs(freefrommiddle)) {
					freefrommiddle = l;
				}
			}
		}

		delete [] freelane;
	}

	const int start = (trackSegId - (int) ((2.0 + myc->CARLEN)/TRACKRES) + nPathSeg) % nPathSeg;
	const int nearend = (trackSegId + (int) (2.0*myc->CARLEN/TRACKRES)) % nPathSeg;

	OtherCar* nearestCar = NULL;	/* car near in time, not in space ! (next reached car) */
	double minTime = FLT_MAX;
	double minorthdist = FLT_MAX;	/* near in space */
	double orthdist = FLT_MAX;
	int minorthdistindex = 0;
	int collcarindex = 0;

	//int i, m = 0;
	for (i = 0; i < collcars; i++) {
// TODO: MAGIC number
		if (o[i].dist < COLLDIST*1.5) { // always?
			double dst = o[i].minorthdist;
			if (o[i].time > 0.0 && o[i].time < minTime) {
				minTime = o[i].time;
				collcarindex = i;
				orthdist = dst;
			}
			if (dst < minorthdist && track->isBetween(start, nearend, o[i].collcar->getCurrentSegId())) {
				minorthdist = dst;
				minorthdistindex = i;
			}
			m++;
		}
	}

	if (m == 0) return 0;

	bool sidechangeallowed;

	if (minorthdist <= myc->OVERTAKEMINDIST && o[collcarindex].dist >= o[minorthdistindex].dist) {
		collcarindex = minorthdistindex;
		nearestCar = o[minorthdistindex].collcar;

		// Reject overtaking teammate if possible.
		if (teammate != NULL && nearestCar->getCarPtr() == teammate) {
			if (o[minorthdistindex].dist > (nearestCar->getCarPtr()->_dimension_x + myc->getCarPtr()->_dimension_x)/2.0 &&
				(nearestCar->getCarPtr()->_dammage - myc->getCarPtr()->_dammage) < myc->TEAM_DAMAGE_CHANGE_LEAD)
			{
				return 0;
			}
		}

		sidechangeallowed = false;
	} else if (minTime < FLT_MAX){
		nearestCar = o[collcarindex].collcar;
		sidechangeallowed = true;
		minorthdist = orthdist;
		int i;
// TODO: MAGIG NUMBER.
		for (i = 0; i <= (int) myc->MINOVERTAKERANGE; i += 10) {
			if (track->getSegmentPtr((trackSegId+((int) (i/TRACKRES))) % nPathSeg)->getRadius() < myc->OVERTAKERADIUS) return 0;
		}

		// Reject overtaking teammate.
		if (teammate != NULL &&
			nearestCar->getCarPtr() == teammate &&
			(nearestCar->getCarPtr()->_dammage - myc->getCarPtr()->_dammage) < myc->TEAM_DAMAGE_CHANGE_LEAD)
		{
			return 0;
		}

	} else return 0;

	/* not enough space, so we try to overtake */
	if (((o[collcarindex].mincorner < myc->CARWIDTH/2.0 + myc->DIST) && (minTime < myc->TIMETOCATCH)) || !sidechangeallowed) {
		int overtakerange = (int) MIN(MAX((3.0*(3.0/myc->TIMETOCATCH*minTime)*myc->getSpeed()), myc->MINOVERTAKERANGE)/TRACKRES, AHEAD );
		double d = o[collcarindex].disttomiddle;
		double mydisttomiddle = track->distToMiddle(myc->getCurrentSegId(), myc->getCurrentPos());
		double y[3], ys[3], s[3];


		y[0] = track->distToMiddle(trackSegId, myc->getCurrentPos());
		double alpha = PI/2.0 - acos((*myc->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));

		int trackSegId1;
		if (minTime < myc->TIMETOCATCH && o[collcarindex].catchdist > 1.0) {
			trackSegId1 = (trackSegId + MIN((int) (MAX((o[collcarindex].catchdist), myc->getSpeed()/4.0)/TRACKRES), overtakerange/3)) % nPathSeg;
		} else {
			trackSegId1 = (trackSegId + overtakerange/3) % nPathSeg;
		}

		double w = track->getSegmentPtr(nearestCar->getCurrentSegId())->getWidth() / 2;
		double pathtomiddle = track->distToMiddle(trackSegId1, psdyn->getLoc(trackSegId1));
		double paralleldist = o[collcarindex].cosalpha*dist(myc->getCurrentPos(), nearestCar->getCurrentPos());

		if (!sidechangeallowed) {
			if (paralleldist > 1.5*myc->CARLEN) {
				int i;
				for (i = 0; i <= (int) myc->MINOVERTAKERANGE; i += 10) {
					if (track->getSegmentPtr((trackSegId+((int) (i/TRACKRES))) % nPathSeg)->getRadius() < myc->OVERTAKERADIUS) return 0;
				}
				vec2d dir = *o[collcarindex].collcar->getCurrentPos()- *myc->getCurrentPos();
				double pathtocarsgn = sign(myc->getDir()->fakeCrossProduct(&dir));

				y[1] = d + myc->OVERTAKEDIST*pathtocarsgn;
				if (fabs(y[1]) > w - (1.0*myc->CARWIDTH)) {
					y[1] = d - myc->OVERTAKEDIST*pathtocarsgn;
				}


				double beta = PI/2.0 - acos((*nearestCar->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));
				//double beta = PI/2.0 - acos((*nearestCar->getDir())*(*psopt->getOptToRight(trackSegId)));

				if (y[1] - mydisttomiddle >= 0.0) {
					if (alpha < beta + myc->OVERTAKEANGLE) alpha = alpha + myc->OVERTAKEANGLE;
				} else {
					if (alpha > beta - myc->OVERTAKEANGLE) alpha = alpha - myc->OVERTAKEANGLE;
				}
			} else {
				double beta = PI/2.0 - acos((*nearestCar->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));
				//double beta = PI/2.0 - acos((*nearestCar->getDir())*(*psopt->getOptToRight(trackSegId)));

				double delta = mydisttomiddle - d;
				if (delta >= 0.0) {
					if (alpha < beta + myc->OVERTAKEANGLE) alpha = beta + myc->OVERTAKEANGLE;
				} else {
					if (alpha > beta - myc->OVERTAKEANGLE) alpha = beta - myc->OVERTAKEANGLE;
				}
				double cartocarsgn = sign(delta);
				y[1] = d + myc->OVERTAKEDIST*cartocarsgn;
				if (fabs(y[1]) > w - (1.0*myc->CARWIDTH)) {
					y[1] = cartocarsgn*(w - (myc->CARWIDTH/2.0 + myc->MARGIN));
				}

				if (minorthdist > 1.0 /*myc->OVERTAKEMINDIST*/) o[collcarindex].overtakee = true;
				//o[collcarindex].overtakee = true;
			}
		} else {
			// Case when we can change the lane without crashing into the opponent.
			double pathtocarsgn = sign(pathtomiddle - d);
			if (collcars == 1) {
				y[1] = d + -sign(d)*(1.5+o[collcarindex].width/2.0 + myc->getCarPtr()->_dimension_y);
			} else if (freefrommiddle > -100.0) {
				y[1] = freefrommiddle;
			} else {
				y[1] = d + (myc->OVERTAKEDIST+o[collcarindex].width/2.0)*pathtocarsgn;
			}
			if (pathtocarsgn < 0.0 && y[1] > pathtomiddle) return 0;
			if (pathtocarsgn > 0.0 && y[1] <pathtomiddle) return 0;

			if (fabs(y[1]) > w - (0.5*myc->CARWIDTH+2.0*myc->MARGIN)) {
				y[1] = d - myc->OVERTAKEDIST*pathtocarsgn;
			}
		}

		double ww = w - (myc->CARWIDTH + myc->MARGIN);
		if ((y[1] >  ww && alpha > 0.0) || (y[1] < -ww && alpha < 0.0)) {
			alpha = 0.0;
		}

		y[1] = y[1] - pathtomiddle;

		ys[0] = tan(alpha);
		ys[1] = 0.0;

		/* set up point 2 */
		int trackSegId2 = (trackSegId + overtakerange) % nPathSeg;
		y[2] = track->distToMiddle(trackSegId2, psopt->getOptLoc(trackSegId2));
		ys[2] = pathOptSlope(trackSegId2);

		// set up parameter s
		s[0] = s[1] = s[2] = 0.0;
		//s[0] = 0.0;
		for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId1; i++) {
			s[1] += dist(track->getSegmentPtr(j)->getMiddle(), track->getSegmentPtr((j+1)%nPathSeg)->getMiddle());//track->getSegmentPtr3D(j)->getLength();
		}
		for (i = trackSegId1; (j = (i + nPathSeg) % nPathSeg) != trackSegId2; i++) {
			s[2] += dist(track->getSegmentPtr(j)->getMiddle(), track->getSegmentPtr((j+1)%nPathSeg)->getMiddle());//track->getSegmentPtr3D(j)->getLength();
		}
		s[2] += s[1];

		// check path for leaving to track
		double newdisttomiddle[AHEAD];
		int i, j;
		double l = 0.0;
		vec2d q;
		const vec2d *pp, *qq;
		for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId2; i++) {
			d = spline(3, l, s, y, ys);
			float bordermargin = (track->getSegmentPtr(j)->getRadius() > 200.0) ? 0.0 : myc->MARGIN;
			if (fabs(d) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - bordermargin) {
				o[collcarindex].overtakee = false;
				return 0;
			}
			newdisttomiddle[i - trackSegId] = d;
			l += dist(track->getSegmentPtr(j)->getMiddle(), track->getSegmentPtr((j+1)%nPathSeg)->getMiddle());
		}

		// set up the path
		for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId2; i++) {
			pp = track->getSegmentPtr(j)->getMiddle();
			qq = track->getSegmentPtr(j)->getToRight();
			q = *pp + (*qq)*newdisttomiddle[i - trackSegId];
			psdyn->setLoc(&q, j);
		}

		// reload old trajectory where needed
		for (i = trackSegId2; (j = (i+nPathSeg) % nPathSeg) != (trackSegId+AHEAD) % nPathSeg; i ++) {
			psdyn->setLoc(psopt->getOptLoc(j), j);
		}

		// align previos point for getting correct speedsqr in Pathfinder::plan(...).
		int p = (trackSegId - 1 + nPathSeg) % nPathSeg;
		int e = (trackSegId + 1 + nPathSeg) % nPathSeg;
		smooth(trackSegId, p, e, 1.0);

		return 1;
	} else {
		return 0;
	}
}


/* collect data about other cars relative to me */
inline int Pathfinder::updateOCar(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o)
{
	const int start = (trackSegId - (int) ((1.0 + myc->CARLEN/2.0)/TRACKRES) + nPathSeg) % nPathSeg;
	const int end = (trackSegId + (int) (COLLDIST/TRACKRES) + nPathSeg) % nPathSeg;

	int i, n = 0;		/* counter for relevant cars */

	for (i = 0; i < s->_ncars; i++) {
		tCarElt* car = ocar[i].getCarPtr();
		/* is it me ? */
		if (car != myc->getCarPtr()) {
			int seg = ocar[i].getCurrentSegId();
			/* get the next car to catch up */
			if (track->isBetween(start, end, seg) && !(car->_state & (RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN | RM_CAR_STATE_NO_SIMU))) {
				o[n].cosalpha = (*myc->getDir())*(*ocar[i].getDir());
				o[n].speed = ocar[i].getSpeed()*o[n].cosalpha;
				int j, k = track->diffSegId(trackSegId, seg);

				// Compute distance along the path to the center of gravity of the opponent.
				if ( k < 40.0/TRACKRES) {
					o[n].dist = 0.0;
					int l = MIN(trackSegId, seg);
					for (j = l; j < l + k; j++) {
						o[n].dist += psdyn->getLength(j % nPathSeg);
					}
				} else {
					o[n].dist = k*TRACKRES;
				}
				o[n].collcar = &ocar[i];
				o[n].time = o[n].dist/(myc->getSpeed() - o[n].speed);
				if (o[n].time < 0.0) {
					o[n].time = FLT_MAX;
				}

				o[n].disttomiddle = track->distToMiddle(seg, ocar[i].getCurrentPos());
				o[n].speedsqr = sqr(o[n].speed);
				o[n].catchdist = (int) (o[n].dist/(MIN(myc->getSpeed(), sqrt(psdyn->getSpeedsqr(seg))) - ocar[i].getSpeed())*MIN(myc->getSpeed(), sqrt(psdyn->getSpeedsqr(seg))));//myc->getSpeed());
				o[n].catchsegid = ((int) (o[n].catchdist/TRACKRES) + trackSegId + nPathSeg) % nPathSeg;
				o[n].overtakee = false;
				o[n].disttopath = distToPath(seg, ocar[i].getCurrentPos());
				double gm = track->getSegmentPtr(seg)->getKfriction()*myc->CFRICTION;
				double qs = o[n].speedsqr;
				o[n].brakedist = (myc->getSpeedSqr() - o[n].speedsqr)*(myc->mass/(2.0*gm*g*myc->mass + (qs)*(gm*myc->ca)));
				o[n].mincorner = FLT_MAX;
				o[n].minorthdist = FLT_MAX;
				for (j = 0; j < 4; j++) {
					vec2d e(car->pub.corner[j].ax, car->pub.corner[j].ay);
					double corner = fabs(distToPath(seg, &e));
					double orthdist = track->distGFromPoint(myc->getCurrentPos(), myc->getDir(), &e) - myc->CARWIDTH/2.0;
					if (corner < o[n].mincorner) o[n].mincorner = corner;
					if (orthdist < o[n].minorthdist) o[n].minorthdist = orthdist;
				}

				// Compute with of the car along the track.
				const vec2d* tr = track->getSegmentPtr(seg)->getToRight();
				vec2d trackdir(-tr->y, tr->x);
				float carcosa = trackdir*(*ocar[i].getDir());
				o[n].width = car->_dimension_x*sin(acos(carcosa)) + car->_dimension_y*carcosa;
				n++;
			}
		}
	}
	return n;
}


inline void Pathfinder::updateOverlapTimer(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o, tOverlapTimer* ov)
{
	const int start = (trackSegId - (int) (myc->OVERLAPSTARTDIST/TRACKRES) + nPathSeg) % nPathSeg;
	const int end = (trackSegId - (int) ((2.0 + myc->CARLEN/2.0)/TRACKRES) + nPathSeg) % nPathSeg;
	const int startfront = (trackSegId + (int) ((2.0 + myc->CARLEN/2.0)/TRACKRES)) % nPathSeg;
	const int endfront = (trackSegId + (int) (myc->OVERLAPSTARTDIST/TRACKRES)) % nPathSeg;

	int i;

	for (i = 0; i < s->_ncars; i++) {
		tCarElt* car = ocar[i].getCarPtr();
		tCarElt* me = myc->getCarPtr();
		/* is it me, and in case not, has the opponent more laps than me? */
		if ((car != me) && (car->race.laps > me->race.laps) &&
		!(car->_state & (RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN | RM_CAR_STATE_NO_SIMU))) {
			int seg = ocar[i].getCurrentSegId();
			if (track->isBetween(start, end, seg)) {
				ov[i].time += s->deltaTime;
			} else if (track->isBetween(startfront, endfront, seg)) {
				ov[i].time = myc->LAPBACKTIMEPENALTY;
			} else {
				if (ov[i].time > 0.0) ov[i].time -= s->deltaTime;
				else ov[i].time += s->deltaTime;
			}
		} else {
			ov[i].time = 0.0;
		}
	}
}


/* compute trajectory to let opponent overlap */
int Pathfinder::letoverlap(int trackSegId, tSituation *situation, MyCar* myc, OtherCar* ocar, tOverlapTimer* ov)
{
	const int start = (trackSegId - (int) (myc->OVERLAPPASSDIST/TRACKRES) + nPathSeg) % nPathSeg;
	const int end = (trackSegId - (int) ((2.0 + myc->CARLEN/2.0)/TRACKRES) + nPathSeg) % nPathSeg;
	int k;

	for (k = 0; k < situation->_ncars; k++) {

		if ((ov[k].time > myc->OVERLAPWAITTIME) && track->isBetween(start, end, ocar[k].getCurrentSegId())) {
			/* let overtake */
			double s[4], y[4], ys[4];
// TODO: constant.
			const int DST = 400;

// TODO: optslope correct or dynslope?
			ys[0] = pathDynSlope(trackSegId);
			if (fabs(ys[0]) > PI/180.0) return 0;

			int trackSegId1 = (trackSegId + (int) (DST/(4.0*TRACKRES)) + nPathSeg) % nPathSeg;
			int trackSegId2 = (trackSegId + (int) (DST*3.0/(4.0*TRACKRES)) + nPathSeg) % nPathSeg;
			int trackSegId3 = (trackSegId + (int) (DST/TRACKRES) + nPathSeg) % nPathSeg;
			double width = track->getSegmentPtr(trackSegId1)->getWidth();

			/* point 0 */
			y[0] = track->distToMiddle(trackSegId, myc->getCurrentPos());

			/* point 1 */
// TODO: REMOVE magic numbers.
			y[1] = sign(y[0])*MIN((width/2.0 - 2.0*myc->CARWIDTH - myc->MARGIN), (15.0/2.0));
			ys[1] = 0.0;

			/* point 2 */
			y[2] = y[1];
			ys[2] = 0.0;

			/* point 3*/
			y[3] = track->distToMiddle(trackSegId3, psopt->getOptLoc(trackSegId3));
// TODO: optslope correct or dynslope?
			ys[3] = pathOptSlope(trackSegId3);

			/* set up parameter s */
			s[0] = 0.0;
			s[1] = countSegments(trackSegId, trackSegId1)*TRACKRES;
			s[2] = s[1] + countSegments(trackSegId1, trackSegId2)*TRACKRES;
			s[3] = s[2] + countSegments(trackSegId2, trackSegId3)*TRACKRES;

			/* check path for leaving to track */
			double newdisttomiddle[AHEAD], d;
			int i, j;
			double l = 0.0;
			vec2d q;
			const vec2d *pp, *qq;
			for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId3; i++) {
				d = spline(4, l, s, y, ys);
				if (fabs(d) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - myc->MARGIN) {
					return 0;
				}
				newdisttomiddle[i - trackSegId] = d;
				l += TRACKRES;
			}

			/* set up the path */
			for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId3; i++) {
				pp = track->getSegmentPtr(j)->getMiddle();
				qq = track->getSegmentPtr(j)->getToRight();
				q = *pp + (*qq)*newdisttomiddle[i - trackSegId];
				psdyn->setLoc(&q, j);
			}

			/* reload old trajectory where needed */
			for (i = trackSegId3; (j = (i+nPathSeg) % nPathSeg) != (trackSegId+AHEAD) % nPathSeg; i ++) {
				psdyn->setLoc(psopt->getOptLoc(j), j);
			}

			/* reset all timer to max 3.0 */
			for (j = 0; j < situation->_ncars; j++) {
				ov[j].time = MIN(ov[j].time, 3.0);
			}
			return 1;
		}
	}
	return 0;
}
