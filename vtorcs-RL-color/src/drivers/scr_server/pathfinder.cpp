/***************************************************************************

    file                 : pathfinder.cpp
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2002 by Bernhard Wymann, some Code from Remi Coulom, K1999.cpp
    email                : berniw@bluewin.ch
    version              : $Id: pathfinder.cpp,v 1.55 2005/02/01 18:33:32 berniw Exp $

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

const double Pathfinder::COLLDIST = 200.0;
const double Pathfinder::TPRES = PI/(NTPARAMS - 1);	/* resolution of the steps */


Pathfinder::Pathfinder(TrackDesc* itrack, tCarElt* car, tSituation *s)
{
	int i;
	track = itrack;
	tTrack* t = track->getTorcsTrack();
	o = new tOCar[s->_ncars];
	overlaptimer = new tOverlapTimer[s->_ncars];

	for (i = 0; i < s->_ncars; i++) overlaptimer[i].time = 0.0;

	/* the path has to have one point per tracksegment */
	nPathSeg = track->getnTrackSegments();

	/* get memory for trajectory */
	ps = new PathSeg[nPathSeg];
	changed = lastPlan = lastPlanRange = 0;
	inPit = pitStop = false;

	/* check if there is a pit type we can use and if for this car is a pit available */
	pit = false;
	if (t->pits.type == TR_PIT_ON_TRACK_SIDE && car->index < t->pits.nMaxPits) {
		pit = true;
	}

	s1 = e3 = 0;
	if (isPitAvailable()) {
		initPit(car);
		s1 = track->getPitEntryStartId();
		s1 = (int) GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_PITENTRY, (char*)NULL, (float) s1);
		e3 = track->getPitExitEndId();
		e3 = (int) GfParmGetNum(car->_carHandle, BERNIW_SECT_PRIV, BERNIW_ATT_PITEXIT, (char*)NULL, (float) e3);
		pitspeedsqrlimit = t->pits.speedLimit - 0.5;
		pitspeedsqrlimit *= pitspeedsqrlimit;
		/* get memory for the pit points */
		pitcord = new v3d[countSegments(s1, e3)];
	}
}


Pathfinder::~Pathfinder()
{
	delete [] ps;
	if (isPitAvailable()) delete [] pitcord;
	delete [] o;
	delete [] overlaptimer;
}


/* compute where the pit is, etc */
void Pathfinder::initPit(tCarElt* car) {
	tTrack* t = track->getTorcsTrack();

	if (t->pits.driversPits != NULL && car != NULL) {
		if (isPitAvailable()) {
			tTrackSeg* pitSeg = t->pits.driversPits->pos.seg;
			if (pitSeg->type == TR_STR) {
				v3d v1, v2;
				/* v1 points in the direction of the segment */
				v1.x = pitSeg->vertex[TR_EL].x - pitSeg->vertex[TR_SL].x;
				v1.y = pitSeg->vertex[TR_EL].y - pitSeg->vertex[TR_SL].y;
				v1.z = pitSeg->vertex[TR_EL].z - pitSeg->vertex[TR_SL].z;
				v1.normalize();

				/* v2 points to the side of the segment */
				double s = (t->pits.side == TR_LFT) ? -1.0 : 1.0 ;
				v2.x = s*(pitSeg->vertex[TR_SR].x - pitSeg->vertex[TR_SL].x);
				v2.y = s*(pitSeg->vertex[TR_SR].y - pitSeg->vertex[TR_SL].y);
				v2.z = s*(pitSeg->vertex[TR_SR].z - pitSeg->vertex[TR_SL].z);
				v2.normalize();

				/* loading starting point of segment */
				pitLoc.x = (pitSeg->vertex[TR_SR].x + pitSeg->vertex[TR_SL].x) / 2.0;
				pitLoc.y = (pitSeg->vertex[TR_SR].y + pitSeg->vertex[TR_SL].y) / 2.0;
				pitLoc.z = (pitSeg->vertex[TR_SR].z + pitSeg->vertex[TR_SL].z) / 2.0;

				/* going along the track */
				double l = t->pits.len*car->index;
				pitLoc = pitLoc + (t->pits.driversPits->pos.toStart + l)*v1;

				/* going sideways, minus because of opposite sign of v2 and the value toMiddle */
				double m = fabs(t->pits.driversPits->pos.toMiddle);
				pitLoc = pitLoc + m*v2;

				pitSegId = track->getNearestId(&pitLoc);

				l = t->pits.len*(car->index + 2);
				v2 = pitLoc - l*v1;
				s3 = track->getNearestId(&v2);

				l = t->pits.len*(t->pits.nMaxPits + 1 + 2);
				v2 = v2 + l*v1;
				e1 = track->getNearestId(&v2);
			} else pit = false;
		}
	} else {
		printf("error: pit struct ptr == NULL. call this NOT in inittrack, call it in newrace.\n");
	}
}


/* call this after you computed a static racing path with plan() */
void Pathfinder::initPitStopPath(void)
{
	tTrack* t = track->getTorcsTrack();
	v3d p, q, *pp, *qq, *pmypitseg = track->getSegmentPtr(pitSegId)->getMiddle();
	double d, dp, sgn;
	double delta = t->pits.width;
	int i;
	double ypit[PITPOINTS], yspit[PITPOINTS], spit[PITPOINTS];
	int snpit[PITPOINTS];

	/* set up point 0 on the track (s1) */
	ypit[0] = track->distToMiddle(s1, ps[s1].getLoc());
	snpit[0] = s1;

	/* set up point 1 pit lane entry (s3) */
	track->dirVector2D(&pitLoc, pmypitseg, &p);
	dp = p.len();
	d = dp - delta;

	sgn = (t->pits.side == TR_LFT) ? -1.0 : 1.0 ;
	ypit[1] = d*sgn;
	snpit[1] = s3;

	/* set up point 2 before we turn into the pit */
	i = (pitSegId - (int) t->pits.len + nPathSeg) % nPathSeg;
	ypit[2] = d*sgn;
	snpit[2] = i;

	/* set up point 3, the pit, we know this already */
	ypit[3] = dp*sgn;
	snpit[3] = pitSegId;

	/* compute point 4, go from pit back to pit lane */
	i = (pitSegId + (int) t->pits.len + nPathSeg) % nPathSeg;
	ypit[4] = d*sgn;
	snpit[4] = i;

	/* compute point 5, drive to end of pit lane (e1) */
	ypit[5] = d*sgn;
	snpit[5] = e1;

	/* compute point 6, back on the track */
	ypit[6] = track->distToMiddle(e3, ps[e3].getLoc());
	snpit[6] = e3;

	/* compute spit array */
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

	/* set up slopes */
	yspit[0] = pathSlope(s1);
	yspit[6] = pathSlope(e3);

	for (i = 1; i < PITPOINTS-1; i++) {
		yspit[i] = 0.0;
	}

	/* compute path to pit */
	double l = 0.0;
	for (i = s1; (i + nPathSeg) % nPathSeg != e3; i++) {
		int j = (i + nPathSeg) % nPathSeg;
		d = spline(PITPOINTS, l, spit, ypit, yspit);

		pp = track->getSegmentPtr(j)->getMiddle();
		qq = track->getSegmentPtr(j)->getToRight();

		p.x = qq->x; p.y = qq->y; p.z = 0.0;
		p.normalize();

		q.x = pp->x + p.x*d;
		q.y = pp->y + p.y*d;
		q.z = (t->pits.side == TR_LFT) ? track->getSegmentPtr(j)->getLeftBorder()->z: track->getSegmentPtr(j)->getRightBorder()->z;

		pitcord[i-s1] = q;
		ps[j].setPitLoc(&pitcord[i-s1]);
		l += TRACKRES;
	}
}


