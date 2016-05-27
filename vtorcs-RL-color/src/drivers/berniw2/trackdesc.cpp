/***************************************************************************

    file                 : trackdesc.cpp
    created              : Tue Oct 9 16:52:00 CET 2001
    copyright            : (C) 2001-2002 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: trackdesc.cpp,v 1.5 2003/08/12 23:56:49 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "trackdesc.h"
#include "berniw.h"

#ifdef DMALLOC
#include "dmalloc.h"
#endif

TrackSegment::TrackSegment()
{
	// nothing so far
}

void TrackSegment::init(int id, const tTrackSeg* s, const v3d* lp, const v3d* mp, const v3d* rp)
{
	/* pointer to the corresponding segment */
	pTrackSeg = (tTrackSeg*) s;

	/* right, middle and left segment (road) border, pointer to right side */
	l = *lp; m = *mp; r = *rp;
	r.dirVector(&l, &tr);
	tr.normalize();

	/* fill in the remaining properties */
	type = s->type;
	raceType = s->raceInfo;
	if (type != TR_STR) radius = s->radius; else radius = FLT_MAX;

	if (s->type == TR_LFT) {
		if (s->lside != NULL && s->lside->style == TR_CURB) l = l - 1.5*tr;
		//else if ((s->lside->style == TR_PLAN) && (strcmp(s->lside->surface->material, TRK_VAL_CONCRETE) == 0)) l = l - 3.0*tr;
	}
	if (s->type == TR_RGT) {
		if (s->rside != NULL && s->rside->style == TR_CURB) r = r + 1.5*tr;
		//else if ((s->rside->style == TR_PLAN) && (strcmp(s->rside->surface->material, TRK_VAL_CONCRETE) == 0)) r = r + 3.0*tr;
	}

	width = distToLeft3D(&r);

	double dz = getRightBorder()->z - getLeftBorder()->z;
	double d = getWidth();
	if (type == TR_LFT) {
		if (dz > 0.0) {
			kalpha = 1.0;
		} else {
			kalpha = cos(asin(fabs(dz/d)));
		}
	} else if (type == TR_RGT) {
		if (dz < 0.0) {
			kalpha = 1.0;
		} else {
			kalpha = cos(asin(fabs(dz/d)));
		}
	} else {
		kalpha = 1.0;
	}
}

TrackSegment::~TrackSegment()
{
	// nothing so far
}

