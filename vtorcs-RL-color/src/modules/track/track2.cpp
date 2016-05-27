/***************************************************************************

    file                 : track2.cpp
    created              : Sun Jan 30 22:57:25 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: track2.cpp,v 1.10.6.1 2008/11/09 17:50:23 berniw Exp $

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

/*
 * Sides global variables
 */
static char *KeySideSurface[2] = {TRK_ATT_RSSURF, TRK_ATT_LSSURF};
static char *KeySideWidth[2] = {TRK_ATT_RSW, TRK_ATT_LSW};
static char *KeySideStartWidth[2] = {TRK_ATT_RSWS, TRK_ATT_LSWS};
static char *KeySideEndWidth[2] = {TRK_ATT_RSWE, TRK_ATT_LSWE};
static char *KeySideBankType[2] = {TRK_ATT_RST, TRK_ATT_LST};


static tdble sideEndWidth[2];
static tdble sideStartWidth[2];
static int sideBankType[2];
static char *sideMaterial[2];
static int envIndex;


static void
InitSides(void *TrackHandle, char *section)
{
    int side;
    
    for (side = 0; side < 2; side++) {
	sideMaterial[side]  = GfParmGetStr(TrackHandle, section, KeySideSurface[side], TRK_VAL_GRASS);
	sideEndWidth[side] = GfParmGetNum(TrackHandle, section, KeySideWidth[side], (char*)NULL, 0.0);
	/* banking of borders */
	if (strcmp(TRK_VAL_LEVEL, GfParmGetStr(TrackHandle, section, KeySideBankType[side], TRK_VAL_LEVEL)) == 0) {
	    sideBankType[side] = 0;
	} else {
	    sideBankType[side] = 1;
	}
    }
}