/* plots pit trajectory to file for gnuplot */
void Pathfinder::plotPitStopPath(char* filename)
{
	FILE* fd = fopen(filename, "w");

	/* plot pit path */
	for (int i = 0; i < nPathSeg; i++) {
		fprintf(fd, "%f\t%f\n", ps[i].getPitLoc()->x, ps[i].getPitLoc()->y);
	}
	fclose(fd);
}


void Pathfinder::plotPath(char* filename)
{
	FILE* fd = fopen(filename, "w");

	/* plot path */
	for (int i = 0; i < nPathSeg; i++) {
		fprintf(fd, "%f\t%f\n", ps[i].getLoc()->x, ps[i].getLoc()->y);
	}
	fclose(fd);
}

#ifdef PATH_BERNIW

/* load parameters for clothoid from the files */
bool Pathfinder::loadClothoidParams(tParam* p)
{
	double dummy;
	FILE* fd = fopen(FNPF, "r");

	/* read FNPF */
	if (fd != NULL) {
		for (int i = 0; i < NTPARAMS; i++) {
			fscanf(fd, "%lf %lf", &p[i].x, &p[i].pd);
		}
	} else {
		printf("error in loadClothoidParams(tParam* p): couldn't open file %s.\n", FNPF);
		return false;
	}
	fclose(fd);

	/* read FNIS */
	fd = fopen(FNIS, "r");
	if (fd != NULL) {
		for (int i = 0; i < NTPARAMS; i++) {
			fscanf(fd, "%lf %lf", &dummy, &p[i].is);
		}
	} else {
		printf("error in loadClothoidParams(tParam* p): couldn't open file %s.\n", FNIS);
		return false;
	}
	fclose(fd);

	/* read FNIC */
	fd = fopen(FNIC, "r");
	if (fd != NULL) {
		for (int i = 0; i < NTPARAMS; i++) {
			fscanf(fd, "%lf %lf", &dummy, &p[i].ic);
		}
	} else {
		printf("error in loadClothoidParams(tParam* p): couldn't open file %s.\n", FNIC);
		return false;
	}
	fclose(fd);

	return true;
}


/*
	computes int(sin(u^2), u=0..alpha), where alpha is [0..PI).
*/
double Pathfinder::intsinsqr(double alpha)
{
	int i = (int) floor(alpha/TPRES), j = i + 1;
	/* linear interpoation between the nearest known two points */
	return cp[i].is + (alpha - cp[i].x)*(cp[j].is - cp[i].is)/TPRES;
}


/*
	computes int(cos(u^2), u=0..alpha), where alpha is [0..PI).
*/
double Pathfinder::intcossqr(double alpha)
{
	int i = (int) floor(alpha/TPRES), j = i + 1;
	/* linear interpoation between the nearest known two points */
	return cp[i].ic + (alpha - cp[i].x)*(cp[j].ic - cp[i].ic)/TPRES;
}


/*
	computes clothoid parameter pd(look with maple at clothoid.mws), where alpha is [0..PI).
*/
double Pathfinder::clothparam(double alpha)
{
	int i = (int) floor(alpha/TPRES), j = i + 1;
	/* linear interpoation between the nearest known two points */
	return cp[i].pd + (alpha - cp[i].x)*(cp[j].pd - cp[i].pd)/TPRES;
}


/*
	computes clothoid parameter sigma (look with maple at clothoid.mws), where beta is [0..PI) and y > 0.0.
*/
double Pathfinder::clothsigma(double beta, double y)
{
		double a = intsinsqr(sqrt(fabs(beta)))/y;
		return a*a*2.0;
}


/*
	computes the langth of the clothoid(look with maple at clothoid.mws), where beta is [0..PI) and y > 0.0.
*/
double Pathfinder::clothlength(double beta, double y)
{
	return 2.0*sqrt(2.0*beta/clothsigma(beta, y));
}


/*
	searches for the startid of a part, eg. TR_STR
*/

int Pathfinder::findStartSegId(int id)
{

	double radius = track->getSegmentPtr(id)->getRadius();
	int type = track->getSegmentPtr(id)->getType();
	int i = (id - 1 + nPathSeg) % nPathSeg, j = id;

	while (track->getSegmentPtr(i)->getType() == type &&
	       track->getSegmentPtr(i)->getRadius() == radius &&
		   i != id) {
		j = i;
		i = (i - 1 + nPathSeg) % nPathSeg;
	}
	return j;
}


/*
	searches for the endid of a part, eg. TR_STR
*/
int Pathfinder::findEndSegId(int id)
{
	double radius = track->getSegmentPtr(id)->getRadius();
	int type = track->getSegmentPtr(id)->getType();
	int i = (id + 1 + nPathSeg) % nPathSeg, j = id;

	while (track->getSegmentPtr(i)->getType() == type &&
	       track->getSegmentPtr(i)->getRadius() == radius &&
		   i != id) {
		j = i;
		i = (i + 1 + nPathSeg) % nPathSeg;
	}
	return j;
}


/*
	weight function, x 0..1
*/
double Pathfinder::computeWeight(double x, double len)
{
	return (x <= 0.5) ? (2.0*x)*len : (2.0*(1.0-x))*len;
}


/*
	modify point according to the weights
*/
void Pathfinder::setLocWeighted(int id, double newweight, v3d* newp)
{
	double oldweight = ps[id].getWeight();
	v3d* oldp = ps[id].getLoc();
	v3d p;

	/* ugly, fix it in init.... */
	if (newweight < 0.001) newweight = 0.001;

	if (oldweight + newweight == 0.0) printf("ops! O: %f, N: %f\n", oldweight, newweight);
	if (oldweight > newweight) {
		double d = newweight/(oldweight+newweight);
		p = (*oldp) + (*newp - *oldp)*d;
		ps[id].setLoc(&p);
		ps[id].setWeight(oldweight+newweight);
	} else {
		double d = oldweight/(oldweight+newweight);
		p = (*newp) + (*oldp - *newp)*d;
		ps[id].setLoc(&p);
		ps[id].setWeight(oldweight+newweight);
	}
}