TrackDesc::TrackDesc(const tTrack* track)
{
	tTrackSeg* first = track->seg;
	tTrackSeg* seg = first;
	double tracklength = 0.0;
	int nsegments = 0;

	/* compute the length of the track */
	do {
		tracklength += seg->length;
		seg = seg->next;
	} while (seg != first);
	nsegments = (int) floor(tracklength);


	/* allocate memory for the track data */
	nTrackSegments = nsegments;
	ts = new TrackSegment[nTrackSegments];
	torcstrack = (tTrack*) track;

	/* init all the segments of my track description */
	v3d l, m, r;
	int currentts = 0;
	double lastseglen = 0.0;
	double curseglen = 0.0;
	seg = (tTrackSeg*) first;

	do {
		if (seg->type == TR_STR) {
			double dxl = (seg->vertex[TR_EL].x - seg->vertex[TR_SL].x) / (seg->length / TRACKRES);
			double dyl = (seg->vertex[TR_EL].y - seg->vertex[TR_SL].y) / (seg->length / TRACKRES);
			double dzl = (seg->vertex[TR_EL].z - seg->vertex[TR_SL].z) / (seg->length / TRACKRES);
			double dxr = (seg->vertex[TR_ER].x - seg->vertex[TR_SR].x) / (seg->length / TRACKRES);
			double dyr = (seg->vertex[TR_ER].y - seg->vertex[TR_SR].y) / (seg->length / TRACKRES);
			double dzr = (seg->vertex[TR_ER].z - seg->vertex[TR_SR].z) / (seg->length / TRACKRES);

			for (int i = 0; curseglen < seg->length && currentts < nTrackSegments; i++) {

				l.x = seg->vertex[TR_SL].x + dxl*curseglen;
				l.y = seg->vertex[TR_SL].y + dyl*curseglen;
				l.z = seg->vertex[TR_SL].z + dzl*curseglen;

				r.x = seg->vertex[TR_SR].x + dxr*curseglen;
				r.y = seg->vertex[TR_SR].y + dyr*curseglen;
				r.z = seg->vertex[TR_SR].z + dzr*curseglen;

				m = (l + r)/2.0;

				ts[currentts].init(seg->id, seg, &l, &m, &r);
				currentts++;

				lastseglen = curseglen;
				curseglen += TRACKRES;
			}
		} else {
			double dphi = TRACKRES / seg->radius;
			double xc = seg->center.x;
			double yc = seg->center.y;
			double dzl = (seg->vertex[TR_EL].z - seg->vertex[TR_SL].z) / (seg->length / TRACKRES);
			double dzr = (seg->vertex[TR_ER].z - seg->vertex[TR_SR].z) / (seg->length / TRACKRES);

			dphi = (seg->type == TR_LFT) ? dphi : -dphi;
			for (int i = 0; curseglen < seg->length && currentts < nTrackSegments; i++) {
				double phi = curseglen * dphi;
				double cs = cos(phi), ss = sin(phi);
				l.x = seg->vertex[TR_SL].x * cs - seg->vertex[TR_SL].y * ss - xc * cs + yc * ss + xc;
				l.y = seg->vertex[TR_SL].x * ss + seg->vertex[TR_SL].y * cs - xc * ss - yc * cs + yc;
				l.z = seg->vertex[TR_SL].z + dzl*curseglen;

				r.x = seg->vertex[TR_SR].x * cs - seg->vertex[TR_SR].y * ss - xc * cs + yc * ss + xc;
				r.y = seg->vertex[TR_SR].x * ss + seg->vertex[TR_SR].y * cs - xc * ss - yc * cs + yc;
				r.z = seg->vertex[TR_SR].z + dzr*curseglen;

				m = (l + r)/2.0;

				ts[currentts].init(seg->id, seg, &l, &m, &r);
				currentts++;

				lastseglen = curseglen;
				curseglen += TRACKRES;
			}
		}

		curseglen = TRACKRES - (seg->length - lastseglen);
		lastseglen = curseglen;
		while (curseglen > TRACKRES) {
			curseglen -= TRACKRES;
		}

		seg = seg->next;
	} while ( seg != first);

	if (currentts != nTrackSegments) printf("error: TrackDesc::TrackDesc currentts %d != nTrackSegments %d.\n", currentts, nTrackSegments);

	int i;

	/* setting up pit and length */
	for (i = 0; i < nTrackSegments; i++) {
		int j = (i+nTrackSegments-1) % nTrackSegments;
		int k = (i+nTrackSegments+1) % nTrackSegments;
		if ((ts[i].getRaceType() & TR_PITENTRY) && !(ts[j].getRaceType() & TR_PITENTRY)) {
			nPitEntryStart = i;
		}
		if ((ts[i].getRaceType() & TR_PITEXIT) && !(ts[k].getRaceType() & TR_PITEXIT)) {
			nPitExitEnd = i;
		}
		v3d* p = ts[k].getMiddle();
		ts[i].setLength(ts[i].distToMiddle2D(p->x, p->y));
	}

	/* init kbeta, for height profile of track */
	v3d *p0, *p1, *p2;
	double dz10, dz21;

	for (i = 0; i < nTrackSegments; i++) {
		p0 = ts[(i+nTrackSegments-5) % nTrackSegments].getMiddle();
		p1 = ts[(i+nTrackSegments) % nTrackSegments].getMiddle();
		p2 = ts[(i+nTrackSegments+5) % nTrackSegments].getMiddle();
		dz10 = p1->z - p0->z;
		dz21 = p2->z - p1->z;

		if (dz21 < dz10) {
			double dl10, dl21, r;
			v3d pr;
			dirVector2D(p1, p0, &pr);
			dl10 = pr.len();
			dirVector2D(p2, p1, &pr);
			dl21 = pr.len();
			r = fabs(radius(0.0, p0->z, dl10, p1->z, dl21+dl10, p2->z));
			if (r < RREL) {
				ts[i].setKbeta(1.0/r);
			} else {
				ts[i].setKbeta(0.0);
			}
		} else {
			ts[i].setKbeta(0.0);
		}
	}

	for (i = 0; i < nTrackSegments; i++) {
		p0 = ts[(i+nTrackSegments-3) % nTrackSegments].getMiddle();
		p1 = ts[(i+nTrackSegments+3) % nTrackSegments].getMiddle();
		ts[i].setKgamma(atan((p1->z - p0->z)/6.0));
	}
}

TrackDesc::~TrackDesc()
{
	delete [] ts;
}

void TrackDesc::plot(char* filename)
{
	FILE *fd = fopen(filename, "w");
	v3d *l, *m, *r;

	/* plot track */
	for (int i = 0; i < getnTrackSegments(); i++) {
		TrackSegment* p = getSegmentPtr(i);
		l = p->getLeftBorder();
		fprintf(fd, "%f\t%f\n", l->x, l->y );
		m = p->getMiddle();
		fprintf(fd, "%f\t%f\n", m->x, m->y );
		r = p->getRightBorder();
		fprintf(fd, "%f\t%f\n", r->x, r->y );
	}

	fclose(fd);
}


/* get the segment on which the car is, searching ALL the segments */
int TrackDesc::getCurrentSegment(tCarElt* car)
{
	double d, min = FLT_MAX;
	TrackSegment* ts;
	int minindex = 0;

    for (int i = 0; i < getnTrackSegments(); i++) {
		ts = getSegmentPtr(i);
		d = ts->distToMiddle3D(car->_pos_X, car->_pos_Y, car->_pos_Z);
		if (d < min) {
			min = d;
			minindex = i;
		}
	}
	return minindex;
}


int TrackDesc::getNearestId(v3d* p)
{
	double tmp, dist = FLT_MAX;
	int minindex = 0;

	for (int i = 0; i < getnTrackSegments(); i++) {
		tmp = getSegmentPtr(i)->distToMiddle3D(p);
		if (tmp < dist) {
			dist = tmp;
			minindex = i;
		}
	}

	return minindex;
}


