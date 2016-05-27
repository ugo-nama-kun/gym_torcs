/***************************************************************************

    file                 : track1.cpp
    created              : Sun Jan 30 22:55:20 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: track1.cpp,v 1.5.6.1 2008/11/09 17:50:23 berniw Exp $

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

static tdble	xmin, xmax, ymin, ymax, zmin, zmax;

#define TSTX(x)			\
    if (xmin > (x)) xmin = (x);	\
    if (xmax < (x)) xmax = (x);

#define TSTY(y)			\
    if (ymin > (y)) ymin = (y);	\
    if (ymax < (y)) ymax = (y);

#define TSTZ(z)			\
    if (zmin > (z)) zmin = (z);	\
    if (zmax < (z)) zmax = (z);

static void
AddSide(tTrackSeg *curSeg, tdble width, char *material, int side, int type, void *TrackHandle)
{
    tTrackSeg	*curSide;
    tdble	x, y, z;
    tdble	al, alfl;
    int		j;
    tdble	x1, x2, y1, y2;
    char	path[256];

    x = y = z = 0;

    curSide = (tTrackSeg*)calloc(1, sizeof(tTrackSeg));
    if (side == 1) {
	curSeg->lside = curSide;
	curSide->vertex[TR_SR] = curSeg->vertex[TR_SL];
	curSide->vertex[TR_ER] = curSeg->vertex[TR_EL];
    } else {
	curSeg->rside = curSide;
	curSide->vertex[TR_SL] = curSeg->vertex[TR_SR];
	curSide->vertex[TR_EL] = curSeg->vertex[TR_ER];
    }

    curSide->width = width;
    curSide->type = curSeg->type;
    curSide->material = material;
    sprintf(path, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, material);
    curSide->kFriction = GfParmGetNum(TrackHandle, path, TRK_ATT_FRICTION, (char*)NULL, 0.8);
    curSide->kRollRes = GfParmGetNum(TrackHandle, path, TRK_ATT_ROLLRES, (char*)NULL, 0.001);
    curSide->kRoughness = GfParmGetNum(TrackHandle, path, TRK_ATT_ROUGHT, (char*)NULL, 0.0) /  2.0;
    curSide->kRoughWaveLen = 2.0 * PI / GfParmGetNum(TrackHandle, path, TRK_ATT_ROUGHTWL, (char*)NULL, 1.0);

    curSide->angle[TR_XS] = curSeg->angle[TR_XS] * (tdble)type;
    curSide->angle[TR_XE] = curSeg->angle[TR_XE] * (tdble)type;
    curSide->angle[TR_ZS] = curSeg->angle[TR_ZS];
    curSide->angle[TR_ZE] = curSeg->angle[TR_ZE];
    curSide->angle[TR_CS] = curSeg->angle[TR_CS];

    switch(curSeg->type) {
    case TR_STR:
	curSide->length = curSeg->length;

	switch(side) {
	case 1:
	    curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x + width * curSeg->rgtSideNormal.x;
	    curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y + width * curSeg->rgtSideNormal.y;
	    curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    x = curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x + width * curSeg->rgtSideNormal.x;	    
	    y = curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y + width * curSeg->rgtSideNormal.y;
	    z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + (tdble)type * width * tan(curSeg->angle[TR_XE]);
	    break;
	case 0:
	    curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x - width * curSeg->rgtSideNormal.x;
	    curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y - width * curSeg->rgtSideNormal.y;
	    curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    x = curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x - width * curSeg->rgtSideNormal.x;	    
	    y = curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y - width * curSeg->rgtSideNormal.y;
	    z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * width * tan(curSeg->angle[TR_XE]);
	    break;
	}
	curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z, curSide->length);
	curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z, curSide->length);

	curSide->Kzl = tan(curSide->angle[TR_YR]);
	curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->length;
	curSide->Kyl = 0;

	curSide->rgtSideNormal.x = curSeg->rgtSideNormal.x;
	curSide->rgtSideNormal.y = curSeg->rgtSideNormal.y;

	TSTX(x);
	TSTY(y);
	TSTZ(z);
	break;

    case TR_LFT:
	curSide->center.x = curSeg->center.x;
	curSide->center.y = curSeg->center.y;

	switch(side) {
	case 1:
	    curSide->radius = curSeg->radiusl - width / 2.0;
	    curSide->radiusr = curSeg->radiusl;
	    curSide->radiusl = curSeg->radiusl - width;
	    curSide->arc = curSeg->arc;
	    curSide->length = curSide->radius * curSide->arc;

	    curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x - width * cos(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y - width * sin(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x - width * cos(curSide->angle[TR_CS] + curSide->arc);	    
	    curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y - width * sin(curSide->angle[TR_CS] + curSide->arc);
	    z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + (tdble)type * width * tan(curSeg->angle[TR_XE]);

	    curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					 curSide->arc * curSide->radiusr);
	    curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					 curSide->arc * curSide->radiusl);

	    curSide->Kzl = tan(curSide->angle[TR_YR]) * curSide->radiusr;
	    curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
	    curSide->Kyl = 0;

	    /* to find the boundary */
	    al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
	    alfl = curSide->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSide->center.x + (curSide->radiusl) * sin(alfl);   /* location of end */
		y1 = curSide->center.y - (curSide->radiusl) * cos(alfl);
		x2 = curSide->center.x + (curSide->radiusr) * sin(alfl);   /* location of end */
		y2 = curSide->center.y - (curSide->radiusr) * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }
	    TSTZ(z);
	    break;

	case 0:
	    curSide->radius = curSeg->radiusr + width / 2.0;
	    curSide->radiusl = curSeg->radiusr;
	    curSide->radiusr = curSeg->radiusr + width;
	    curSide->arc = curSeg->arc;
	    curSide->length = curSide->radius * curSide->arc;

	    curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x + width * cos(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y + width * sin(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x + width * cos(curSide->angle[TR_CS] + curSide->arc);	    
	    curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y + width * sin(curSide->angle[TR_CS] + curSide->arc);
	    z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * width * tan(curSeg->angle[TR_XE]);

	    curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					 curSide->arc * curSide->radiusr);
	    curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					 curSide->arc * curSide->radiusl);

	    curSide->Kzl = tan(curSide->angle[TR_YR]) * (curSide->radiusr);
	    curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
	    curSide->Kyl = 0;

	    /* to find the boundary */
	    al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
	    alfl = curSide->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSide->center.x + (curSide->radiusl) * sin(alfl);   /* location of end */
		y1 = curSide->center.y - (curSide->radiusl) * cos(alfl);
		x2 = curSide->center.x + (curSide->radiusr) * sin(alfl);   /* location of end */
		y2 = curSide->center.y - (curSide->radiusr) * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }
	    TSTZ(z);
	    break;

	}
	break;
    case TR_RGT:
	curSide->center.x = curSeg->center.x;
	curSide->center.y = curSeg->center.y;

	switch(side) {
	case 1:
	    curSide->radius = curSeg->radiusl + width / 2.0;
	    curSide->radiusr = curSeg->radiusl;
	    curSide->radiusl = curSeg->radiusl + width;
	    curSide->arc = curSeg->arc;
	    curSide->length = curSide->radius * curSide->arc;

	    curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x + width * cos(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y + width * sin(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x + width * cos(curSide->angle[TR_CS] - curSide->arc);	    
	    curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y + (tdble)type * width * sin(curSide->angle[TR_CS] - curSide->arc);
	    z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + width * tan(curSeg->angle[TR_XE]);

	    curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					 curSide->arc * curSide->radiusr);
	    curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					 curSide->arc * curSide->radiusl);

	    curSide->Kzl = tan(curSide->angle[TR_YR]) * curSide->radiusr;
	    curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
	    curSide->Kyl = 0;

	    /* to find the boundary */
	    al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
	    alfl = curSide->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSide->center.x - (curSide->radiusl) * sin(alfl);   /* location of end */
		y1 = curSide->center.y + (curSide->radiusl) * cos(alfl);
		x2 = curSide->center.x - (curSide->radiusr) * sin(alfl);   /* location of end */
		y2 = curSide->center.y + (curSide->radiusr) * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }
	    TSTZ(z);
	    break;

	case 0:
	    curSide->radius = curSeg->radiusr - width / 2.0;
	    curSide->radiusl = curSeg->radiusr;
	    curSide->radiusr = curSeg->radiusr - width;
	    curSide->arc = curSeg->arc;
	    curSide->length = curSide->radius * curSide->arc;

	    curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x - width * cos(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y - width * sin(curSide->angle[TR_CS]);
	    curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * width * tan(curSeg->angle[TR_XS]);
	    curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x - width * cos(curSide->angle[TR_CS] - curSide->arc);	    
	    curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y - width * sin(curSide->angle[TR_CS] - curSide->arc);
	    z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * width * tan(curSeg->angle[TR_XE]);

	    curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					 curSide->arc * curSide->radiusr);
	    curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					 curSide->arc * curSide->radiusl);

	    curSide->Kzl = tan(curSide->angle[TR_YR]) * (curSide->radiusr);
	    curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
	    curSide->Kyl = 0;

	    /* to find the boundary */
	    al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
	    alfl = curSide->angle[TR_ZS];

	    for (j = 0; j < 36; j++) {
		alfl += al;
		x1 = curSide->center.x - (curSide->radiusl) * sin(alfl);   /* location of end */
		y1 = curSide->center.y + (curSide->radiusl) * cos(alfl);
		x2 = curSide->center.x - (curSide->radiusr) * sin(alfl);   /* location of end */
		y2 = curSide->center.y - (curSide->radiusr) * cos(alfl);
		TSTX(x1); TSTX(x2);
		TSTY(y1); TSTY(y2);
	    }
	    TSTZ(z);
	    break;
	}
	break;
    }
}