/*
	initializes the path for straight parts of the track.
*/
int Pathfinder::initStraight(int id, double w)
{
	int start = findStartSegId(id), end = findEndSegId(id);
	int prev = (start - 1 + nPathSeg) % nPathSeg, next = (end + 1) % nPathSeg;
	int prevtype = track->getSegmentPtr(prev)->getType();
	int nexttype = track->getSegmentPtr(next)->getType();
	int len = track->diffSegId(start, end);

	if (prevtype == nexttype) {
		if (prevtype == TR_RGT) {
			int l = 0;
			for (int i = start; i != next; i++) {
				i = i % nPathSeg;
				if (ps[i].getWeight() == 0.0) {
					v3d* p = track->getSegmentPtr(i)->getLeftBorder();
					v3d* r = track->getSegmentPtr(i)->getToRight();
					v3d np;
					np = (*p) + (*r)*w;
					setLocWeighted(i, computeWeight(((double) l) / ((double) len), len), &np);
					l++;
				}
			}
		} else {
			int l = 0;
			for (int i = start; i != next; i++) {
				i = i % nPathSeg;
				if (ps[i].getWeight() == 0.0) {
					v3d* p = track->getSegmentPtr(i)->getRightBorder();
					v3d* r = track->getSegmentPtr(i)->getToRight();
					v3d np;
					np = (*p) - (*r)*w;
					setLocWeighted(i, computeWeight(((double) l) / ((double) len), len), &np);
					l++;
				}
			}
		}
	} else {
		double startwidth = track->getSegmentPtr(start)->getWidth()/2.0 - w;
		double endwidth = track->getSegmentPtr(end)->getWidth()/2.0 - w;
		double dw = (startwidth + endwidth) / len;
		int l = 0;
		if (prevtype == TR_RGT) {
			for (int i = start; i != next; i++) {
				i = i % nPathSeg;
				v3d* p = track->getSegmentPtr(i)->getLeftBorder();
				v3d* r = track->getSegmentPtr(i)->getToRight();
				v3d np;
				np = (*p) + (*r)*(w + dw*l);
				setLocWeighted(i, computeWeight(((double) l) / ((double) len), len), &np);
				l++;
			}
		} else {
			for (int i = start; i != next; i++) {
				i = i % nPathSeg;
				v3d* p = track->getSegmentPtr(i)->getRightBorder();
				v3d* r = track->getSegmentPtr(i)->getToRight();
				v3d np;
				np = (*p) - (*r)*(w + dw*l);
				setLocWeighted(i, computeWeight(((double) l) / ((double) len), len), &np);
				l++;
			}
		}
	}
	return next;
}


/*
	initializes the path for left turns.
*/
int Pathfinder::initLeft(int id, double w)
{
	int start = findStartSegId(id), end = findEndSegId(id);
	int prev = (start - 1 + nPathSeg) % nPathSeg, next = (end + 1) % nPathSeg;
	int len = track->diffSegId(start, end);
	int tseg = (start + (len)/2) % nPathSeg;
	v3d* s1 = track->getSegmentPtr(start)->getRightBorder();
	v3d* s2 = track->getSegmentPtr(prev)->getRightBorder();
	v3d* tr = track->getSegmentPtr(prev)->getToRight();
	v3d* tg = track->getSegmentPtr(tseg)->getLeftBorder();
	v3d* trtg = track->getSegmentPtr(tseg)->getToRight();
	v3d sdir, sp, t;

	double beta = acos(track->cosalpha(trtg, tr));

	if (beta < 0.0) printf("error in initLeft: turn > 360� ??\n");

	s1->dirVector(s2, &sdir);
	sp = (*s2) - (*tr)*w;
	t = (*tg) + (*trtg)*w;

	double yd = track->distGFromPoint(&sp, &sdir, &t);
	int tlen = (int) ceil(clothlength(beta, yd));

	if (tlen < 0) printf("error in initLeft: tlen < 0 ??\n");

	int startsp = (tseg - tlen/2 + nPathSeg) % nPathSeg;
	int endsp = (startsp + tlen) % nPathSeg;

	double s[3], y[3], ys[3];

	ys[0] = ys[1] = ys[2] = 0.0;
	s[0] = 0;
	s[1] = tlen/2;
	s[2] = tlen;
	y[0] = track->getSegmentPtr(startsp)->getWidth()/2.0 - w;
	y[1] = -(track->getSegmentPtr(tseg)->getWidth()/2.0 - w);
	y[2] = track->getSegmentPtr(endsp)->getWidth()/2.0 - w;

	double l = 0.0;
	v3d q, *pp, *qq;
	for (int i = startsp; (i + nPathSeg) % nPathSeg != endsp; i++) {
		int j = (i + nPathSeg) % nPathSeg;
		double d = spline(3, l, s, y, ys);

		pp = track->getSegmentPtr(j)->getMiddle();
		qq = track->getSegmentPtr(j)->getToRight();
		q = (*pp) + (*qq)*d;
		setLocWeighted(j, computeWeight(((double) l) / ((double) tlen), tlen), &q);

		l += TRACKRES;
	}

	return next;
}


/*
	initializes the path for right turns.
*/
int Pathfinder::initRight(int id, double w)
{
	int start = findStartSegId(id), end = findEndSegId(id);
	int prev = (start - 1 + nPathSeg) % nPathSeg, next = (end + 1) % nPathSeg;
	int len = track->diffSegId(start, end);
	int tseg = (start + (len)/2) % nPathSeg;
	v3d* s1 = track->getSegmentPtr(start)->getLeftBorder();
	v3d* s2 = track->getSegmentPtr(prev)->getLeftBorder();
	v3d* tr = track->getSegmentPtr(prev)->getToRight();
	v3d* tg = track->getSegmentPtr(tseg)->getRightBorder();
	v3d* trtg = track->getSegmentPtr(tseg)->getToRight();
	v3d sdir, sp, t;

	double beta = acos(track->cosalpha(trtg, tr));

	if (beta < 0.0) printf("error in initRight: turn > 360� ??\n");

	s1->dirVector(s2, &sdir);
	sp = (*s2) + (*tr)*w;
	t = (*tg) - (*trtg)*w;

	double yd = track->distGFromPoint(&sp, &sdir, &t);
	int tlen = (int) ceil(clothlength(beta, yd));

	if (tlen < 0) printf("error in initRight: tlen < 0 ??\n");

	int startsp = (tseg - tlen/2 + nPathSeg) % nPathSeg;
	int endsp = (startsp + tlen) % nPathSeg;

	double s[3], y[3], ys[3];

	ys[0] = ys[1] = ys[2] = 0.0;
	s[0] = 0;
	s[1] = tlen/2;
	s[2] = tlen;
	y[0] = -(track->getSegmentPtr(startsp)->getWidth()/2.0 - w);
	y[1] = track->getSegmentPtr(tseg)->getWidth()/2.0 - w;
	y[2] = -(track->getSegmentPtr(endsp)->getWidth()/2.0 - w);

	double l = 0.0;
	v3d q, *pp, *qq;
	for (int i = startsp; (i + nPathSeg) % nPathSeg != endsp; i++) {
		int j = (i + nPathSeg) % nPathSeg;
		double d = spline(3, l, s, y, ys);

		pp = track->getSegmentPtr(j)->getMiddle();
		qq = track->getSegmentPtr(j)->getToRight();
		q = (*pp) + (*qq)*d;
		setLocWeighted(j, computeWeight(((double) l) / ((double) tlen), tlen), &q);

		l += TRACKRES;
	}

	return next;
}

