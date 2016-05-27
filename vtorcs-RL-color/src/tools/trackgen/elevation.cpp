/***************************************************************************

	file                 : elevation.cpp
	created              : Mon May 20 22:31:09 CEST 2002
	copyright            : (C) 2001 by Eric Espie
	email                : eric.espie@torcs.org
	version              : $Id: elevation.cpp,v 1.9.2.2 2008/11/09 17:50:23 berniw Exp $

***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

/** @file   
			
	@author	<a href=mailto:eric.espie@torcs.org>Eric Espie</a>
	@version	$Id: elevation.cpp,v 1.9.2.2 2008/11/09 17:50:23 berniw Exp $
 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <math.h>

#include <tgfclient.h>
#include <track.h>

#include "trackgen.h"
#include "util.h"
#include "elevation.h"

static unsigned char	*ElvImage;
static int	ElvOk = 0;
static tdble 	Margin;

static tdble	kX, kY, dX, dY;
static tdble	kZ, dZ;
static int	width, height;

#define MAX_CLR	255.0

void LoadElevation(tTrack *track, void *TrackHandle, char *imgFile)
{
	tdble zmin, zmax;
	tdble xmin, xmax, ymin, ymax;

	ElvImage = GfImgReadPng(imgFile, &width, &height, 2.0);
	if (!ElvImage) {
		return;
	}

	printf("Loading Elevation Map %s\n", imgFile);

	Margin = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BMARGIN, NULL, Margin);

	xmin = track->min.x - Margin;
	xmax = track->max.x + Margin;
	ymin = track->min.y - Margin;
	ymax = track->max.y + Margin;

	kX = (tdble)(width - 1) / (xmax - xmin);
	dX = -xmin * kX;
	kY = (tdble)(height - 1) / (ymax - ymin);
	dY = -ymin * kY;
	ElvOk = 1;

	zmin = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_ALT_MIN, NULL, track->min.z);
	zmax = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_ALT_MAX, NULL, track->max.z);

	dZ = zmin;
	kZ = (zmax - dZ) / MAX_CLR;
}


tdble GetElevation(tdble x, tdble y, tdble z)
{
	int iX, iY;
	int clr;

	if (ElvOk) {
		iX = (int)(x * kX + dX);
		iY = (int)(y * kY + dY);
		/* RGBA */
		clr = ElvImage[4 * (iY * width + iX)];
		return (tdble)clr * kZ + dZ;
	}

	return z;
}


void SaveElevation(tTrack *track, void *TrackHandle, char *imgFile, char *meshFile, int dispf)
{
	ssgLoaderOptionsEx options;
	float zmin, zmax;
	float xmin, xmax, ymin, ymax;
	float x, y, z;
	int clr;
	int i, j, k, l;
	ssgRoot	*root;
	int columns;
	static char	buf[1024];
	char *s;
	float heightStep;

	s = getenv("COLUMNS");
	if (s) {
		columns = strtol(getenv("COLUMNS"), NULL, 0);
	} else {
		columns = 80;
	}

	Margin = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_BMARGIN, NULL, Margin);

	xmin = track->min.x - Margin;
	xmax = track->max.x + Margin;
	ymin = track->min.y - Margin;
	ymax = track->max.y + Margin;

	width = 1024;
	height = (int)((ymax - ymin) * width / (xmax - xmin));

	printf("Generating Elevation Map %s (%d, %d)\n", imgFile, width, height);
	kX = (xmax - xmin) / width;
	dX = xmin;
	kY = (ymax - ymin) / height;
	dY = ymin;

	zmin = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_ALT_MIN, NULL, track->min.z);
	zmax = GfParmGetNum(TrackHandle, TRK_SECT_TERRAIN, TRK_ATT_ALT_MAX, NULL, track->max.z);

	heightStep = (float)(zmax - zmin) / (float)HeightSteps;
	if (dispf == 2) {
		printf("Height of steps = %f\n", heightStep);
	}

	kZ = MAX_CLR / (zmax - zmin);
	dZ = - zmin * MAX_CLR / (zmax - zmin);

	ElvImage = (unsigned char*)calloc(width * height, 3);
	if (!ElvImage) {
		return;
	}

	ssgSetCurrentOptions(&options);
	sprintf(buf, "tracks/%s/%s;data/textures;data/img;.", track->category, track->internalname);
	ssgTexturePath(buf);
	sprintf(buf, ".;tracks/%s/%s", track->category, track->internalname);
	ssgModelPath(buf);
	root = (ssgRoot*)ssgLoadAC(meshFile);

	if (root == NULL) {
		printf("Could not load %s, ", meshFile);
		printf("please generate it with \"trackgen -c %s -n %s -a\"\n", track->category, track->internalname);
		return;
	}

	l = columns - 18;
	for (j = 0; j < height; j++) {
		s = buf;
		s += sprintf(buf, "%4d%% |", (j+1) * 100 / height);
		for (k = s - buf; k < s - buf + l; k++) {
			if ((k - (s - buf)) > (l * (j+1) / height)) {
				buf[k] = ' ';
			} else {
				buf[k] = '*';
			}
		}
		s += l;
		sprintf(s, "| row %4d", j+1);
		printf("\r%s", buf);
		fflush(stdout);
		for (i = 0; i < width; i++) {
			x = i * kX + dX;
			y = j * kY + dY;
			z = getHOT(root, x, y);
			if (z != -1000000.0f) {
				switch (dispf) {
					case 0:
						clr = 0;
						break;
					case 1:
						clr = (int)(z * kZ + dZ);
						break;
					case 2:
						clr = (int)(floor((z + heightStep / 2.0) / heightStep) * heightStep * kZ + dZ);
						break;
					default:
						clr = 0;
						break;
				}
			} else {
				clr = (int)MAX_CLR;
			}

			ElvImage[3 * (i + width * j)]     = (unsigned char)clr;
			ElvImage[3 * (i + width * j) + 1] = (unsigned char)clr;
			ElvImage[3 * (i + width * j) + 2] = (unsigned char)clr;
		}
	}

	printf("\n");
	GfImgWritePng(ElvImage, imgFile, width, height);
}