static void
normSeg(tTrackSeg *curSeg)
{
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
}

/*
 * Read version 1 track segments
 */
void 
ReadTrack1(tTrack *theTrack, void *TrackHandle, tRoadCam **camList)
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
    tdble	x1, x2, y1, y2;
    tdble	al, alfl;
    tdble	zsl, zsr, zel, zer, zs, ze;
    char        *segtype = (char*)NULL;
    char	*material;
    char	*segName;
    int		segId;
    tRoadCam	*curCam;
    tTrkLocPos	trkPos;
    tdble	kFriction, kRollRes;
    tdble	kRoughness, kRoughWaveLen;
    char	path[256];
    char	path2[256];

    /* sides */
    tdble	lsw, rsw;
    char	*lsmaterial;
    char	*rsmaterial;
    int 	lst, rst;
    
    

    width = theTrack->width;
    wi2 = width / 2.0;
    xr = xl = newxr = newxl = 0.0;
    yr = newyr = newyl = 0.0;
    yl = width;
    xmin = xmax = ymin = zmin = zmax = 0.0;
    ymax = yl;
    alf = alfl = 0.0;
    zsl = zsr = zel = zer = zs = ze = 0.0;
    lsw = rsw = 0.0;

    /* Main Track */
    material = GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_SURF, TRK_VAL_ASPHALT);
    sprintf(path, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, material);
    kFriction = GfParmGetNum(TrackHandle, path, TRK_ATT_FRICTION, (char*)NULL, 0.8);
    kRollRes = GfParmGetNum(TrackHandle, path, TRK_ATT_ROLLRES, (char*)NULL, 0.001);
    kRoughness = GfParmGetNum(TrackHandle, path, TRK_ATT_ROUGHT, (char*)NULL, 0.0) / 2.0;
    kRoughWaveLen = 2.0 * PI / GfParmGetNum(TrackHandle, path, TRK_ATT_ROUGHTWL, (char*)NULL, 1.0);
    lsw = GfParmGetNum(TrackHandle, TRK_SECT_HDR, TRK_ATT_LSW, (char*)NULL, 0.0);
    rsw = GfParmGetNum(TrackHandle, TRK_SECT_HDR, TRK_ATT_RSW, (char*)NULL, 0.0);
    lsmaterial = GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_LSSURF, TRK_VAL_GRASS);
    rsmaterial = GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_RSSURF, TRK_VAL_GRASS);

    if (strcmp("level", GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_RST, "level")) == 0) {
	rst = 0;
    } else {
	rst = 1;
    }
    if (strcmp("level", GfParmGetStr(TrackHandle, TRK_SECT_HDR, TRK_ATT_LST, "level")) == 0) {
	lst = 0;
    } else {
	lst = 1;
    }
    
    segread = 0;
    curindex = 0;
    sprintf(path, "%s/%s", TRK_SECT_CAM, TRK_LST_CAM);
    GfParmListSeekFirst(TrackHandle, path);
    do {
	segtype = GfParmGetCurStr(TrackHandle, path, TRK_ATT_TYPE, NULL);
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
	GfParmSetCurNum(TrackHandle, path, TRK_ATT_ID, (char*)NULL, (tdble)curindex);
	curSeg->name = GfParmListGetCurEltName(TrackHandle, path);
	//sprintf(path, "%s/%s/%s", TRK_SECT_CAM, TRK_LST_CAM, curSeg->name);
	curSeg->id = curindex;
	curSeg->width = width;
	curSeg->material = material;
	curSeg->kFriction = kFriction;
	curSeg->kRollRes = kRollRes;
	curSeg->kRoughness = kRoughness;
	curSeg->kRoughWaveLen = kRoughWaveLen;
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

	if (lsw > 0.0) {
	    AddSide(curSeg, lsw, lsmaterial, 1, lst, TrackHandle);
	}
	if (rsw > 0.0) {
	    AddSide(curSeg, rsw, rsmaterial, 0, rst, TrackHandle);
	}

	theTrack->length += curSeg->length;
	xr = newxr;
	yr = newyr;
	xl = newxl;
	yl = newyl;
	curindex++;
        //sprintf(path, "%s/%s", TRK_SECT_MAIN, TRK_LST_SEG);

    } while (GfParmListSeekNext(TrackHandle, path) == 0);

    theTrack->nseg = segread;

    /* 
     * camera definitions
     */
    sprintf(path, "%s/%s", TRK_SECT_CAM, TRK_LST_CAM);
    if (GfParmListSeekFirst(TrackHandle, path) == 0) {
	do {
	    curCam = (tRoadCam*)calloc(1, sizeof(tRoadCam));
	    if (*camList == NULL) {
		*camList = curCam;
		curCam->next = curCam;
	    } else {
		curCam->next = (*camList)->next;
		(*camList)->next = curCam;
		*camList = curCam;
	    }
	    curCam->name = GfParmListGetCurEltName(TrackHandle, path);
	    segName = GfParmGetCurStr(TrackHandle, path, TRK_ATT_SEGMENT, NULL);
	    if (segName == 0) {
		GfFatal("Bad Track Definition: in Camera %s %s is missing\n", curCam->name, TRK_ATT_SEGMENT);
	    }
	    sprintf(path2, "%s/%s/%s", TRK_SECT_MAIN, TRK_LST_SEG, segName);
	    segId = (int)GfParmGetNum(TrackHandle, path2, TRK_ATT_ID, (char*)NULL, 0);
	    curSeg = theTrack->seg;
	    for(i=0; i<theTrack->nseg; i++)  {
		if (curSeg->id == segId) {
		    break;
		}
		curSeg = curSeg->next;
	    }

	    trkPos.seg = curSeg;
	    trkPos.toRight = GfParmGetNum(TrackHandle, path2, TRK_ATT_TORIGHT, (char*)NULL, 0);
	    trkPos.toStart = GfParmGetNum(TrackHandle, path2, TRK_ATT_TOSTART, (char*)NULL, 0);
	    TrackLocal2Global(&trkPos, &(curCam->pos.x), &(curCam->pos.y));
	    curCam->pos.z = GfParmGetNum(TrackHandle, path2, TRK_ATT_HEIGHT, (char*)NULL, 0);

	    segName = GfParmGetCurStr(TrackHandle, path, TRK_ATT_CAM_FOV, NULL);
	    if (segName == 0) {
		GfFatal("Bad Track Definition: in Camera %s %s is missing\n", curCam->name, TRK_ATT_CAM_FOV);
	    }
	    sprintf(path2, "%s/%s/%s", TRK_SECT_MAIN, TRK_LST_SEG, segName);
	    segId = (int)GfParmGetNum(TrackHandle, path2, TRK_ATT_ID, (char*)NULL, 0);
	    curSeg = theTrack->seg;
	    for(i=0; i<theTrack->nseg; i++)  {
		if (curSeg->id == segId) {
		    break;
		}
		curSeg = curSeg->next;
	    }
	    segName = GfParmGetCurStr(TrackHandle, path, TRK_ATT_CAM_FOVE, NULL);
	    if (segName == 0) {
		GfFatal("Bad Track Definition: in Camera %s %s is missing\n", curCam->name, TRK_ATT_CAM_FOVE);
	    }
	    sprintf(path2, "%s/%s/%s", TRK_SECT_MAIN, TRK_LST_SEG, segName);
	    segId = (int)GfParmGetNum(TrackHandle, path2, TRK_ATT_ID, (char*)NULL, 0);
	
	    do {
		curSeg->cam = curCam;
		curSeg = curSeg->next;
	    } while (curSeg->id != segId);
	} while (GfParmListSeekNext(TrackHandle, path) == 0);
    }

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
	normSeg(curSeg);
	if (curSeg->lside) {
	    normSeg(curSeg->lside);
	}
	if (curSeg->rside) {
	    normSeg(curSeg->rside);
	}
	curSeg->next->prev = curSeg;
	curSeg = curSeg->next;
    }

    if (*camList != NULL) {
	curCam = *camList;
	do {
	    curCam = curCam->next;
	    curCam->pos.x -= xmin;
	    curCam->pos.y -= ymin;
	    curCam->pos.z -= zmin;
	} while (curCam != *camList);
    }
    
}