#endif // PATH_BERNIW

/*
	plans a static route ignoring current situation
*/
void Pathfinder::plan(MyCar* myc)
{
	double r, length, speedsqr;
	int u, v, w;
	v3d dir;
	int i;

	/* basic initialisation */
	for (i = 0; i < nPathSeg; i++) {
		ps[i].setLoc(track->getSegmentPtr(i)->getMiddle());
		ps[i].setWeight(0.0);
	}

#ifdef PATH_BERNIW
	/* read parameter files and compute path */
	if (loadClothoidParams(cp)) {
		int i = 0, k = 0;
		while (k < nPathSeg) {
			int j = k % nPathSeg;
			switch (track->getSegmentPtr(j)->getType()) {
			case TR_STR:
				i = initStraight(j, myc->CARWIDTH/2.0+myc->MARGIN);
				break;
			case TR_LFT:
				i = initLeft(j, myc->CARWIDTH/2.0+myc->MARGIN);
				break;
			case TR_RGT:
				i = initRight(j, myc->CARWIDTH/2.0+myc->MARGIN);
				break;
			default:
				printf("error in plan(MyCar* myc): segment is of unknown type.\n");
				break;
			}
			k = k + (i - k + nPathSeg) % nPathSeg;
		}
	}

	optimize3(0, nPathSeg, 1.0);
	optimize3(2, nPathSeg, 1.0);
	optimize3(1, nPathSeg, 1.0);

	optimize2(0, 10*nPathSeg, 0.5);
	optimize(0, 80*nPathSeg, 1.0);

	for (int k = 0; k < 10; k++) {
		const int step = 65536*64;
		for (int j = 0; j < nPathSeg; j++) {
			for (int i = step; i > 0; i /=2) {
				smooth(j, (double) i / (step / 2), myc->CARWIDTH/2.0);
			}
		}
	}
#endif	// PATH_BERNIW

#ifdef PATH_K1999
	/* compute path */
	for (int step = 128; (step /= 2) > 0;) {
		for (int i = 100 * int(sqrt(double(step))); --i >= 0;) smooth(step);
		interpolate(step);
	}
#endif	// PATH_K1999

	/* init pit ond optimal path */
	for (i = 0; i < nPathSeg; i++) {
		ps[i].setOptLoc(ps[i].getLoc());
		ps[i].setPitLoc(ps[i].getOptLoc());
	}

	/* compute possible speeds, direction vector and length of trajectoies */
	u = nPathSeg - 1; v = 0; w = 1;

	for (i = 0; i < nPathSeg; i++) {
		r = radius(ps[u].getLoc()->x, ps[u].getLoc()->y,
			ps[v].getLoc()->x, ps[v].getLoc()->y, ps[w].getLoc()->x, ps[w].getLoc()->y);
		ps[i].setRadius(r);
		r = fabs(r);

		length = dist(ps[v].getLoc(), ps[w].getLoc());

		tdble mu = track->getSegmentPtr(i)->getKfriction()*myc->CFRICTION*track->getSegmentPtr(i)->getKalpha();
		tdble b = track->getSegmentPtr(i)->getKbeta();
		speedsqr = myc->SPEEDSQRFACTOR*r*g*mu/(1.0 - MIN(1.0, (mu*myc->ca*r/myc->mass)) + mu*r*b);

		dir = (*ps[w].getLoc()) - (*ps[u].getLoc());
		dir.normalize();

		//ps[i].set(speedsqr, length, ps[v].getLoc(), &dir);
		ps[i].set(speedsqr, length, &dir);

		u = v; v = w; w = (w + 1 + nPathSeg) % nPathSeg;
	}

	/* add path to pit to pit-path */
	if (isPitAvailable()) initPitStopPath();
}


/*
	plans a route according to the situation
*/
void Pathfinder::plan(int trackSegId, tCarElt* car, tSituation *situation, MyCar* myc, OtherCar* ocar)
{
	double r, length, speedsqr;
	int u, v, w;
	v3d dir;

	int i, start;

	if (myc->derror > myc->PATHERR*myc->PATHERRFACTOR) {
		start = trackSegId;
	} else {
		start = lastPlan+lastPlanRange;
	}

	if (track->isBetween(e3, s1, trackSegId)) inPit = false;
	/* relies on that pitstop dosen't get enabled between s1, e3 */
	if (track->isBetween(s1, e3, trackSegId) && (pitStop)) inPit = true;

	/* load precomputed trajectory */
	if (!pitStop && !inPit) {
		for (i = start; i < trackSegId+AHEAD+SEGRANGE; i++) {
			int j = (i+nPathSeg) % nPathSeg;
			ps[j].setLoc(ps[j].getOptLoc());
		}
	} else {
		for (i = start; i < trackSegId+AHEAD+SEGRANGE; i++) {
			int j = (i+nPathSeg) % nPathSeg;
			ps[j].setLoc(ps[j].getPitLoc());
		}
	}

	collcars = updateOCar(trackSegId, situation, myc, ocar, o);
	updateOverlapTimer(trackSegId, situation, myc, ocar, o, overlaptimer);

	if (!inPit && (!pitStop || track->isBetween(e3, (s1 - AHEAD + nPathSeg) % nPathSeg, trackSegId))) {
		/* are we on the trajectory or do i need a correction */
		if ((myc->derror > myc->PATHERR*myc->PATHERRFACTOR ||
		(myc->getDeltaPitch() > myc->MAXALLOWEDPITCH && myc->getSpeed() > myc->FLYSPEED))) {
			changed += correctPath(trackSegId, car, myc);
		}
		/* overtaking */
		if (changed == 0) {
			changed += overtake(trackSegId, situation, myc, ocar);
		}
		/* if we have nothing better to, let opponents overlap */
		if (changed == 0) {
			changed = letoverlap(trackSegId, situation, myc, ocar, overlaptimer);
		}
	}

	/* recompute speed and direction of new trajectory */
	if (changed > 0 || (ps[trackSegId].getSpeedsqr() < 5.0)) {
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
		double r2 = radius(ps[u].getLoc()->x, ps[u].getLoc()->y,
			ps[v].getLoc()->x, ps[v].getLoc()->y, ps[w].getLoc()->x, ps[w].getLoc()->y);
		double r1 = radius(ps[u2].getLoc()->x, ps[u2].getLoc()->y,
			ps[v].getLoc()->x, ps[v].getLoc()->y, ps[w2].getLoc()->x, ps[w2].getLoc()->y);

		if (fabs(r1) > fabs(r2)) {
			ps[j].setRadius(r1);
			r = fabs(r1);
		} else {
			ps[j].setRadius(r2);
			r = fabs(r2);
		}

		length = dist(ps[v].getLoc(), ps[w].getLoc());

		/* compute allowed speedsqr */
		double mu = track->getSegmentPtr(j)->getKfriction()*myc->CFRICTION*track->getSegmentPtr(j)->getKalpha();
		double b = track->getSegmentPtr(j)->getKbeta();
		speedsqr = myc->SPEEDSQRFACTOR*r*g*mu/(1.0 - MIN(1.0, (mu*myc->ca*r/myc->mass)) + mu*r*b);
		if (pitStop && track->isBetween(s3, pitSegId, j)) {
			double speedsqrpit = ((double) segmentsToPit(j) / TRACKRES) *2.0*g*track->getSegmentPtr(j)->getKfriction()*myc->CFRICTION*myc->cgcorr_b;
			if (speedsqr > speedsqrpit) speedsqr = speedsqrpit;
		}
		if ((pitStop || inPit) && track->isBetween(s3, e1, j)) {
			if (speedsqr > getPitSpeedSqrLimit()) speedsqr = getPitSpeedSqrLimit();
		}

		dir = (*ps[w].getLoc()) - (*ps[u].getLoc());
		dir.normalize();

		//ps[j].set(speedsqr, length, ps[v].getLoc(), &dir);
		ps[j].set(speedsqr, length, &dir);

		u = v; v = w; w = (w + 1 + nPathSeg) % nPathSeg;
		w2 = (w2 + 1 + nPathSeg) % nPathSeg;
		u2 = (u2 + 1 + nPathSeg) % nPathSeg;
	}

	changed = 0;

	/* set speed limits on the path, in case there is an obstacle (other car) */
	changed += collision(trackSegId, car, situation, myc, ocar);

	lastPlan = trackSegId; lastPlanRange = AHEAD;
}