static void
AddSides(tTrackSeg *curSeg, void *TrackHandle, char *section, int curStep, int steps)
{
    tTrackSeg	*curSide;
    tdble	x, y, z;
    tdble	al, alfl;
    int		j;
    tdble	x1, x2, y1, y2;
    tdble	sw, ew;
    tdble	minWidth;
    tdble	maxWidth;
    int		type;
    int		side;
    char	*material;
    tdble	Kew;
    char	path[256];
    char	path2[256];

    x = y = z = 0;

    sprintf(path, "%s/%s", section, TRK_LST_SEG);
    for (side = 0; side < 2; side++) {
	if (curStep == 0) {
	    sw = GfParmGetCurNum(TrackHandle, path, KeySideStartWidth[side], (char*)NULL,sideEndWidth[side]);
	    ew = GfParmGetCurNum(TrackHandle, path, KeySideEndWidth[side], (char*)NULL, sw);
	    sideStartWidth[side] = sw;
	    sideEndWidth[side] = ew;
	} else {
	    sw = sideStartWidth[side];
	    ew = sideEndWidth[side];
	}
	Kew = (ew - sw) / (tdble)steps;
	ew = sw + (tdble)(curStep+1) * Kew;
	sw = sw + (tdble)(curStep) * Kew;
	
	if ((sw == 0.0) && (ew == 0.0)) {
	    /* no additional track side */
	    continue;
	}
    
	curSide = (tTrackSeg*)calloc(1, sizeof(tTrackSeg));
	if (side == 1) {
	    curSeg->lside = curSide;
	    curSide->vertex[TR_SR] = curSeg->vertex[TR_SL];
	    curSide->vertex[TR_ER] = curSeg->vertex[TR_EL];
	    curSide->type2 = TR_LSIDE;
	} else {
	    curSeg->rside = curSide;
	    curSide->vertex[TR_SL] = curSeg->vertex[TR_SR];
	    curSide->vertex[TR_EL] = curSeg->vertex[TR_ER];
	    curSide->type2 = TR_RSIDE;
	}

	type = sideBankType[side];
	curSide->startWidth = sw;
	curSide->endWidth = ew;
	curSide->width = minWidth = MIN(sw, ew);
	maxWidth = MAX(sw, ew);
	curSide->type = curSeg->type;
	material = GfParmGetCurStr(TrackHandle, path, KeySideSurface[side], sideMaterial[side]);
	sideMaterial[side] = curSide->material = material;
	sprintf(path2, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, material);
	curSide->kFriction = GfParmGetNum(TrackHandle, path2, TRK_ATT_FRICTION, (char*)NULL, 0.8);
	curSide->kRollRes = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROLLRES, (char*)NULL, 0.001);
	curSide->kRoughness = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHT, (char*)NULL, 0.0) /  2.0;
	curSide->kRoughWaveLen = 2.0 * PI / GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHTWL, (char*)NULL, 1.0);
	curSide->envIndex = envIndex;
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
		curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x + sw * curSeg->rgtSideNormal.x;
		curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y + sw * curSeg->rgtSideNormal.y;
		curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		x = curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x + ew * curSeg->rgtSideNormal.x;	    
		y = curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y + ew * curSeg->rgtSideNormal.y;
		z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + (tdble)type * ew * tan(curSeg->angle[TR_XE]);
		break;
	    case 0:
		curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x - sw * curSeg->rgtSideNormal.x;
		curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y - sw * curSeg->rgtSideNormal.y;
		curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		x = curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x - ew * curSeg->rgtSideNormal.x;	    
		y = curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y - ew * curSeg->rgtSideNormal.y;
		z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * ew * tan(curSeg->angle[TR_XE]);
		break;
	    }
	    curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z, curSide->length);
	    curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z, curSide->length);

	    curSide->Kzl = tan(curSide->angle[TR_YR]);
	    curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->length;
	    curSide->Kyl = (ew - sw) / curSide->length;

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
		curSide->radius = curSeg->radiusl - sw / 2.0;
		curSide->radiusr = curSeg->radiusl;
		curSide->radiusl = curSeg->radiusl - maxWidth;
		curSide->arc = curSeg->arc;
		curSide->length = curSide->radius * curSide->arc;

		curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x - sw * cos(curSide->angle[TR_CS]);
		curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y - sw * sin(curSide->angle[TR_CS]);
		curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x - ew * cos(curSide->angle[TR_CS] + curSide->arc);	    
		curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y - ew * sin(curSide->angle[TR_CS] + curSide->arc);
		z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + (tdble)type * ew * tan(curSeg->angle[TR_XE]);

		curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					      curSide->arc * curSide->radiusr);
		curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					      curSide->arc * curSide->radiusl);

		curSide->Kzl = tan(curSide->angle[TR_YR]) * curSide->radiusr;
		curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
		curSide->Kyl = (ew - sw) / curSide->arc;

		/* to find the boundary */
		al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
		alfl = curSide->angle[TR_ZS];

		for (j = 0; j < 36; j++) {
		    alfl += al;
		    x1 = curSide->center.x + (curSide->radiusl) * sin(alfl);   /* location of end */
		    y1 = curSide->center.y - (curSide->radiusl) * cos(alfl);
		    TSTX(x1);
		    TSTY(y1);
		}
		TSTZ(z);
		break;

	    case 0:
		curSide->radius = curSeg->radiusr + sw / 2.0;
		curSide->radiusl = curSeg->radiusr;
		curSide->radiusr = curSeg->radiusr + maxWidth;
		curSide->arc = curSeg->arc;
		curSide->length = curSide->radius * curSide->arc;

		curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x + sw * cos(curSide->angle[TR_CS]);
		curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y + sw * sin(curSide->angle[TR_CS]);
		curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x + ew * cos(curSide->angle[TR_CS] + curSide->arc);	    
		curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y + ew * sin(curSide->angle[TR_CS] + curSide->arc);
		z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * ew * tan(curSeg->angle[TR_XE]);

		curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					      curSide->arc * curSide->radiusr);
		curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					      curSide->arc * curSide->radiusl);

		curSide->Kzl = tan(curSide->angle[TR_YR]) * (curSide->radiusr);
		curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
		curSide->Kyl = (ew - sw) / curSide->arc;

		/* to find the boundary */
		al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
		alfl = curSide->angle[TR_ZS];

		for (j = 0; j < 36; j++) {
		    alfl += al;
		    x2 = curSide->center.x + (curSide->radiusr) * sin(alfl);   /* location of end */
		    y2 = curSide->center.y - (curSide->radiusr) * cos(alfl);
		    TSTX(x2);
		    TSTY(y2);
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
		curSide->radius = curSeg->radiusl + sw / 2.0;
		curSide->radiusr = curSeg->radiusl;
		curSide->radiusl = curSeg->radiusl + maxWidth;
		curSide->arc = curSeg->arc;
		curSide->length = curSide->radius * curSide->arc;

		curSide->vertex[TR_SL].x = curSide->vertex[TR_SR].x + sw * cos(curSide->angle[TR_CS]);
		curSide->vertex[TR_SL].y = curSide->vertex[TR_SR].y + sw * sin(curSide->angle[TR_CS]);
		curSide->vertex[TR_SL].z = curSide->vertex[TR_SR].z + (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		curSide->vertex[TR_EL].x = curSide->vertex[TR_ER].x + ew * cos(curSide->angle[TR_CS] - curSide->arc);	    
		curSide->vertex[TR_EL].y = curSide->vertex[TR_ER].y + ew * sin(curSide->angle[TR_CS] - curSide->arc);
		z = curSide->vertex[TR_EL].z = curSide->vertex[TR_ER].z + (tdble)type * ew * tan(curSeg->angle[TR_XE]);

		curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					      curSide->arc * curSide->radiusr);
		curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					      curSide->arc * curSide->radiusl);

		curSide->Kzl = tan(curSide->angle[TR_YR]) * curSide->radiusr;
		curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
		curSide->Kyl = (ew - sw) / curSide->arc;

		/* to find the boundary */
		al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
		alfl = curSide->angle[TR_ZS];

		for (j = 0; j < 36; j++) {
		    alfl += al;
		    x1 = curSide->center.x - (curSide->radiusl) * sin(alfl);   /* location of end */
		    y1 = curSide->center.y + (curSide->radiusl) * cos(alfl);
		    TSTX(x1);
		    TSTY(y1);
		}
		TSTZ(z);
		break;

	    case 0:
		curSide->radius = curSeg->radiusr - sw / 2.0;
		curSide->radiusl = curSeg->radiusr;
		curSide->radiusr = curSeg->radiusr - maxWidth;
		curSide->arc = curSeg->arc;
		curSide->length = curSide->radius * curSide->arc;

		curSide->vertex[TR_SR].x = curSide->vertex[TR_SL].x - sw * cos(curSide->angle[TR_CS]);
		curSide->vertex[TR_SR].y = curSide->vertex[TR_SL].y - sw * sin(curSide->angle[TR_CS]);
		curSide->vertex[TR_SR].z = curSide->vertex[TR_SL].z - (tdble)type * sw * tan(curSeg->angle[TR_XS]);
		curSide->vertex[TR_ER].x = curSide->vertex[TR_EL].x - ew * cos(curSide->angle[TR_CS] - curSide->arc);	    
		curSide->vertex[TR_ER].y = curSide->vertex[TR_EL].y - ew * sin(curSide->angle[TR_CS] - curSide->arc);
		z = curSide->vertex[TR_ER].z = curSide->vertex[TR_EL].z - (tdble)type * ew * tan(curSeg->angle[TR_XE]);

		curSide->angle[TR_YR] = atan2(curSide->vertex[TR_ER].z - curSide->vertex[TR_SR].z,
					      curSide->arc * curSide->radiusr);
		curSide->angle[TR_YL] = atan2(curSide->vertex[TR_EL].z - curSide->vertex[TR_SL].z,
					      curSide->arc * curSide->radiusl);

		curSide->Kzl = tan(curSide->angle[TR_YR]) * (curSide->radiusr);
		curSide->Kzw = (curSide->angle[TR_XE] - curSide->angle[TR_XS]) / curSide->arc;
		curSide->Kyl = (ew - sw) / curSide->arc;

		/* to find the boundary */
		al = (curSide->angle[TR_ZE] - curSide->angle[TR_ZS])/36.0;
		alfl = curSide->angle[TR_ZS];

		for (j = 0; j < 36; j++) {
		    alfl += al;
		    x2 = curSide->center.x - (curSide->radiusr) * sin(alfl);   /* location of end */
		    y2 = curSide->center.y - (curSide->radiusr) * cos(alfl);
		    TSTX(x2);
		    TSTY(y2);
		}
		TSTZ(z);
		break;
	    }
	    break;
	}
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

