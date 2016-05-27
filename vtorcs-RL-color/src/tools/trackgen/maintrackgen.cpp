/***************************************************************************

    file                 : trackgen.cpp
    created              : Sat Dec 23 09:27:43 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: maintrackgen.cpp,v 1.5.2.2 2008/11/09 17:50:23 berniw Exp $

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
    		
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: maintrackgen.cpp,v 1.5.2.2 2008/11/09 17:50:23 berniw Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#ifndef WIN32
#include <unistd.h>
#endif
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef WIN32
#include <getopt.h>
#endif
#include <math.h>
#include <plib/ul.h>
#include <plib/ssg.h>
#include <GL/glut.h>

#include <tgfclient.h>
#include <track.h>

#include "ac3d.h"
#include "easymesh.h"
#include "objects.h"
#include "elevation.h"
#include "trackgen.h"

float	GridStep = 40.0;
float	TrackStep = 5.0;
float	Margin = 100.0;
float	ExtHeight = 5.0;

int	HeightSteps = 30;

int	bump = 0;
int	UseBorder = 1;

char		*OutputFileName;
char		*TrackName;
char		*TrackCategory;

void		*TrackHandle;
void		*CfgHandle;

tTrack		*Track;
tTrackItf	TrackItf;

int		TrackOnly;
int		JustCalculate;
int		MergeAll;
int		MergeTerrain;

static char	buf[1024];
static char	buf2[1024];
static char	trackdef[1024];

char		*OutTrackName;
char		*OutMeshName;

tModList	*modlist = NULL;

int		saveElevation;
char		*ElevationFile;

static void Generate(void);

void usage(void)
{
    fprintf(stderr, "Terrain generator for tracks $Revision: 1.5.2.2 $ \n");
    fprintf(stderr, "Usage: trackgen -c category -n name [-a] [-m] [-s] [-S] [-E <n> [-H <nb>]]\n");
    fprintf(stderr, "       -c category    : track category (road, oval, dirt...)\n");
    fprintf(stderr, "       -n name        : track name\n");
    fprintf(stderr, "       -b             : draw bump track\n");
    fprintf(stderr, "       -B             : Don't use terrain border (relief supplied int clockwise, ext CC)\n");
    fprintf(stderr, "       -a             : draw all (default is track only)\n");
    fprintf(stderr, "       -z             : Just calculate track parameters and exit\n");
    fprintf(stderr, "       -s             : split the track and the terrain\n");
    fprintf(stderr, "       -S             : split all\n");
    fprintf(stderr, "       -E <n>         : save elevation file n\n");
    fprintf(stderr, "                         0: all elevatation files\n");
    fprintf(stderr, "                         1: elevation file of terrain + track\n");
    fprintf(stderr, "                         2: elevation file of terrain with track white\n");
    fprintf(stderr, "                         3: track only\n");
    fprintf(stderr, "                         4: track elevations with height steps\n");
    fprintf(stderr, "       -H <nb>        : nb of height steps for 4th elevation file [30]\n");
#ifndef WIN32
    fprintf(stderr, "       -L libDir      : Library directory\n");
#endif
}

void init_args(int argc, char **argv)
{
#ifdef WIN32
    int i=0;
#else
    int	c;    
#endif
    TrackOnly = 1;
	JustCalculate = 0;
    MergeAll = 1;
    MergeTerrain = 1;
    TrackName = NULL;
    TrackCategory = NULL;
    saveElevation = -1;

#ifndef WIN32
    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"help", 0, 0, 0},
	    {"version", 1, 0, 0}
	};

	c = getopt_long(argc, argv, "hvn:c:azsSE:H:bBL:",
			long_options, &option_index);
	if (c == -1)
	    break;
	     
	switch (c) {
	case 0:
	    switch (option_index) {
	    case 0:
		usage();
		exit(0);
		break;
	    case 1:
		printf("Terrain generator for tracks $Revision: 1.5.2.2 $ \n");
		exit(0);
		break;
	    default:
		usage();
		exit(1);
	    }
	    break;
	case 'h':
	    usage();
	    exit(0);
	    break;
	case 'H':
	    HeightSteps = strtol(optarg, NULL, 0);
	    break;
	case 'v':
	    printf("Terrain generator for tracks $Revision: 1.5.2.2 $ \n");
	    exit(0);
	    break;
	case 'a':
	    TrackOnly = 0;
	    break;
	case 'z':
	    JustCalculate = 1;
	    break;
	case 'b':
	    bump = 1;
	    break;
	case 's':
	    MergeAll = 0;
	    MergeTerrain = 1;
	    break;
	case 'S':
	    MergeAll = 0;
	    MergeTerrain = 0;
	    break;
	case 'n':
	    TrackName = strdup(optarg);
	    break;
	case 'c':
	    TrackCategory = strdup(optarg);
	    break;
	case 'E':
	    saveElevation = strtol(optarg, NULL, 0);;
	    TrackOnly = 0;
	    break;
	case 'B':
	    UseBorder = 0;
	    break;
	case 'L':
	    sprintf(buf, "%s/", optarg);
	    SetLibDir(buf);
	    break;
	default:
	    usage();
	    exit(1);
	    break;
	}
    }
#else
    i = 1;
    while (i < argc) {
	if (strncmp(argv[i], "-h", 2) == 0) {
	    usage();
	    exit(0);
	}
	if (strncmp(argv[i], "-v", 2) == 0) {
	    printf("Terrain generator for tracks $Revision: 1.5.2.2 $ \n");
	    exit(0);
	}
	if (strncmp(argv[i], "-a", 2) == 0) {
	    TrackOnly = 0;
	} else if (strncmp(argv[i], "-z", 2) == 0) {
	    JustCalculate = 1;
	} else if (strncmp(argv[i], "-s", 2) == 0) {
	    MergeAll = 0;
	    MergeTerrain = 1;
	} else if (strncmp(argv[i], "-B", 2) == 0) {
	    UseBorder = 0;
	} else if (strncmp(argv[i], "-S", 2) == 0) {
	    MergeAll = 0;
	    MergeTerrain = 0;
	} else if (strncmp(argv[i], "-n", 2) == 0) {
	    if (i + 1 < argc) {
		TrackName = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-E", 2) == 0) {
	    if (i + 1 < argc) {
		saveElevation = strtol(argv[++i], NULL, 0);
	    } else {
		usage();
		exit(0);
	    }
	    TrackOnly = 0;
	} else if (strncmp(argv[i], "-c", 2) == 0) {
	    if (i + 1 < argc) {
		TrackCategory = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-H", 2) == 0) {
	    if (i + 1 < argc) {
		HeightSteps = strtol(argv[++i], NULL, 0);
	    } else {
		usage();
		exit(0);
	    }
	} else {
	    usage();
	    exit(0);
	}
	i++;
    }
#endif

    if (!TrackName || !TrackCategory) {
	usage();
	exit(1);
    }
}
#ifndef WIN32
extern void LinuxSpecInit(void);
#else
extern void WindowsSpecInit(void);
#endif

int
main(int argc, char **argv)
{

    init_args(argc, argv);


    glutInit(&argc, argv);
    glutCreateWindow(argv[1]);

    ssgInit();
    
#ifndef WIN32
    LinuxSpecInit();
#else
    WindowsSpecInit();
#endif

    Generate();
    return 0;
}

#ifdef WIN32
#define INSTBASE "./"
#endif

static void
Generate(void)
{
	char *trackdllname;
	char *extName;
	FILE *outfd = NULL;

	// Get the trackgen paramaters.
	sprintf(buf, "%s", CFG_FILE);
	CfgHandle = GfParmReadFile(buf, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);

	trackdllname = GfParmGetStr(CfgHandle, "Modules", "track", "track");
	sprintf(buf, "%smodules/track/%s.%s", GetLibDir (), trackdllname, DLLEXT);
	if (GfModLoad(TRK_IDENT, buf, &modlist) < 0) {
		GfFatal("Failed to find the track module %s", buf);
	}

	if (modlist->modInfo->fctInit(modlist->modInfo->index, &TrackItf)) {
		GfFatal("Failed to init the track module %s", buf);
	}

	// This is the track definition.
	sprintf(trackdef, "tracks/%s/%s/%s.xml", TrackCategory, TrackName, TrackName);
	TrackHandle = GfParmReadFile(trackdef, GFPARM_RMODE_STD);
	if (!TrackHandle) {
		fprintf(stderr, "Cannot find %s\n", trackdef);
		exit(1);
	}

	// Build the track structure with graphic extensions.
	Track = TrackItf.trkBuildEx(trackdef);

	if (!JustCalculate) {
		// Get the output file radix.
		sprintf(buf2, "tracks/%s/%s/%s", Track->category, Track->internalname, Track->internalname);
		OutputFileName = strdup(buf2);

		// Number of goups for the complete track.
		if (TrackOnly) {
			sprintf(buf2, "%s.ac", OutputFileName);
			// Track.
			outfd = Ac3dOpen(buf2, 1);
		} else if (MergeAll) {
			sprintf(buf2, "%s.ac", OutputFileName);
			// track + terrain + objects.
			outfd = Ac3dOpen(buf2, 2 + GetObjectsNb(TrackHandle));
		}

		// Main Track.
		if (bump) {
			extName = "trk-bump";
		} else {
			extName = "trk";
		}

		sprintf(buf2, "%s-%s.ac", OutputFileName, extName);
		OutTrackName = strdup(buf2);
	}

	if (JustCalculate){
		CalculateTrack(Track, TrackHandle, bump);
		return;
	}

	GenerateTrack(Track, TrackHandle, OutTrackName, outfd, bump);

	if (TrackOnly) {
		return;
	}

	// Terrain.
	if (MergeTerrain && !MergeAll) {
		sprintf(buf2, "%s.ac", OutputFileName);
		/* terrain + objects  */
		outfd = Ac3dOpen(buf2, 1 + GetObjectsNb(TrackHandle));
	}

	extName = "msh";
	sprintf(buf2, "%s-%s.ac", OutputFileName, extName);
	OutMeshName = strdup(buf2);

	GenerateTerrain(Track, TrackHandle, OutMeshName, outfd, saveElevation);

	if (saveElevation != -1) {
		if (outfd) {
			Ac3dClose(outfd);
		}
		switch (saveElevation) {
		case 0:
		case 1:
			sprintf(buf2, "%s.ac", OutputFileName);
			sprintf(buf, "%s-elv.png", OutputFileName);
			SaveElevation(Track, TrackHandle, buf, buf2, 1);
			if (saveElevation) {
			break;
			}
		case 2:
			sprintf(buf, "%s-elv2.png", OutputFileName);
			SaveElevation(Track, TrackHandle, buf, OutMeshName, 1);
			if (saveElevation) {
			break;
			}
		case 3:
			sprintf(buf, "%s-elv3.png", OutputFileName);
			SaveElevation(Track, TrackHandle, buf, OutMeshName, 0);
			if (saveElevation) {
			break;
			}
		case 4:
			sprintf(buf, "%s-elv4.png", OutputFileName);
			SaveElevation(Track, TrackHandle, buf, OutTrackName, 2);
			break;
		}
		return;
	}

	GenerateObjects(Track, TrackHandle, CfgHandle, outfd, OutMeshName);
}