void Pathfinder::smooth(int id, double delta, double w)
{
	int ids[5] = {id-2, id-1, id, id+1, id+2};
	double x[5], y[5], r, rmin = RMAX;
	TrackSegment* t = track->getSegmentPtr(id);
	v3d* tr = t->getToRight();
	int i;

	for (i = 0; i < 5; i++) {
		ids[i] = (ids[i] + nPathSeg) % nPathSeg;
		x[i] = ps[ids[i]].getLoc()->x;
		y[i] = ps[ids[i]].getLoc()->y;
	}

	for (i = 0; i < 3; i++) {
		r = fabs(radius(x[i], y[i], x[i+1], y[i+1], x[i+2], y[i+2]));
		if (r < rmin) rmin = r;
	}

	/* no optimisation needed */
	if (rmin == RMAX) return;

	double xp, yp, xm, ym, xo = x[2], yo = y[2], rp = RMAX, rm = RMAX;

	xp = x[2] = xo + delta*tr->x; yp = y[2] = yo + delta*tr->y;
	for (i = 0; i < 3; i++) {
		r = fabs(radius(x[i], y[i], x[i+1], y[i+1], x[i+2], y[i+2]));
		if (r < rp) rp = r;
	}

	xm = x[2] = xo - delta*tr->x; ym = y[2] = yo - delta*tr->y;
	for (i = 0; i < 3; i++) {
		r = fabs(radius(x[i], y[i], x[i+1], y[i+1], x[i+2], y[i+2]));
		if (r < rm) rm = r;
	}

	if (rp > rmin && rp > rm) {
		v3d n;
		n.x = xp;
		n.y = yp;
		n.z = ps[id].getLoc()->z + delta*tr->z;
		ps[id].setLoc(&n);
	} else if (rm > rmin && rm > rp) {
		v3d n;
		n.x = xm;
		n.y = ym;
		n.z = ps[id].getLoc()->z - delta*tr->z;
		ps[id].setLoc(&n);
	}
}

void Pathfinder::smooth(int s, int p, int e, double w)
{
	TrackSegment* t = track->getSegmentPtr(p);
	v3d *rgh = t->getToRight();
	v3d *rs = ps[s].getLoc(), *rp = ps[p].getLoc(), *re = ps[e].getLoc(), n;

	double rgx = (re->x - rs->x), rgy = (re->y - rs->y);
	double m = (rs->x * rgy + rgx * rp->y - rs->y * rgx - rp->x * rgy) / (rgy * rgh->x - rgx * rgh->y);

	n = (*rp) + (*rgh)*m;

	ps[p].setLoc(&n);
}


void Pathfinder::optimize(int start, int range, double w)
{
	for (int p = start; p < start + range; p = p + 1) {
		int j = (p) % nPathSeg;
		int k = (p+1) % nPathSeg;
		int l = (p+2) % nPathSeg;
		smooth(j, k, l, w);
	}
}


void Pathfinder::optimize2(int start, int range, double w)
{
	for (int p = start; p < start + range; p = p + 1) {
		int j = (p) % nPathSeg;
		int k = (p+1) % nPathSeg;
		int l = (p+2) % nPathSeg;
		int m = (p+3) % nPathSeg;
		smooth(j, k, m, w);
		smooth(j, l, m, w);
	}
}


void Pathfinder::optimize3(int start, int range, double w)
{
	for (int p = start; p < start + range; p = p + 3) {
		int j = (p) % nPathSeg;
		int k = (p+1) % nPathSeg;
		int l = (p+2) % nPathSeg;
		int m = (p+3) % nPathSeg;
		smooth(j, k, m, w);
		smooth(j, l, m, w);
	}
}


/* collision avoidence with braking */
int Pathfinder::collision(int trackSegId, tCarElt* mycar, tSituation* s, MyCar* myc, OtherCar* ocar)
{
	int end = (trackSegId + (int) COLLDIST + nPathSeg) % nPathSeg;
	int didsomething = 0;
	int i, n = collcars;

	for (i = 0; i < n; i++) {
		if (o[i].overtakee == true) continue;
		int currentsegid = o[i].collcar->getCurrentSegId();
		if (track->isBetween(trackSegId, end, currentsegid) && (myc->getSpeed() > o[i].speed)) {
			int spsegid = (currentsegid - (int) (myc->CARLEN + 1) + nPathSeg) % nPathSeg;

			if (o[i].mincorner < myc->CARWIDTH/2.0 + myc->DIST) {
				double cmpdist = o[i].dist - myc->CARLEN - myc->DIST;
				if ((o[i].brakedist >= cmpdist) && (ps[spsegid].getSpeedsqr() > o[i].speedsqr)) {
					int j;
					for (j = spsegid - 3; j < spsegid + 3; j++) {
						ps[(j + nPathSeg) % nPathSeg].setSpeedsqr(o[i].speedsqr);
					}
					didsomething = 1;
				}
			}

			if (track->isBetween(trackSegId, end, o[i].catchsegid)) {
				double myd = track->distToMiddle(o[i].catchsegid, ps[o[i].catchsegid].getLoc());
				v3d r;
				o[i].collcar->getDir()->crossProduct(myc->getDir(), &r);
				double sina = r.len()*sign(r.z);
				double otherd = o[i].disttomiddle + sina*o[i].collcar->getSpeed()*o[i].time;

				if (fabs(myd - otherd) < myc->CARWIDTH + myc->DIST) {
					if ((o[i].catchdist > 0.0) && (o[i].brakedist >= (o[i].catchdist - (myc->CARLEN + myc->DIST)))) {
						PathSeg* catchseg = getPathSeg((o[i].catchsegid - (int) myc->CARLEN + nPathSeg) % nPathSeg);
						if (catchseg->getSpeedsqr() > o[i].speedsqr) {
							catchseg->setSpeedsqr(o[i].speedsqr);
							didsomething = 1;
						}
					}
				}
			}
		}
	}
	return didsomething;
}


