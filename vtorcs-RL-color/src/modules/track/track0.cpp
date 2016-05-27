/***************************************************************************

    file                 : track0.cpp
    created              : Sun Jan 30 22:55:09 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: track0.cpp,v 1.4.6.1 2008/11/09 17:50:23 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <tgf.h>
#include <track.h>
#include "trackinc.h"

/*
 * Read version 0 track segments
 */
void 
ReadTrack0(tTrack *theTrack, void *TrackHandle, tRoadCam **camList)
{
    int		i,j;
    int		segread, curindex;
    tdble	radius;
    tdble	innerradius;
    tdble	arc;
    tdble	length;
    tTrackSeg	*curSeg;
    tdble	alf;
    tdble	xr, yr, newxr, newyr;
    tdble	xl, yl, newxl, newyl;
    tdble	cenx, ceny;
    tdble	width, wi2;
    tdble	xmin, xmax, ymin, ymax, zmin, zmax;
    tdble	x1, x2, y1, y2;
    tdble	al, alfl;
    tdble	zsl, zsr, zel, zer, zs, ze;
    char        *segtype = (char*)NULL;
    char	*material;
    tdble	friction = 1.0;
    tdble	rollres = 0.001;
    /* sides */
    tdble	lsw, rsw;
    tdble	lssw, lsew;
    tdble	rssw, rsew;
    char	path[256];
    
    
#define TSTX(x)			\
    if (xmin > (x)) xmin = (x);	\
    if (xmax < (x)) xmax = (x);

#define TSTY(y)			\
    if (ymin > (y)) ymin = (y);	\
    if (ymax < (y)) ymax = (y);

#define TSTZ(z)			\
    if (zmin > (z)) zmin = (z);	\
    if (zmax < (z)) zmax = (z);

    width = theTrack->width;
    wi2 = width / 2.0;
    xr = xl = newxr = newxl = 0.0;
    yr = newyr = newyl = 0.0;
    yl = width;
    xmin = xmax = ymin = zmin = zmax = 0.0;
    ymax = yl;
    alf = alfl = 0.0;
    zsl = zsr = zel = zer = zs = ze = 0.0;
    lsw = rsw = lssw = lsew = rssw = rsew = 0.0;

    /* Main Track */
    material = GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_SURF, TRK_VAL_ASPHALT);
	
    segread = 0;
    curindex = 0;

    sprintf(path , "%s/%s", TRK_SECT_MAIN, TRK_LST_SEG);
    GfParmListSeekFirst(TrackHandle, path);
    do {
	segtype = GfParmGetCurStr(TrackHandle, path, TRK_ATT_TYPE, NULL);
	
	curindex++;
	if (segtype == 0) {
	    continue;
	}
	segread++;
	
	zsl = zel;
	zsr = zer;
	TSTZ(zsl);
	TSTZ(zsr);
	

	/* allocate a new segment */
	curSeg = (tTrackSeg*)calloc(1, sizeof(tTrackSeg));
	if (theTrack->seg == NULL) {
	    theTrack->seg = curSeg;
	    curSeg->next = curSeg;
	} else {
	    curSeg->next = theTrack->seg->next;
	    theTrack->seg->next = curSeg;
	    theTrack->seg = curSeg;
	}
	curSeg->name = GfParmListGetCurEltName(TrackHandle, path);
	curSeg->id = curindex-1;
	curSeg->width = width;
	curSeg->material = material;
	curSeg->kFriction = friction;
	curSeg->kRollRes = rollres;
	curSeg->kRoughness = 0;
	curSeg->kRoughWaveLen = 1.0;
	curSeg->lgfromstart = theTrack->length;

	zsl = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZSL, (char*)NULL, zsl);
	zsr = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZSR, (char*)NULL, zsr);
	zel = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZEL, (char*)NULL, zel);
	zer = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZER, (char*)NULL, zer);
	ze = zs = -100000.0;
	ze = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZE, (char*)NULL, ze);
	zs = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZS, (char*)NULL, zs);

	if (ze != -100000.0) {
	    zer = zel = ze;
	}
	if (zs != -100000.0) {
	    zsr = zsl = zs;
	}
	TSTZ(zsl);
	TSTZ(zsr);


	if (strcmp(segtype, TRK_VAL_STR) == 0) {
	    /* straight */
	    length = GfParmGetCurNum(TrackHandle, path, TRK_ATT_LG, (char*)NULL, 0);
	    
	    curSeg->type = TR_STR;
	    curSeg->length = length;

	    newxr = xr + length * cos(alf);      /* find end coordinates */
	    newyr = yr + length * sin(alf);
	    newxl = xl + length * cos(alf);
	    newyl = yl + length * sin(alf);

	    curSeg->vertex[TR_SR].x = xr;
	    curSeg->vertex[TR_SR].y = yr;
	    curSeg->vertex[TR_SR].z = zsr;

	    curSeg->vertex[TR_SL].x = xl;
	    curSeg->vertex[TR_SL].y = yl;
	    curSeg->vertex[TR_SL].z = zsl;

	    curSeg->vertex[TR_ER].x = newxr;
	    curSeg->vertex[TR_ER].y = newyr;
	    curSeg->vertex[TR_ER].z = zer;

	    curSeg->vertex[TR_EL].x = newxl;
	    curSeg->vertex[TR_EL].y = newyl;
	    curSeg->vertex[TR_EL].z = zel;

	    curSeg->angle[TR_ZS] = alf;
	    curSeg->angle[TR_ZE] = alf;
	    curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, length);
	    curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, length);
	    curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
	    curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);
	    
	    curSeg->Kzl = tan(curSeg->angle[TR_YR]);
	    curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / length;
	    curSeg->Kyl = 0;

	    curSeg->rgtSideNormal.x = -sin(alf);
	    curSeg->rgtSideNormal.y = cos(alf);

	    TSTX(newxr); TSTX(newxl);
	    TSTY(newyr); TSTY(newyl);


	} else if (strcmp(segtype, TRK_VAL_LFT) == 0) {
	    /* left curve */
	    radius = GfParmGetCurNum(TrackHandle, path, TRK_ATT_RADIUS, (char*)NULL, 0);
	    arc = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ARC, (char*)NULL, 0);

	    curSeg->type = TR_LFT;
	    curSeg->radius = radius;
	    curSeg->radiusr = radius + wi2;
	    curSeg->radiusl = radius - wi2;
	    curSeg->arc = arc;
	    curSeg->length = radius * arc;
	    
	    innerradius = radius - wi2; /* left side aligned */
	    cenx = xl - innerradius * sin(alf);  /* compute center location: */
	    ceny = yl + innerradius * cos(alf);
	    curSeg->center.x = cenx;
	    curSeg->center.y = ceny;

	    curSeg->angle[TR_ZS] = alf;
	    curSeg->angle[TR_CS] = alf - PI / 2.0;
	    alf += arc;
	    curSeg->angle[TR_ZE] = alf;

	    newxl = cenx + innerradius * sin(alf);   /* location of end */
	    newyl = ceny - innerradius * cos(alf);
	    newxr = cenx + (innerradius + width) * sin(alf);   /* location of end */
	    newyr = ceny - (innerradius + width) * cos(alf);

	    curSeg->vertex[TR_SR].x = xr;
	    curSeg->vertex[TR_SR].y = yr;
	    curSeg->vertex[TR_SR].z = zsr;

	    curSeg->vertex[TR_SL].x = xl;
	    curSeg->vertex[TR_SL].y = yl;
	    curSeg->vertex[TR_SL].z = zsl;

	    curSeg->vertex[TR_ER].x = newxr;
	    curSeg->vertex[TR_ER].y = newyr;
	    curSeg->vertex[TR_ER].z = zer;

	    curSeg->vertex[TR_EL].x = newxl;
	    curSeg->vertex[TR_EL].y = newyl;
	    curSeg->vertex[TR_EL].z = zel;

	    curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, arc * (innerradius + width));
	    curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, arc * innerradius);
	    curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
	    curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);

	    curSeg->Kzl = tan(curSeg->angle[TR_YR]) * (innerradius + width);
	    curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / arc;
	    curSeg->Kyl = 0;
	    
	    /* to find the boundary */
	    al = (curSeg->angle[TR_ZE] - curSeg->angle[TR_ZS])/36.0;
	    alfl = curSeg->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSeg->center.x + (innerradius) * sin(alfl);   /* location of end */
		y1 = curSeg->center.y - (innerradius) * cos(alfl);
		x2 = curSeg->center.x + (innerradius + width) * sin(alfl);   /* location of end */
		y2 = curSeg->center.y - (innerradius + width) * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }

	} else if (strcmp(segtype, TRK_VAL_RGT) == 0) {
	    /* right curve */
	    radius = GfParmGetCurNum(TrackHandle, path, TRK_ATT_RADIUS, (char*)NULL, 0);
	    arc = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ARC, (char*)NULL, 0);

	    curSeg->type = TR_RGT;
	    curSeg->radius = radius;
	    curSeg->radiusr = radius - wi2;
	    curSeg->radiusl = radius + wi2;
	    curSeg->arc = arc;
	    curSeg->length = radius * arc;

	    innerradius = radius - wi2; /* right side aligned */
	    cenx = xr + innerradius * sin(alf);  /* compute center location */
	    ceny = yr - innerradius * cos(alf);
	    curSeg->center.x = cenx;
	    curSeg->center.y = ceny;

	    curSeg->angle[TR_ZS] = alf;
	    curSeg->angle[TR_CS] = alf + PI / 2.0;
	    alf -= curSeg->arc;
	    curSeg->angle[TR_ZE] = alf;

	    newxl = cenx - (innerradius + width) * sin(alf);   /* location of end */
	    newyl = ceny + (innerradius + width) * cos(alf);
	    newxr = cenx - innerradius * sin(alf);   /* location of end */
	    newyr = ceny + innerradius * cos(alf);

	    curSeg->vertex[TR_SR].x = xr;
	    curSeg->vertex[TR_SR].y = yr;
	    curSeg->vertex[TR_SR].z = zsr;

	    curSeg->vertex[TR_SL].x = xl;
	    curSeg->vertex[TR_SL].y = yl;
	    curSeg->vertex[TR_SL].z = zsl;

	    curSeg->vertex[TR_ER].x = newxr;
	    curSeg->vertex[TR_ER].y = newyr;
	    curSeg->vertex[TR_ER].z = zer;

	    curSeg->vertex[TR_EL].x = newxl;
	    curSeg->vertex[TR_EL].y = newyl;
	    curSeg->vertex[TR_EL].z = zel;

	    curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, arc * innerradius);
	    curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, arc * (innerradius + width));
	    curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
	    curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);

	    curSeg->Kzl = tan(curSeg->angle[TR_YR]) * innerradius;
	    curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / arc;
	    curSeg->Kyl = 0;

	    /* to find the boundaries */
	    al = (curSeg->angle[TR_ZE] - curSeg->angle[TR_ZS])/36.0;
	    alfl = curSeg->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSeg->center.x - (innerradius + width) * sin(alfl);   /* location of end */
		y1 = curSeg->center.y + (innerradius + width) * cos(alfl);
		x2 = curSeg->center.x - innerradius * sin(alfl);   /* location of end */
		y2 = curSeg->center.y + innerradius * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }

	}

	theTrack->length += curSeg->length;
	xr = newxr;
	yr = newyr;
	xl = newxl;
	yl = newyl;
    } while (GfParmListSeekNext(TrackHandle, path) == 0);

    theTrack->nseg = segread;

    /* Update the coord to be positives */
    theTrack->min.x = 0;
    theTrack->min.y = 0;
    theTrack->min.z = 0;
    theTrack->max.x = xmax - xmin;
    theTrack->max.y = ymax - ymin;
    theTrack->max.z = zmax - zmin;

    curSeg = theTrack->seg;
    for(i=0; i<theTrack->nseg; i++)  {         /* read the segment data: */
	if (i == 0) {
	    curSeg->raceInfo = TR_START;
	} else if (i == theTrack->nseg-1) {
	    curSeg->raceInfo = TR_LAST;
	} else {
	    curSeg->raceInfo = TR_NORMAL;
	}
	curSeg->vertex[TR_SR].x -= xmin;
	curSeg->vertex[TR_SR].y -= ymin;
	curSeg->vertex[TR_SR].z -= zmin;
	curSeg->vertex[TR_SL].x -= xmin;
	curSeg->vertex[TR_SL].y -= ymin;
	curSeg->vertex[TR_SL].z -= zmin;
	curSeg->vertex[TR_ER].x -= xmin;
	curSeg->vertex[TR_ER].y -= ymin;
	curSeg->vertex[TR_ER].z -= zmin;
	curSeg->vertex[TR_EL].x -= xmin;
	curSeg->vertex[TR_EL].y -= ymin;
	curSeg->vertex[TR_EL].z -= zmin;
	curSeg->center.x -= xmin;
	curSeg->center.y -= ymin;
	curSeg->next->prev = curSeg;
	curSeg = curSeg->next;
    }
}