static void
CreateSegRing(void *TrackHandle, char *section, tTrackSeg **pRoot, tdble *pLength, int *pNseg, tTrackSeg *start, tTrackSeg *end, int ext)
{
    int		j;
    int		segread, curindex;
    tdble	radius;
    tdble	innerradius;
    tdble	arc;
    tdble	length;
    tTrackSeg	*curSeg;
    tTrackSeg	*root;
    tdble	alf;
    tdble	xr, yr, newxr, newyr;
    tdble	xl, yl, newxl, newyl;
    tdble	cenx, ceny;
    tdble	width, wi2;
    tdble	x1, x2, y1, y2;
    tdble	al, alfl;
    tdble	zsl, zsr, zel, zer, zs, ze;
    tdble	bankings, bankinge, dz, dzl, dzr;
    tdble	etgt, stgt;
    tdble	etgtl, stgtl;
    tdble	etgtr, stgtr;
    int		steps, curStep;
    char        *segtype = (char*)NULL;
    char	*material;
    char	*segName;
    int		type;
    tdble	kFriction, kRollRes;
    tdble	kRoughness, kRoughWaveLenP, kRoughWaveLen;
    char	*profil;
    tdble	totLength;

    tdble	tl, dtl, T1l, T2l;
    tdble	tr, dtr, T1r, T2r;
    tdble	curzel, curzer, curArc, curLength, curzsl, curzsr;
    tdble	grade;

    char	path[256];
    char	path2[256];
#define MAX_TMP_INTS	256
    int		mi[MAX_TMP_INTS];
    int		ind = 0;

    radius = arc = length = alf = xr = yr = newxr = newyr = xl = yl = 0;
    zel = zer = etgtl = etgtr = newxl = newyl = 0;
    type = 0;
    
    width = GfParmGetNum(TrackHandle, section, TRK_ATT_WIDTH, (char*)NULL, 15.0);
    wi2 = width / 2.0;

    grade = -100000.0;
    root = (tTrackSeg*)NULL;
    totLength = 0;
    
    sprintf(path, "%s/%s", section, TRK_LST_SEG);
    if (start == NULL) {
	xr = xl = 0.0;
	yr = 0.0;
	yl = width;
	alf = 0.0;
	zsl = zsr = zel = zer = zs = ze = 0.0;
	stgt = etgt = 0.0;
	stgtl = etgtl = 0.0;
	stgtr = etgtr = 0.0;
    } else {
	GfParmListSeekFirst(TrackHandle, path);
	segtype = GfParmGetCurStr(TrackHandle, path, TRK_ATT_TYPE, "");
	if (strcmp(segtype, TRK_VAL_STR) == 0) {
	} else if (strcmp(segtype, TRK_VAL_LFT) == 0) {
	} else if (strcmp(segtype, TRK_VAL_RGT) == 0) {
	    xr = start->vertex[TR_SR].x;
	    yr = start->vertex[TR_SR].y;
	    zsl = zsr = zel = zer = zs = ze = start->vertex[TR_SR].z;
	    alf = start->angle[TR_ZS];
	    xl = xr - width * sin(alf);
	    yl = yr + width * cos(alf);
	    stgt = etgt = 0.0;
	    stgtl = etgtl = 0.0;
	    stgtr = etgtr = 0.0;	    
	}
    }
    

    /* Main Track */
    material = GfParmGetStr(TrackHandle, section, TRK_ATT_SURF, TRK_VAL_ASPHALT);
    sprintf(path2, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, material);
    kFriction = GfParmGetNum(TrackHandle, path2, TRK_ATT_FRICTION, (char*)NULL, 0.8);
    kRollRes = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROLLRES, (char*)NULL, 0.001);
    kRoughness = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHT, (char*)NULL, 0.0) / 2.0;
    kRoughWaveLenP = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHTWL, (char*)NULL, 1.0);
    kRoughWaveLen = 2.0 * PI / kRoughWaveLenP;
    envIndex = 0;

    InitSides(TrackHandle, section);
    
    segread = 0;
    curindex = 0;
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
	
	/* Turn Marks */
	if (ext) {
	    char *marks = GfParmGetCurStr(TrackHandle, path, TRK_ATT_MARKS, NULL);
	    ind = 0;
	    if (marks) {
		marks = strdup(marks);
		char *s = strtok(marks, ";");
		while ((s != NULL) && (ind < MAX_TMP_INTS)) {
		    mi[ind] = (int)strtol(s, NULL, 0);
		    ind++;
		    s = strtok(NULL, ";");
		}
		free(marks);
	    }
	}
	
	/* surface change */
	material = GfParmGetCurStr(TrackHandle, path, TRK_ATT_SURF, material);
	sprintf(path2, "%s/%s/%s", TRK_SECT_SURFACES, TRK_LST_SURF, material);
	kFriction = GfParmGetNum(TrackHandle, path2, TRK_ATT_FRICTION, (char*)NULL, kFriction);
	kRollRes = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROLLRES, (char*)NULL, kRollRes);
	kRoughness = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHT, (char*)NULL, kRoughness * 2.0) / 2.0;
	kRoughWaveLenP = GfParmGetNum(TrackHandle, path2, TRK_ATT_ROUGHTWL, (char*)NULL, kRoughWaveLenP);
	kRoughWaveLen = 2.0 * PI / kRoughWaveLenP;
	envIndex = (int)GfParmGetCurNum(TrackHandle, path, TRK_ATT_ENVIND, (char*)NULL, envIndex+1) - 1;

	/* get segment type and lenght */
	if (strcmp(segtype, TRK_VAL_STR) == 0) {
	    /* straight */
	    length = GfParmGetCurNum(TrackHandle, path, TRK_ATT_LG, (char*)NULL, 0);
	    type = TR_STR;
	} else if (strcmp(segtype, TRK_VAL_LFT) == 0) {
	    /* left curve */
	    radius = GfParmGetCurNum(TrackHandle, path, TRK_ATT_RADIUS, (char*)NULL, 0);
	    arc = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ARC, (char*)NULL, 0);
	    type = TR_LFT;
	    length = radius * arc;
	} else if (strcmp(segtype, TRK_VAL_RGT) == 0) {
	    /* right curve */
	    radius = GfParmGetCurNum(TrackHandle, path, TRK_ATT_RADIUS, (char*)NULL, 0);
	    arc = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ARC, (char*)NULL, 0);
	    type = TR_RGT;
	    length = radius * arc;
	}
	segName = GfParmListGetCurEltName(TrackHandle, path);

	/* elevation and banking */
	zsl = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZSL, (char*)NULL, zsl);
	zsr = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZSR, (char*)NULL, zsr);
	zel = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZEL, (char*)NULL, zel);
	zer = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZER, (char*)NULL, zer);
	ze = zs = -100000.0;
	ze = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZE, (char*)NULL, ze);
	zs = GfParmGetCurNum(TrackHandle, path, TRK_ATT_ZS, (char*)NULL, zs);
	grade = GfParmGetCurNum(TrackHandle, path, TRK_ATT_GRADE, (char*)NULL, grade);
	if (zs != -100000.0) {
	    zsr = zsl = zs;
	} else {
	    zs = (zsl + zsr) / 2.0;
	}
	if (ze != -100000.0) {
	    zer = zel = ze;
	} else if (grade != -100000.0) {
	    ze = zs + length * grade;
	} else {
	    ze = (zel + zer) / 2.0;
	}
	bankings = atan2(zsl - zsr, width);
	bankinge = atan2(zel - zer, width);
	bankings = GfParmGetCurNum(TrackHandle, path, TRK_ATT_BKS, (char*)NULL, bankings);
	bankinge = GfParmGetCurNum(TrackHandle, path, TRK_ATT_BKE, (char*)NULL, bankinge);
	dz = tan(bankings) * width / 2.0;
	zsl = zs + dz;
	zsr = zs - dz;
	dz = tan(bankinge) * width / 2.0;
	zel = ze + dz;
	zer = ze - dz;

	TSTZ(zsl);
	TSTZ(zsr);

	/* Get segment profil */
	profil = GfParmGetCurStr(TrackHandle, path, TRK_ATT_PROFIL, TRK_VAL_LINEAR);
	stgtl = etgtl;
	stgtr = etgtr;
	if (strcmp(profil, TRK_VAL_SPLINE) == 0) {
	    steps = (int)GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFSTEPS, (char*)NULL, 1.0);
	    stgtl = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTSL, (char*)NULL, stgtl);
	    etgtl = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTEL, (char*)NULL, etgtl);
	    stgtr = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTSR, (char*)NULL, stgtr);
	    etgtr = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTER, (char*)NULL, etgtr);
	    stgt = etgt = -100000.0;
	    stgt = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTS, (char*)NULL, stgt);
	    etgt = GfParmGetCurNum(TrackHandle, path, TRK_ATT_PROFTGTE, (char*)NULL, etgt);
	    if (stgt != -100000.0) {
		stgtl = stgtr = stgt;
	    }
	    if (etgt != -100000.0) {
		etgtl = etgtr = etgt;
	    }
	} else {
	    steps = 1;
	    stgtl = etgtl = (zel - zsl) / length;
	    stgtr = etgtr = (zer - zsr) / length;
	}
	GfParmSetCurNum(TrackHandle, path, TRK_ATT_ID, (char*)NULL, (tdble)curindex);
	
	dzl = zel - zsl;
	dzr = zer - zsr;
	T1l = stgtl * length;
	T2l = etgtl * length;
	tl = 0.0;
	dtl = 1.0 / (tdble)steps;
	T1r = stgtr * length;
	T2r = etgtr * length;
	tr = 0.0;
	dtr = 1.0 / (tdble)steps;

	curStep = 0;
	curzel = zsl;
	curzer = zsr;
	curArc = arc / (tdble)steps;
	curLength = length / (tdble)steps;

	while (curStep < steps) {
	    
	    tl += dtl;
	    tr += dtr;

	    curzsl = curzel;
	    curzel = TrackSpline(zsl, zel, T1l, T2l, tl);
	    
	    curzsr = curzer;
	    curzer = TrackSpline(zsr, zer, T1r, T2r, tr);
	    
	    
	    /* allocate a new segment */
	    curSeg = (tTrackSeg*)calloc(1, sizeof(tTrackSeg));
	    if (root == NULL) {
		root = curSeg;
		curSeg->next = curSeg;
		curSeg->prev = curSeg;
	    } else {
		curSeg->next = root->next;
		curSeg->next->prev = curSeg;
		curSeg->prev = root;
		root->next = curSeg;
		root = curSeg;
	    }
	    curSeg->type2 = TR_MAIN;
	    curSeg->name = segName;
	    curSeg->id = curindex;
	    curSeg->width = curSeg->startWidth = curSeg->endWidth = width;
	    curSeg->material = material;
	    curSeg->kFriction = kFriction;
	    curSeg->kRollRes = kRollRes;
	    curSeg->kRoughness = kRoughness;
	    curSeg->kRoughWaveLen = kRoughWaveLen;
	    curSeg->envIndex = envIndex;
	    curSeg->lgfromstart = totLength;
	    
	    if (ext && ind) {
		int	*mrks = (int*)calloc(ind, sizeof(int));
		tSegExt	*segExt = (tSegExt*)calloc(1, sizeof(tSegExt));

		memcpy(mrks, mi, ind*sizeof(int));
		segExt->nbMarks = ind;
		segExt->marks = mrks;
		curSeg->ext = segExt;
		ind = 0;
	    }
		

	    switch (type) {
	    case TR_STR:
		/* straight */
		curSeg->type = TR_STR;
		curSeg->length = curLength;

		newxr = xr + curLength * cos(alf);      /* find end coordinates */
		newyr = yr + curLength * sin(alf);
		newxl = xl + curLength * cos(alf);
		newyl = yl + curLength * sin(alf);

		curSeg->vertex[TR_SR].x = xr;
		curSeg->vertex[TR_SR].y = yr;
		curSeg->vertex[TR_SR].z = curzsr;

		curSeg->vertex[TR_SL].x = xl;
		curSeg->vertex[TR_SL].y = yl;
		curSeg->vertex[TR_SL].z = curzsl;

		curSeg->vertex[TR_ER].x = newxr;
		curSeg->vertex[TR_ER].y = newyr;
		curSeg->vertex[TR_ER].z = curzer;

		curSeg->vertex[TR_EL].x = newxl;
		curSeg->vertex[TR_EL].y = newyl;
		curSeg->vertex[TR_EL].z = curzel;

		curSeg->angle[TR_ZS] = alf;
		curSeg->angle[TR_ZE] = alf;
		curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, curLength);
		curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, curLength);
		curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
		curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);
	    
		curSeg->Kzl = tan(curSeg->angle[TR_YR]);
		curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / curLength;
		curSeg->Kyl = 0;

		curSeg->rgtSideNormal.x = -sin(alf);
		curSeg->rgtSideNormal.y = cos(alf);

		TSTX(newxr); TSTX(newxl);
		TSTY(newyr); TSTY(newyl);

		break;
	    
	    case TR_LFT:
		/* left curve */
		curSeg->type = TR_LFT;
		curSeg->radius = radius;
		curSeg->radiusr = radius + wi2;
		curSeg->radiusl = radius - wi2;
		curSeg->arc = curArc;
		curSeg->length = curLength;
	    
		innerradius = radius - wi2; /* left side aligned */
		cenx = xl - innerradius * sin(alf);  /* compute center location: */
		ceny = yl + innerradius * cos(alf);
		curSeg->center.x = cenx;
		curSeg->center.y = ceny;

		curSeg->angle[TR_ZS] = alf;
		curSeg->angle[TR_CS] = alf - PI / 2.0;
		alf += curArc;
		curSeg->angle[TR_ZE] = alf;

		newxl = cenx + innerradius * sin(alf);   /* location of end */
		newyl = ceny - innerradius * cos(alf);
		newxr = cenx + (innerradius + width) * sin(alf);   /* location of end */
		newyr = ceny - (innerradius + width) * cos(alf);

		curSeg->vertex[TR_SR].x = xr;
		curSeg->vertex[TR_SR].y = yr;
		curSeg->vertex[TR_SR].z = curzsr;

		curSeg->vertex[TR_SL].x = xl;
		curSeg->vertex[TR_SL].y = yl;
		curSeg->vertex[TR_SL].z = curzsl;

		curSeg->vertex[TR_ER].x = newxr;
		curSeg->vertex[TR_ER].y = newyr;
		curSeg->vertex[TR_ER].z = curzer;

		curSeg->vertex[TR_EL].x = newxl;
		curSeg->vertex[TR_EL].y = newyl;
		curSeg->vertex[TR_EL].z = curzel;

		curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, curArc * (innerradius + width));
		curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, curArc * innerradius);
		curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
		curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);

		curSeg->Kzl = tan(curSeg->angle[TR_YR]) * (innerradius + width);
		curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / curArc;
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

		break;
	    
	    case TR_RGT:
		/* right curve */
		curSeg->type = TR_RGT;
		curSeg->radius = radius;
		curSeg->radiusr = radius - wi2;
		curSeg->radiusl = radius + wi2;
		curSeg->arc = curArc;
		curSeg->length = curLength;

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
		curSeg->vertex[TR_SR].z = curzsr;

		curSeg->vertex[TR_SL].x = xl;
		curSeg->vertex[TR_SL].y = yl;
		curSeg->vertex[TR_SL].z = curzsl;

		curSeg->vertex[TR_ER].x = newxr;
		curSeg->vertex[TR_ER].y = newyr;
		curSeg->vertex[TR_ER].z = curzer;

		curSeg->vertex[TR_EL].x = newxl;
		curSeg->vertex[TR_EL].y = newyl;
		curSeg->vertex[TR_EL].z = curzel;

		curSeg->angle[TR_YR] = atan2(curSeg->vertex[TR_ER].z - curSeg->vertex[TR_SR].z, curArc * innerradius);
		curSeg->angle[TR_YL] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_SL].z, curArc * (innerradius + width));
		curSeg->angle[TR_XS] = atan2(curSeg->vertex[TR_SL].z - curSeg->vertex[TR_SR].z, width);
		curSeg->angle[TR_XE] = atan2(curSeg->vertex[TR_EL].z - curSeg->vertex[TR_ER].z, width);

		curSeg->Kzl = tan(curSeg->angle[TR_YR]) * innerradius;
		curSeg->Kzw = (curSeg->angle[TR_XE] - curSeg->angle[TR_XS]) / curArc;
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
		break;

	    }

	    AddSides(curSeg, TrackHandle, section, curStep, steps);

	    totLength += curSeg->length;
	    xr = newxr;
	    yr = newyr;
	    xl = newxl;
	    yl = newyl;
	    curindex++;
	    curStep++;
	}

    } while (GfParmListSeekNext(TrackHandle, path) == 0);

    *pRoot = root;
    *pLength = totLength;
    *pNseg = curindex;
}