#ifdef PATH_K1999

/* computes curvature, from Remi Coulom, K1999.cpp */
inline double Pathfinder::curvature(double xp, double yp, double x, double y, double xn, double yn)
{
	double x1 = xn - x;
	double y1 = yn - y;
	double x2 = xp - x;
	double y2 = yp - y;
	double x3 = xn - xp;
	double y3 = yn - yp;

	double det = x1 * y2 - x2 * y1;
	double n1 = x1 * x1 + y1 * y1;
	double n2 = x2 * x2 + y2 * y2;
	double n3 = x3 * x3 + y3 * y3;
	double nnn = sqrt(n1 * n2 * n3);
	return 2 * det / nnn;
}


/* optimize point p ala k1999 (curvature), Remi Coulom, K1999.cpp */
inline void Pathfinder::adjustRadius(int s, int p, int e, double c, double security) {
	const double sidedistext = 2.0;
	const double sidedistint = 1.2;

	TrackSegment* t = track->getSegmentPtr(p);
	v3d *rgh = t->getToRight();
	v3d *left = t->getLeftBorder();
	v3d *right = t->getRightBorder();
	v3d *rs = ps[s].getLoc(), *rp = ps[p].getLoc(), *re = ps[e].getLoc(), n;
	double oldlane = track->distToMiddle(p, rp)/t->getWidth() + 0.5;

	double rgx = (re->x - rs->x), rgy = (re->y - rs->y);
	double m = (rs->x * rgy + rgx * rp->y - rs->y * rgx - rp->x * rgy) / (rgy * rgh->x - rgx * rgh->y);

	n = (*rp) +(*rgh)*m;
	ps[p].setLoc(&n);
	double newlane = track->distToMiddle(p, rp)/t->getWidth() + 0.5;

	/* get an estimate how much the curvature changes by moving the point 1/10000 of track width */
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
		v3d* trackmiddle = t->getMiddle();

		n = (*trackmiddle) + (*rgh)*d;
		ps[p].setLoc(&n);
	}
}


/* interpolation step from Remi Coulom, K1999.cpp */
void Pathfinder::stepInterpolate(int iMin, int iMax, int Step)
{
	int next = (iMax + Step) % nPathSeg;
	if (next > nPathSeg - Step) next = 0;

	int prev = (((nPathSeg + iMin - Step) % nPathSeg) / Step) * Step;
	if (prev > nPathSeg - Step)
	prev -= Step;

	v3d *pp = ps[prev].getLoc();
	v3d *p = ps[iMin].getLoc();
	v3d *pn = ps[iMax % nPathSeg].getLoc();
	v3d *pnn = ps[next].getLoc();

	double ir0 = curvature(pp->x, pp->y, p->x, p->y, pn->x, pn->y);
	double ir1 = curvature(p->x, p->y, pn->x, pn->y, pnn->x, pnn->y);

	for (int k = iMax; --k > iMin;) {
		double x = double(k - iMin) / double(iMax - iMin);
		double TargetRInverse = x * ir1 + (1 - x) * ir0;
		adjustRadius(iMin, k, iMax % nPathSeg, TargetRInverse, 0.0);
	}
}


/* interpolating from Remi Coulom, K1999.cpp */
void Pathfinder::interpolate(int step)
{
	if (step > 1) {
		int i;
		for (i = step; i <= nPathSeg - step; i += step) stepInterpolate(i - step, i, step);
		stepInterpolate(i - step, nPathSeg, step);
	}
}


/* smoothing from Remi Coulom, K1999.cpp */
void Pathfinder::smooth(int Step)
{
	int prev = ((nPathSeg - Step) / Step) * Step;
	int prevprev = prev - Step;
	int next = Step;
	int nextnext = next + Step;

	v3d *pp, *p, *n, *nn, *cp;

	for (int i = 0; i <= nPathSeg - Step; i += Step) {
		pp = ps[prevprev].getLoc();
		p = ps[prev].getLoc();
		cp = ps[i].getLoc();
		n = ps[next].getLoc();
		nn = ps[nextnext].getLoc();

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

#endif // PATH_K1999


/* compute a path back to the planned path */
int Pathfinder::correctPath(int id, tCarElt* car, MyCar* myc)
{
	double s[2], y[2], ys[2];
	bool out;

	double d = track->distToMiddle(id, myc->getCurrentPos());
//	double factor = MIN(myc->CORRLEN*fabs(d), nPathSeg/2.0);
	double factor = MIN(MIN(myc->CORRLEN*myc->derror, nPathSeg/2.0), AHEAD);
	int endid = (id + (int) (factor) + nPathSeg) % nPathSeg;

	if (fabs(d) > (track->getSegmentPtr(id)->getWidth() - myc->CARWIDTH)/2.0) {
		d = sign(d)*((track->getSegmentPtr(id)->getWidth() - myc->CARWIDTH)/2.0 - myc->MARGIN);
    	ys[0] = 0.0;
		out = true;
	} else {
		v3d pathdir = *ps[id].getDir();
		pathdir.z = 0.0;
		pathdir.normalize();
		double alpha = PI/2.0 - acos((*myc->getDir())*(*track->getSegmentPtr(id)->getToRight()));
		//if (alpha > 4.0*PI/180.0) alpha = 4.0*PI/180.0;
		//if (alpha < -4.0*PI/180.0) alpha = -4.0*PI/180.0;
		ys[0] = tan(alpha);
		out = false;
	}

	double ed = track->distToMiddle(endid, ps[endid].getLoc());


	/* set up points */
	y[0] = d;
	//ys[0] = 0.0;

	y[1] = ed;
	ys[1] = pathSlope(endid);

	s[0] = 0.0;
	s[1] = countSegments(id, endid);

	/* modify path */
	double l = 0.0;
	v3d q, *pp, *qq;
	int i, j;

	if (out == true) {
		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			d = spline(2, l, s, y, ys);
			if (fabs(d) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH)/2.0) {
				d = sign(d)*((track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH)/2.0 - myc->MARGIN);
			}

			pp = track->getSegmentPtr(j)->getMiddle();
			qq = track->getSegmentPtr(j)->getToRight();
			q = (*pp) + (*qq)*d;
			ps[j].setLoc(&q);
			l += TRACKRES;
		}
	} else {
		double newdisttomiddle[AHEAD];
		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			d = spline(2, l, s, y, ys);
			if (fabs(d) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - myc->MARGIN) {
			    return 0;
			}
			newdisttomiddle[i - id] = d;
			l += TRACKRES;
		}

		for (i = id; (j = (i + nPathSeg) % nPathSeg) != endid; i++) {
			pp = track->getSegmentPtr(j)->getMiddle();
			qq = track->getSegmentPtr(j)->getToRight();
			q = *pp + (*qq)*newdisttomiddle[i - id];
			ps[j].setLoc(&q);
		}
	}

	/* align previos point for getting correct speedsqr in Pathfinder::plan(...) */
	int p = (id - 1 + nPathSeg) % nPathSeg;
	int e = (id + 1 + nPathSeg) % nPathSeg;
	smooth(id, p, e, 1.0);

	return 1;
}


