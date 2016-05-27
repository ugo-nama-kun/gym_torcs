/***************************************************************************

    file                 : main.cpp
    created              : Sat Mar 18 23:54:30 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: main.cpp,v 1.14.2.1 2008/11/09 17:50:22 berniw Exp $

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

#include <GL/glut.h>

#include <tgfclient.h>
#include <client.h>

#include "linuxspec.h"

static void
init_args(int argc, char **argv)
{
    int		i;
    char	*buf;
    
    setTextOnly(false);
    setNoisy(false);
    setVersion("2010");

    i = 1;

//    for( int asdf = 0; asdf<argc; asdf++)
//      printf("arg %d: %s\n",asdf,argv[asdf]);

    printf("\n");


    while (i < argc) {
//    printf("arg %d: %s\n",i,argv[i]);
	if (strncmp(argv[i], "-c", 2) == 0) {
	    i++;
	    if (i < argc) {
		buf = (char *)malloc(strlen(argv[i]) + 2);
		sprintf(buf, "%s/", argv[i]);
		SetLocalDir(buf);
		free(buf);
		i++;
	    }
	} else if (strncmp(argv[i], "-L", 2) == 0) {
	    i++;
	    if (i < argc) {
		buf = (char *)malloc(strlen(argv[i]) + 2);
		sprintf(buf, "%s/", argv[i]);
		SetLibDir(buf);
		free(buf);
		i++;
	    }
	} else if (strncmp(argv[i], "-D", 2) == 0) {
	    i++;
	    if (i < argc) {
		buf = (char *)malloc(strlen(argv[i]) + 2);
		sprintf(buf, "%s/", argv[i]);
		SetDataDir(buf);
		free(buf);
		i++;
	    }
	} else if (strncmp(argv[i], "-s", 2) == 0) {
	    i++;
	    SetSingleTextureMode ();
	} else if (strncmp(argv[i], "-t", 2) == 0) {
	    i++;
	    if (i < argc) {
		long int t;
		sscanf(argv[i],"%ld",&t);
		setTimeout(t);
		printf("UDP Timeout set to %ld 10E-6 seconds.\n",t);
		i++;
	    }
	} else if (strncmp(argv[i], "-nodamage", 9) == 0) {
	    i++;
	    setDamageLimit(false);
	    printf("Car damages disabled!\n");
	} else if (strncmp(argv[i], "-nofuel", 7) == 0) {
	    i++;
	    setFuelConsumption(false);
	    printf("Fuel consumption disabled!\n");
	} else if (strncmp(argv[i], "-noisy", 6) == 0) {
	    i++;
	    setNoisy(true);
	    printf("Noisy Sensors!\n");
	} else if (strncmp(argv[i], "-ver", 4) == 0) {
	    i++;
	    if (i < argc) {
				setVersion(argv[i]);
	    		printf("Set version: \"%s\"\n",getVersion());
	    		i++;
	    }
	} else if (strncmp(argv[i], "-nolaptime", 10) == 0) {
	    i++;
	    setLaptimeLimit(false);
	    printf("Laptime limit disabled!\n");   
	} else if (strncmp(argv[i], "-T", 2) == 0) {
		    i++;
		    setTextOnly(true);
		    printf("Text Version!\n"); 
  // GIUSE - UDP PORT AS ARGUMENT
	} else if (strncmp(argv[i], "-p", 2) == 0) {
		    i++;
		    setUDPListenPort(atoi(argv[i]));
        i++;
		    printf("UDP Listen Port set to %d!\n", getUDPListenPort()); 
  // GIUSE - VISION HERE! ACTIVATE IMAGE GENERATION (and send them to clients if specified in the car/server)
	} else if (strncmp(argv[i], "-vision", 7) == 0) {
		    i++;
		    setVision(true);
		    printf("Image generation is ON!\n"); 
  // GIUSE - FASTER THEN RUNTIME ACTIVATION FOR NON-TEXTUAL COMPUTATION
	} else if (strncmp(argv[i], "-a", 2) == 0) {
		    i++;
		    printf("Speed set to %dx realtime!\n", atoi(argv[i])); 
		    setSpeedMult((double) (1.0 / (double) atoi(argv[i])));
        i++;

#ifndef FREEGLUT
	} else if (strncmp(argv[i], "-m", 2) == 0) {
	    i++;
	    GfuiMouseSetHWPresent(); /* allow the hardware cursor */
#endif
	} else {
	    i++;		/* ignore bad args */
	}
    }
#ifdef FREEGLUT
    GfuiMouseSetHWPresent(); /* allow the hardware cursor (freeglut pb ?) */
#endif
}

/*
 * Function
 *	main
 *
 * Description
 *	LINUX entry point of TORCS
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
int 
main(int argc, char *argv[])
{
    init_args(argc, argv);
    
    LinuxSpecInit();		/* init specific linux functions */
    
    if (getTextOnly()==false)
    	GfScrInit(argc, argv);	/* init screen */

    TorcsEntry();		/* launch TORCS */
    
    if (getTextOnly()==false)
    	glutMainLoop();		/* event loop of glut */

    return 0;			/* just for the compiler, never reached */
}