/*
 * Read version 2 track segments
 */
void 
ReadTrack2(tTrack *theTrack, void *TrackHandle, tRoadCam **camList, int ext)
{
    int		i;
    tTrackSeg	*curSeg = NULL, *mSeg;
    tTrackSeg	*pitEntrySeg = NULL;
    tTrackSeg	*pitExitSeg = NULL;
    tTrackSeg	*pitStart = NULL;
    tTrackSeg	*pitEnd = NULL;
    char	*segName;
    int		segId;
    tRoadCam	*curCam;
    tTrkLocPos	trkPos;
    int		found = 0;
    char	*paramVal;
    char	*pitType;
    char	path[256];
    char	path2[256];

    xmin = xmax = ymin = ymax = zmin = zmax = 0.0;
    
    CreateSegRing(TrackHandle, TRK_SECT_MAIN, &(theTrack->seg), &(theTrack->length), &(theTrack->nseg), (tTrackSeg*)NULL, (tTrackSeg*)NULL, ext);

    pitType = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_TYPE, TRK_VAL_PIT_TYPE_NONE);
    
    if (strcmp(pitType, TRK_VAL_PIT_TYPE_NONE) != 0) {
	segName = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_ENTRY, NULL);
	if (segName != 0) {
	    pitEntrySeg = theTrack->seg;
	    found = 0;
	    for(i = 0; i < theTrack->nseg; i++)  {
		if (!strcmp(segName, pitEntrySeg->name)) {
		    found = 1;
		} else if (found) {
		    pitEntrySeg = pitEntrySeg->next;
		    break;
		}
		pitEntrySeg = pitEntrySeg->prev;
	    }
	    if (!found) {
		pitEntrySeg = NULL;
	    }
	}
	segName = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_EXIT, NULL);
	if (segName != 0) {
	    pitExitSeg = theTrack->seg->next;
	    found = 0;
	    for(i = 0; i < theTrack->nseg; i++)  {
		if (!strcmp(segName, pitExitSeg->name)) {
		    found = 1;
		} else if (found) {
		    pitExitSeg = pitExitSeg->prev;
		    break;
		}
		pitExitSeg = pitExitSeg->next;
	    }
	    if (!found) {
		pitExitSeg = NULL;
	    }
	}
	segName = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_START, NULL);
	if (segName != 0) {
	    pitStart = theTrack->seg;
	    found = 0;
	    for(i = 0; i < theTrack->nseg; i++)  {
		if (!strcmp(segName, pitStart->name)) {
		    found = 1;
		} else if (found) {
		    pitStart = pitStart->next;
		    break;
		}
		pitStart = pitStart->prev;
	    }
	    if (!found) {
		pitStart = NULL;
	    }
	}
	segName = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_END, NULL);
	if (segName != 0) {
	    pitEnd = theTrack->seg->next;
	    found = 0;
	    for(i = 0; i < theTrack->nseg; i++)  {
		if (!strcmp(segName, pitEnd->name)) {
		    found = 1;
		} else if (found) {
		    pitEnd = pitEnd->prev;
		    break;
		}
		pitEnd = pitEnd->next;
	    }
	    if (!found) {
		pitEnd = NULL;
	    }
	}
	paramVal = GfParmGetStr(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_SIDE, "right");
	if (strcmp(paramVal, "right") == 0) {
	    theTrack->pits.side = TR_RGT;
	} else {
	    theTrack->pits.side = TR_LFT;
	}
	if ((pitEntrySeg != NULL) && (pitExitSeg != NULL) && (pitStart != NULL) && (pitEnd != NULL)) {
	    theTrack->pits.pitEntry = pitEntrySeg;
	    theTrack->pits.pitExit  = pitExitSeg;
	    theTrack->pits.pitStart = pitStart;
	    theTrack->pits.pitEnd = pitEnd;
	    pitEntrySeg->raceInfo |= TR_PITENTRY;
	    pitExitSeg->raceInfo |= TR_PITEXIT;
	    theTrack->pits.len   = GfParmGetNum(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_LEN, (char*)NULL, 15.0);
	    theTrack->pits.width = GfParmGetNum(TrackHandle, TRK_SECT_MAIN, TRK_ATT_PIT_WIDTH, (char*)NULL, 5.0);
	    found = 1;
	} else {
	    found = 0;
	}
    }

    if (found && (strcmp(pitType, TRK_VAL_PIT_TYPE_SIDE) == 0)) {
	theTrack->pits.type     = TR_PIT_ON_TRACK_SIDE;
	theTrack->pits.nPitSeg  = 0;
	if (pitStart->lgfromstart > pitEnd->lgfromstart) {
	    theTrack->pits.nMaxPits = (int)((theTrack->length - pitStart->lgfromstart + pitEnd->lgfromstart +
					     pitEnd->length + theTrack->pits.len / 2.0) / theTrack->pits.len);
	} else {
	    theTrack->pits.nMaxPits = (int)((- pitStart->lgfromstart + pitEnd->lgfromstart +
					     pitEnd->length + theTrack->pits.len / 2.0) / theTrack->pits.len);
	}
	for (mSeg = pitStart; mSeg != pitEnd->next; mSeg = mSeg->next) {
	    switch(theTrack->pits.side) {
	    case TR_RGT:
		curSeg = mSeg->rside;
		break;
	    case TR_LFT:
		curSeg = mSeg->lside;
		break;
	    }
	    curSeg->raceInfo |= TR_PIT;
	}
	    
    }