/* compute path for overtaking the "next colliding" car */
int Pathfinder::overtake(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar)
{
	if (collcars == 0) return 0;

	const int start = (trackSegId - (int) (2.0 + myc->CARLEN) + nPathSeg) % nPathSeg;
	const int nearend = (trackSegId + (int) (2.0*myc->CARLEN)) % nPathSeg;

	OtherCar* nearestCar = NULL;	/* car near in time, not in space ! (next reached car) */
	double minTime = FLT_MAX;
	double minorthdist = FLT_MAX;	/* near in space */
	double orthdist = FLT_MAX;
	int minorthdistindex = 0;
	int collcarindex = 0;

	int i, m = 0;
	for (i = 0; i < collcars; i++) {
		if (o[i].dist < COLLDIST/3) {
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
		sidechangeallowed = false;
	} else if (minTime < FLT_MAX){
		nearestCar = o[collcarindex].collcar;
		sidechangeallowed = true;
		minorthdist = orthdist;
		int i;
		for (i = 0; i <= (int) myc->MINOVERTAKERANGE; i += 10) {
			if (track->getSegmentPtr((trackSegId+i) % nPathSeg)->getRadius() < myc->OVERTAKERADIUS) return 0;
		}
	} else return 0;

	/* not enough space, so we try to overtake */
	if (((o[collcarindex].mincorner < myc->CARWIDTH/2.0 + myc->DIST) && (minTime < myc->TIMETOCATCH)) || !sidechangeallowed) {
		int overtakerange = (int) MIN(MAX((3*minTime*myc->getSpeed()), myc->MINOVERTAKERANGE), AHEAD-50);
		double d = o[collcarindex].disttomiddle;
		double mydisttomiddle = track->distToMiddle(myc->getCurrentSegId(), myc->getCurrentPos());
		double y[3], ys[3], s[3];

		y[0] = track->distToMiddle(trackSegId, myc->getCurrentPos());
		double alpha = PI/2.0 - acos((*myc->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));
		int trackSegId1 = (trackSegId + overtakerange/3) % nPathSeg;
		double w = track->getSegmentPtr(nearestCar->getCurrentSegId())->getWidth() / 2;
		double pathtomiddle = track->distToMiddle(trackSegId1, ps[trackSegId1].getLoc());
		double paralleldist = o[collcarindex].cosalpha*dist(myc->getCurrentPos(), nearestCar->getCurrentPos());

		if (!sidechangeallowed) {
			if (paralleldist > 1.5*myc->CARLEN) {
				int i;
				for (i = 0; i <= (int) myc->MINOVERTAKERANGE; i += 10) {
					if (track->getSegmentPtr((trackSegId+i) % nPathSeg)->getRadius() < myc->OVERTAKERADIUS) return 0;
				}
				v3d r, dir = *o[collcarindex].collcar->getCurrentPos()- *myc->getCurrentPos();
				myc->getDir()->crossProduct(&dir, &r);
				double pathtocarsgn = sign(r.z);

				y[1] = d + myc->OVERTAKEDIST*pathtocarsgn;
				if (fabs(y[1]) > w - (1.5*myc->CARWIDTH)) {
					y[1] = d - myc->OVERTAKEDIST*pathtocarsgn;
				}
				double beta = PI/2.0 - acos((*nearestCar->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));
				if (y[1] - mydisttomiddle >= 0.0) {
					if (alpha < beta + myc->OVERTAKEANGLE) alpha = alpha + myc->OVERTAKEANGLE;
				} else {
					if (alpha > beta - myc->OVERTAKEANGLE) alpha = alpha - myc->OVERTAKEANGLE;
				}
			} else {
				double beta = PI/2.0 - acos((*nearestCar->getDir())*(*track->getSegmentPtr(trackSegId)->getToRight()));
				double delta = mydisttomiddle - d;
				if (delta >= 0.0) {
					if (alpha < beta + myc->OVERTAKEANGLE) alpha = beta + myc->OVERTAKEANGLE;
				} else {
					if (alpha > beta - myc->OVERTAKEANGLE) alpha = beta - myc->OVERTAKEANGLE;
				}
				double cartocarsgn = sign(delta);
				y[1] = d + myc->OVERTAKEDIST*cartocarsgn;
				if (fabs(y[1]) > w - (1.5*myc->CARWIDTH)) {
					y[1] = cartocarsgn*(w - (myc->CARWIDTH + myc->MARGIN));
				}
				if (minorthdist > myc->OVERTAKEMINDIST) o[collcarindex].overtakee = true;
			}
		} else {
			double pathtocarsgn = sign(pathtomiddle - d);
			y[1] = d + myc->OVERTAKEDIST*pathtocarsgn;
			if (fabs(y[1]) > w - (1.5*myc->CARWIDTH)) {
				y[1] = d - myc->OVERTAKEDIST*pathtocarsgn;
			}
		}

		double ww = w - (myc->CARWIDTH + myc->MARGIN);
		if ((y[1] >  ww && alpha > 0.0) || (y[1] < -ww && alpha < 0.0)) {
			alpha = 0.0;
		}

		//ys[0] = sin(alpha);
		ys[0] = tan(alpha);
		ys[1] = 0.0;

		/* set up point 2 */
		int trackSegId2 = (trackSegId + overtakerange) % nPathSeg;
		y[2] = track->distToMiddle(trackSegId2, ps[trackSegId2].getOptLoc());
		ys[2] = pathSlope(trackSegId2);

		/* set up parameter s */
		s[0] = 0.0;
		s[1] = countSegments(trackSegId, trackSegId1);
		s[2] = s[1] + countSegments(trackSegId1, trackSegId2);

		/* check path for leaving to track */
		double newdisttomiddle[AHEAD];
		int i, j;
		double l = 0.0; v3d q, *pp, *qq;
		for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId2; i++) {
			d = spline(3, l, s, y, ys);
			if (fabs(d) > (track->getSegmentPtr(j)->getWidth() - myc->CARWIDTH) / 2.0 - myc->MARGIN) {
				o[collcarindex].overtakee = false;
				return 0;
			}
			newdisttomiddle[i - trackSegId] = d;
			l += TRACKRES;
		}

		/* set up the path */
		for (i = trackSegId; (j = (i + nPathSeg) % nPathSeg) != trackSegId2; i++) {
			pp = track->getSegmentPtr(j)->getMiddle();
			qq = track->getSegmentPtr(j)->getToRight();
			q = *pp + (*qq)*newdisttomiddle[i - trackSegId];
			ps[j].setLoc(&q);
		}

		/* reload old trajectory where needed */
		for (i = trackSegId2; (j = (i+nPathSeg) % nPathSeg) != (trackSegId+AHEAD) % nPathSeg; i ++) {
			ps[j].setLoc(ps[j].getOptLoc());
		}

		/* align previos point for getting correct speedsqr in Pathfinder::plan(...) */
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
	const int start = (trackSegId - (int) (1.0 + myc->CARLEN/2.0) + nPathSeg) % nPathSeg;
	const int end = (trackSegId + (int) COLLDIST + nPathSeg) % nPathSeg;

	int i, n = 0;		/* counter for relevant cars */

	for (i = 0; i < s->_ncars; i++) {
		tCarElt* car = ocar[i].getCarPtr();
		/* is it me ? */
		if (car != myc->getCarPtr()) {
			int seg = ocar[i].getCurrentSegId();
			/* get the next car to catch up */
			if (track->isBetween(start, end, seg) && !(car->_state & (RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN))) {
				o[n].cosalpha = (*myc->getDir())*(*ocar[i].getDir());
				o[n].speed = ocar[i].getSpeed()*o[n].cosalpha;
				int j, k = track->diffSegId(trackSegId, seg);
				if ( k < 40) {
					o[n].dist = 0.0;
					int l = MIN(trackSegId, seg);
					for (j = l; j < l + k; j++) o[n].dist += ps[j % nPathSeg].getLength();
					if (o[n].dist > k) o[n].dist = k;
				} else {
					o[n].dist = k;
				}
				o[n].collcar = &ocar[i];
				o[n].time = o[n].dist/(myc->getSpeed() - o[n].speed);
				o[n].disttomiddle = track->distToMiddle(seg, ocar[i].getCurrentPos());
				o[n].speedsqr = sqr(o[n].speed);
				o[n].catchdist = (int) (o[n].dist/(myc->getSpeed() - ocar[i].getSpeed())*myc->getSpeed());
				o[n].catchsegid = (o[n].catchdist + trackSegId + nPathSeg) % nPathSeg;
				o[n].overtakee = false;
				o[n].disttopath = distToPath(seg, ocar[i].getCurrentPos());
				double gm = track->getSegmentPtr(seg)->getKfriction()*myc->CFRICTION;
				double qs = o[n].speedsqr;
				o[n].brakedist = (myc->getSpeedSqr() - o[n].speedsqr)*(myc->mass/(2.0*gm*g*myc->mass + (qs)*(gm*myc->ca)));
				o[n].mincorner = FLT_MAX;
				o[n].minorthdist = FLT_MAX;
				for (j = 0; j < 4; j++) {
					v3d e(car->pub.corner[j].ax, car->pub.corner[j].ay, car->_pos_Z);
					double corner = fabs(distToPath(seg, &e));
					double orthdist = track->distGFromPoint(myc->getCurrentPos(), myc->getDir(), &e) - myc->CARWIDTH/2.0;
					if (corner < o[n].mincorner) o[n].mincorner = corner;
					if (orthdist < o[n].minorthdist) o[n].minorthdist = orthdist;
				}
				n++;
			}
		}
	}
	return n;
}


inline void Pathfinder::updateOverlapTimer(int trackSegId, tSituation *s, MyCar* myc, OtherCar* ocar, tOCar* o, tOverlapTimer* ov)
{
	const int start = (trackSegId - (int) myc->OVERLAPSTARTDIST + nPathSeg) % nPathSeg;
	const int end = (trackSegId - (int) (2.0 + myc->CARLEN/2.0) + nPathSeg) % nPathSeg;
	const int startfront = (trackSegId + (int) (2.0 + myc->CARLEN/2.0)) % nPathSeg;
	const int endfront = (trackSegId + (int) myc->OVERLAPSTARTDIST) % nPathSeg;

	int i;

	for (i = 0; i < s->_ncars; i++) {
		tCarElt* car = ocar[i].getCarPtr();
		tCarElt* me = myc->getCarPtr();
		/* is it me, and in case not, has the opponent more laps than me? */
		if ((car != me) && (car->race.laps > me->race.laps) &&
		!(car->_state & (RM_CAR_STATE_DNF | RM_CAR_STATE_PULLUP | RM_CAR_STATE_PULLSIDE | RM_CAR_STATE_PULLDN))) {
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
	const int start = (trackSegId - (int) myc->OVERLAPPASSDIST + nPathSeg) % nPathSeg;
	const int end = (trackSegId - (int) (2.0 + myc->CARLEN/2.0) + nPathSeg) % nPathSeg;
	int k;

	for (k = 0; k < situation->_ncars; k++) {
		if ((ov[k].time > myc->OVERLAPWAITTIME) && track->isBetween(start, end, ocar[k].getCurrentSegId())) {
			/* let overtake */
			double s[4], y[4], ys[4];
			const int DST = 400;

			ys[0] = pathSlope(trackSegId);
			if (fabs(ys[0]) > PI/180.0) return 0;

			int trackSegId1 = (trackSegId + (int) DST/4 + nPathSeg) % nPathSeg;
			int trackSegId2 = (trackSegId + (int) DST*3/4 + nPathSeg) % nPathSeg;
			int trackSegId3 = (trackSegId + (int) DST + nPathSeg) % nPathSeg;
			double width = track->getSegmentPtr(trackSegId1)->getWidth();

			/* point 0 */
			y[0] = track->distToMiddle(trackSegId, myc->getCurrentPos());

			/* point 1 */
			y[1] = sign(y[0])*MIN((width/2.0 - 2.0*myc->CARWIDTH - myc->MARGIN), (15.0/2.0));
			ys[1] = 0.0;

			/* point 2 */
			y[2] = y[1];
			ys[2] = 0.0;

			/* point 3*/
			y[3] = track->distToMiddle(trackSegId3, ps[trackSegId3].getOptLoc());
			ys[3] = pathSlope(trackSegId3);

			/* set up parameter s */
			s[0] = 0.0;
			s[1] = countSegments(trackSegId, trackSegId1);
			s[2] = s[1] + countSegments(trackSegId1, trackSegId2);
			s[3] = s[2] + countSegments(trackSegId2, trackSegId3);

			/* check path for leaving to track */
			double newdisttomiddle[AHEAD], d;
			int i, j;
			double l = 0.0; v3d q, *pp, *qq;
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
				ps[j].setLoc(&q);
			}

			/* reload old trajectory where needed */
			for (i = trackSegId3; (j = (i+nPathSeg) % nPathSeg) != (trackSegId+AHEAD) % nPathSeg; i ++) {
				ps[j].setLoc(ps[j].getOptLoc());
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
