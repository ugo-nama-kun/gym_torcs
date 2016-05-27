/***************************************************************************

    file        : nfsperf.cpp
    created     : Tue Aug  5 23:20:23 CEST 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: mainnfsperf.cpp,v 1.2 2005/02/01 15:08:12 berniw Exp $                                  

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
    @version	$Id: mainnfsperf.cpp,v 1.2 2005/02/01 15:08:12 berniw Exp $
*/


#ifdef WIN32
#include <windows.h>
#include <io.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <getopt.h>
#endif

#include <tgf.h>

char	*catfile = NULL;
char	*infile = "carp.txt";
char	*outfile = "carp.xml";

char	*gears[] = {"r", "n", "1", "2", "3", "4", "5", "6", "7", "8", "9"};


void usage(void)
{
    fprintf(stderr, "Car Specifications Converter $Revision: 1.2 $ \n");
    fprintf(stderr, "Usage: nfsperf [-c category.xml] [-i <input carp.txt>] [-o output carp.xml]\n");
    fprintf(stderr, "       -c category.xml    : car category description\n");
    fprintf(stderr, "       -i input carp.txt  : input NFS carp.txt\n");
    fprintf(stderr, "       -o output carp.xml : converted parameters\n");
}

void init_args(int argc, char **argv)
{
#ifdef WIN32
    int i;
#else
    int	c;    
#endif

#ifndef WIN32
    while (1) {
	int option_index = 0;
	static struct option long_options[] = {
	    {"help", 0, 0, 0},
	    {"version", 1, 0, 0}
	};

	c = getopt_long(argc, argv, "c:i:o:",
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
		printf("Car Specifications Converter $Revision: 1.2 $ \n");
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
	case 'c':
	    catfile = strdup(optarg);
	    break;
	case 'i':
	    infile = strdup(optarg);
	    break;
	case 'o':
	    outfile = strdup(optarg);
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
	    printf("Car Specifications Converter $Revision: 1.2 $ \n");
	    exit(0);
	}
	if (strncmp(argv[i], "-c", 2) == 0) {
	    if (i + 1 < argc) {
		catfile = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-i", 2) == 0) {
	    if (i + 1 < argc) {
		infile = strdup(argv[++i]);
	    } else {
		usage();
		exit(0);
	    }
	} else if (strncmp(argv[i], "-o", 2) == 0) {
	    if (i + 1 < argc) {
		outfile = strdup(argv[++i]);
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
}
int main (int argc, char **argv)
{
    FILE	*fin;
    void	*param;
    char	buf[1024];
    char	path[1024];
    char	*s;
    char	*end;
    int		i;
    int		idx;
    int		vald;
    tdble	valf;

    init_args (argc, argv);
    
    fin = fopen (infile, "rb");
    if (!fin) {
	perror (infile);
	exit (1);
    }
    
    if (catfile) {
	param = GfParmReadFile (catfile, GFPARM_RMODE_STD);
    } else {
	param = GfParmReadFile (outfile, GFPARM_RMODE_STD | GFPARM_RMODE_CREAT);
	GfParmClean(param);
    }
    

    /* Main parser */
    while (fgets (buf, sizeof (buf), fin)) {
	s = strrchr (buf, '(');
	if (!s) {
	    printf ("Syntax error: \"%s\"\n", buf);
	    exit (1);
	}
	
	idx = strtol (s + 1, NULL, 0);
	switch (idx) {

	case 2:			/* mass [kg] */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    GfParmSetNum (param, "Car", "mass", "kg", valf);
	    break;

	case 4:			/* gear shift delay (ticks)  */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%d", &vald);
	    valf = 0.025f * (tdble)vald;
	    GfParmSetNum (param, "Gearbox", "shift time", "s", valf);
	    break;

	case 8:			/* gear ratios (size 8) */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    s = buf;
	    i = 0;
	    while (s) {
		end = strchr (s, ',');
		if (end) {
		    *end = 0;
		    end++;
		}
		sscanf (s, "%f", &valf);
		if (i == 0) {
		    valf = -valf;
		} else if (i == 1) {
		    i++;
		    s = end;
		    continue;
		}
		sprintf (path, "Gearbox/gears/%s", gears [i++]);
		GfParmSetNum (param, path, "ratio", NULL, valf);
		s = end;
	    }
	    break;

	case 9:			/* gear efficiency (size 8) */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    s = buf;
	    i = 0;
	    while (s) {
		end = strchr (s, ',');
		if (end) {
		    *end = 0;
		    end++;
		}
		sscanf (s, "%f", &valf);
		sprintf (path, "Gearbox/gears/%s", gears [i++]);
		GfParmSetNum (param, path, "efficiency", NULL, valf);
		s = end;
	    }
	    break;

	case 10:		/* torque curve (size 21) in 500 rpm increments */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    s = buf;
	    i = 0;
	    while (s) {
		end = strchr (s, ',');
		if (end) {
		    *end = 0;
		    end++;
		}
		sscanf (s, "%f", &valf);
		sprintf (path, "Engine/data points/%d", i + 1);
		GfParmSetNum (param, path, "rpm", "rpm", 500.0f * i);
		GfParmSetNum (param, path, "Tq", "N.m", valf);
		s = end;
		i++;
	    }
	    break;

	case 11:			/* final gear */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    GfParmSetNum (param, "Rear Differential", "ratio", NULL, valf);
	    break;

	case 12:			/* engine minimum rpm */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%d", &vald);
	    valf = (tdble)vald;
	    GfParmSetNum (param, "Engine", "tickover", "rpm", valf);
	    break;

	case 13:			/* engine redline in rpm */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%d", &vald);
	    valf = (tdble)vald;
	    GfParmSetNum (param, "Engine", "revs limiter", "rpm", valf);
	    GfParmSetNum (param, "Engine", "revs maxi", "rpm", valf + 1000.0f);
	    break;

	case 16:			/* front drive ratio */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    if (valf != 0) {
		if (valf == 1.0) {
		    GfParmSetStr (param, "Drivetrain", "type", "FWD");
		    GfParmSetStr (param, "Rear Differential", "type", "NONE");
		    GfParmSetStr (param, "Front Differential", "type", "FREE");
		    GfParmSetNum (param, "Front Differential", "ratio", NULL, valf);
		} else {
		    GfParmSetStr (param, "Drivetrain", "type", "4WD");
		    GfParmSetStr (param, "Front Differential", "type", "FREE");
		    GfParmSetStr (param, "Central Differential", "type", "VISCOUS COUPLER");
		    GfParmSetNum (param, "Central Differential", "min torque bias", NULL, MAX (valf - 1.0f, 0.1f));
		    GfParmSetNum (param, "Central Differential", "max torque bias", NULL, MIN (valf + 1.0f, 0.9f));
		    valf = GfParmGetNum (param, "Rear Differential", "ratio", NULL, 3.5);
		    GfParmSetNum (param, "Central Differential", "ratio", NULL, valf);
		    GfParmSetNum (param, "Rear Differential", "ratio", NULL, 1.0);
		    GfParmSetNum (param, "Front Differential", "ratio", NULL, 1.0);
		}
	    }
	    break;
	    
	case 19:			/* front bias brake ratio */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    GfParmSetNum (param, "Brake System", "front-rear brake repartition", NULL, valf);
	    break;

	case 25:		/* front grip bias */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    GfParmSetNum (param, "Front Right Wheel", "mu", NULL, valf * 3.0f);
	    GfParmSetNum (param, "Front Left Wheel", "mu", NULL, valf * 3.0f);
	    GfParmSetNum (param, "Rear Right Wheel", "mu", NULL, (1.0f - valf) * 3.0f);
	    GfParmSetNum (param, "Rear Left Wheel", "mu", NULL, (1.0f - valf) * 3.0f);
	    break;

	case 31:		/* aerodynamic downforce multiplier */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    sscanf (buf, "%f", &valf);
	    GfParmSetNum (param, "Aerodynamics", "front Clift", NULL, valf * 120.0f);
	    GfParmSetNum (param, "Aerodynamics", "rear Clift", NULL, valf * 100.0f);
	    break;

	case 35:		/* tire specs front */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    s = buf;
	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Front Right Wheel", "tire width", "mm", (tdble)vald);
	    GfParmSetNum (param, "Front Left Wheel", "tire width", "mm", (tdble)vald);
	    s = end;

	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Front Right Wheel", "tire height-width ratio", "%", (tdble)vald);
	    GfParmSetNum (param, "Front Left Wheel", "tire height-width ratio", "%", (tdble)vald);
	    s = end;

	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Front Right Wheel", "rim diameter", "in", (tdble)vald);
	    GfParmSetNum (param, "Front Left Wheel", "rim diameter", "in", (tdble)vald);
	    s = end;

	    break;

	case 36:		/* tire specs rear */
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    s = buf;
	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Rear Right Wheel", "tire width", "mm", (tdble)vald);
	    GfParmSetNum (param, "Rear Left Wheel", "tire width", "mm", (tdble)vald);
	    s = end;

	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Rear Right Wheel", "tire height-width ratio", "%", (tdble)vald);
	    GfParmSetNum (param, "Rear Left Wheel", "tire height-width ratio", "%", (tdble)vald);
	    s = end;

	    end = strchr (s, ',');
	    if (end) {
		*end = 0;
		end++;
	    }
	    sscanf (s, "%d", &vald);
	    GfParmSetNum (param, "Rear Right Wheel", "rim diameter", "in", (tdble)vald);
	    GfParmSetNum (param, "Rear Left Wheel", "rim diameter", "in", (tdble)vald);
	    s = end;

	    break;

	    
	default:
	    if (!fgets (buf, sizeof (buf), fin)) {
		exit (1);
	    }
	    break;
	}
    }

    GfParmWriteFile (outfile, param, "car");

	return 0;
}