#ifdef EEE
    if (found && (strcmp(pitType, TRK_VAL_PIT_TYPE_SEP_PATH) == 0)) {
	tTrackSeg	*pitSeg = NULL;
	tdble		pitLen;
	int		pitNseg;

	theTrack->pits.type = TR_PIT_ON_SEPARATE_PATH;
	CreateSegRing(TrackHandle, TRK_SECT_PITS, &pitSeg, &pitLen, &pitNseg, pitEntrySeg, pitExitSeg->next);
	theTrack->pits.nPitSeg = pitNseg;
	pitSeg->next->raceInfo |= TR_PITENTRY;
	pitSeg->raceInfo |= TR_PITEXIT;

	segName = GfParmGetStr(TrackHandle, TRK_SECT_PITS, TRK_ATT_FINISH, NULL);
	if (segName != 0) {
	    sprintf(path, "%s/%s/%s", TRK_SECT_MAIN, TRK_LST_SEG, segName);
	    segId = (int)GfParmGetNum(TrackHandle, path, TRK_ATT_ID, (char*)NULL, 0);
	    curSeg = pitSeg->next;
	    found = 0;
	    for (i = 0; i < pitNseg; i++) {
		if (curSeg->id == segId) {
		    found = 1;
		    break;
		}
		curSeg = curSeg->next;
	    }
	    if (found) {
		curSeg->raceInfo |= TR_LAST;
		curSeg->next->raceInfo |= TR_START;
	    }
	}

	switch(pitSeg->next->type) {
	case TR_RGT:
	    pitEntrySeg->ralt = pitSeg->next;
	    pitSeg->next->lalt = pitEntrySeg;
	    break;
	case TR_LFT:
	    pitEntrySeg->lalt = pitSeg->next;
	    pitSeg->next->ralt = pitEntrySeg;
	    break;
	case TR_STR:
	    switch(pitEntrySeg->type) {
	    case TR_RGT:
		pitEntrySeg->lalt = pitSeg->next;
		pitSeg->next->ralt = pitEntrySeg;
		break;
	    case TR_LFT:
		pitEntrySeg->ralt = pitSeg->next;
		pitSeg->next->lalt = pitEntrySeg;
		break;
	    }
	    break;
	}
	switch(pitSeg->type) {
	case TR_RGT:
	    pitExitSeg->ralt = pitSeg;
	    pitSeg->lalt = pitExitSeg;
	    break;
	case TR_LFT:
	    pitExitSeg->lalt = pitSeg;
	    pitSeg->ralt = pitExitSeg;
	    break;
	case TR_STR:
	    switch(pitExitSeg->type) {
	    case TR_RGT:
		pitExitSeg->lalt = pitSeg;
		pitSeg->ralt = pitExitSeg;
		break;
	    case TR_LFT:
		pitExitSeg->ralt = pitSeg;
		pitSeg->lalt = pitExitSeg;
		break;
	    }
	    break;
	}
	pitSeg->next = pitExitSeg;
    }
#endif
    

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
	    trkPos.toRight = GfParmGetCurNum(TrackHandle, path, TRK_ATT_TORIGHT, (char*)NULL, 0);
	    trkPos.toStart = GfParmGetCurNum(TrackHandle, path, TRK_ATT_TOSTART, (char*)NULL, 0);
	    TrackLocal2Global(&trkPos, &(curCam->pos.x), &(curCam->pos.y));
	    curCam->pos.z = TrackHeightL(&trkPos) + GfParmGetCurNum(TrackHandle, path, TRK_ATT_HEIGHT, (char*)NULL, 0);

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

    if (theTrack->pits.type == TR_PIT_ON_SEPARATE_PATH) {
	curSeg = theTrack->pits.pitEntry;
	for(i = 0; i < theTrack->pits.nPitSeg; i++)  {         /* read the segment data: */
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
    }
    
    curSeg = theTrack->seg;
    for(i=0; i<theTrack->nseg; i++)  {         /* read the segment data: */
	if (curSeg->lgfromstart > (theTrack->length - 50.0)) {
	    curSeg->raceInfo |= TR_LAST;
	} else if (curSeg->lgfromstart < 50.0) {
	    curSeg->raceInfo |= TR_START;
	} else {
	    curSeg->raceInfo |= TR_NORMAL;
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




