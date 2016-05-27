/***************************************************************************

    file        : accc.cpp
    created     : Fri Apr 18 23:10:53 CEST 2003
    copyright   : (C) 2003 by Christophe Guionneau
    version     : $Id: mainaccc.cpp,v 1.4.2.1 2008/11/09 17:50:23 berniw Exp $                                  

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
    		
    @author	Christophe Guionneau
    @version	$Id: mainaccc.cpp,v 1.4.2.1 2008/11/09 17:50:23 berniw Exp $
*/


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cstring>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>

#include "accc.h"
/* -g  g-track-2.ac -l0 g2.ac -d3 200900 -d2 200 -d1 100 -S 200 >t */
/* +es 80 p-gt1.ac porsche-gt1.ac>t */
int distSplit=0;
char	*InputFileName = NULL;
char	*OutputFileName = NULL;
char * fileL0=NULL;
char * fileL1=NULL;
char * fileL2=NULL;
char * fileL3=NULL;
char * OrderString=NULL;
int d1=0;
int d2=0;
int d3=0;
int useStrip=0;
int extendedStrips=0;
int extendedEnvCoord=0;
int isobjectacar=0;
int extendedTriangles=0;
int notexturesplit=0;
int collapseObject=0;
int typeConvertion =0;
int far_dist=10000;
int normalMapping=0;
extern double smooth_angle;

void usage(void)
{
    fprintf(stderr, "Ac3d Compiler $Revision: 1.4.2.1 $ \n");
    fprintf(stderr, "Usage: accc [option] <inputfile> <outputfile> [dist][-order \"orderstring\"]<\n");
    /*fprintf(stderr, "-strip : stripify any ac3d output models\n");*/
    fprintf(stderr, "+o: ac3d to ac3d : the result file is optimized\n");
    fprintf(stderr, "                   in using groups by zone of track\n");
    fprintf(stderr, "                   track must be named TKMNxxx\n");
    fprintf(stderr, "                   dist is used to determine the distance of the objects\n");
    /*    fprintf(stderr, "+om: ac3d to ac3d : the result is a vertex merged unique obj\n" );
	  fprintf(stderr, "                    for smooth car only\n");*/
    fprintf(stderr, "-o: ac3d to obj \n");
    fprintf(stderr, "+s: ac3d to ac3d with strips (used for cars) \n");
    fprintf(stderr, "+es <angle_of_smooth>: ac3d to ac3d with strips (used for cars) and extended ac3d file output \n");
    fprintf(stderr, "    (computed normals) angle is in degrees : example : +es 80.0 clkdtm_o.ac clkdtm.ac  \n");
    fprintf(stderr, "    the output file will also contains the secondary textcoord for the env sliding \n");
    fprintf(stderr, "+et: ac3d to ac3d with triangles (used for cars) and extended ac3d file output (normals normals) \n");
    fprintf(stderr, "+etn: ac3d to ac3d with triangles no split and extended ac3d file output (normals normals) \n");
    fprintf(stderr, "+shad: ac3d to ac3d with triangles, create a new ac3d file used for track shading\n   vertical mapping of a single texture \n");
    fprintf(stderr, "<inputfile>: \n");
    fprintf(stderr, "<oututfile>: \n");
    fprintf(stderr, "-order \"orderstring\": only used with +es , +s and  +et  \n");
    fprintf(stderr, "       \"orderstring\" is the order of the objects during the save\n");
    fprintf(stderr, "        for example : \"HB;OB;OBM\" is the names of the objet separate\n");
    fprintf(stderr, "        by a semicolon\n");
    fprintf(stderr, "        for imported NFS cars the object name \"HB\" must be used instead\n");
    fprintf(stderr, "        of \":HB\"\n");
    fprintf(stderr, "-g <outputfile>: for grouping multilayer ac3d track \n");
    fprintf(stderr, "     -l0 <input filename> : the base geometry\n");
    fprintf(stderr, "     -l1 <input filename> : the base geometry map-tiled \n");
    fprintf(stderr, "     -l2 <input filename> : the base geometry skids/and grass details \n");
    fprintf(stderr, "     -l3 <input filename> : the base geometry shadows \n");
    fprintf(stderr, "     -d3 <val> : the max distance of groups in tkmn\n");
    fprintf(stderr, "     -d2 <val> : the distance of group 2 in tkmn\n");
    fprintf(stderr, "     -d1 <val> : the distance of group 1 in tkmn\n");
    fprintf(stderr, "     -S <val> : the distance of splitting for TERRAIN* objects\n");
    fprintf(stderr, "     -es : for extended ac3d file format for strips\n");
    fprintf(stderr, "     -nts : no texture split (for debug)\n");
    
}

void init_args(int argc, char **argv)
{
  if (argc<4)
    {usage();exit(1);}
    if (!strcmp(argv[1],"-o")) {
      typeConvertion=_AC3DTOOBJ;
    }
    else  if (!strcmp(argv[1],"+o")) {
      typeConvertion=_AC3DTOAC3D;
    } 
    else  if (!strcmp(argv[1],"+om")) {
      typeConvertion=_AC3DTOAC3DM;
    } 
    else  if (!strcmp(argv[1],"+s")) {
      typeConvertion=_AC3DTOAC3DS;
    } 
    else  if (!strcmp(argv[1],"+es")) {
      typeConvertion=_AC3DTOAC3DS;
      isobjectacar=1;
      extendedStrips=1;
      extendedEnvCoord=1;
      collapseObject=1;
      sscanf(argv[2],"%lf",&smooth_angle);
      InputFileName = strdup(argv[3]);
      OutputFileName = strdup(argv[4]);
    } 
    else  if (!strcmp(argv[1],"+et")) {
      typeConvertion=_AC3DTOAC3D;
      extendedStrips=0;
      isobjectacar=1;
      extendedEnvCoord=1;
      extendedTriangles=1;
      collapseObject=1;
    } 
    else  if (!strcmp(argv[1],"+etn")) {
      typeConvertion=_AC3DTOAC3D;
      extendedStrips=0;
      isobjectacar=1;
      extendedEnvCoord=1;
      notexturesplit=0;
      collapseObject=1;
      extendedTriangles=1;
    } 
    else  if (!strcmp(argv[1],"+shad")) {
      typeConvertion=_AC3DTOAC3D;
      extendedStrips=0;
      isobjectacar=0;
      notexturesplit=1;
      extendedTriangles=0;
      normalMapping=1;
      collapseObject=1;
    } 
    else  if (!strcmp(argv[1],"-g")) {
      typeConvertion=_AC3DTOAC3DGROUP;
    } 
    else {
      usage();
      exit(1);
    }

    if (typeConvertion==_AC3DTOAC3DGROUP)
      {
	int i=3;
	/* read other arguments */
	OutputFileName = strdup(argv[2]);
	for (i=3; i<argc; i++)
	  {
	    if (!strcmp(argv[i],"-es"))
	      {
		extendedStrips=1;
		continue;
	      }

	    if (!strcmp(argv[i],"-nts"))
	      {
		notexturesplit=1;
		continue;
	      }

	    if (!strcmp(argv[i],"-S"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		distSplit=atoi(argv[i]);
		continue;
	      }


	    if (!strcmp(argv[i],"-l0"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		fileL0=argv[i];
		continue;
	      }
	    if (!strcmp(argv[i],"-l1"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		fileL1=argv[i];
		continue;
	      }
	    if (!strcmp(argv[i],"-l2"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		fileL2=argv[i];
		continue;
	      }
	    if (!strcmp(argv[i],"-l3"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		fileL3=argv[i];
		continue;
	      }
	    if (!strcmp(argv[i],"-d3"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		d3=atoi(argv[i]);
		continue;
	      }
	    if (!strcmp(argv[i],"-d2"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		d2=atoi(argv[i]);
		continue;
	      }
	    if (!strcmp(argv[i],"-d1"))
	      {
		if (argc<=i+1){usage();exit(-1);}
		i++;
		d1=atoi(argv[i]);
		continue;
	      }
	    
	  }
      }
    else
      {
	if (InputFileName==NULL)
	  InputFileName = strdup(argv[2]);
	if (OutputFileName==NULL)
	  OutputFileName = strdup(argv[3]);

	if (argc==7)
	  {if (!strcmp(argv[5],"-order"))
	      OrderString=argv[6];
	  }
	else
	  if (typeConvertion==_AC3DTOAC3D)
	    if (argc>=5)
	      if (!strcmp(argv[4],"-order"))
		OrderString=argv[5];
	      else
		far_dist=atoi(argv[4]);

      }
}

/*extern void WindowsSpecInit(void);*/
void redraw ()
{}
void reshape ( int w, int h )
{}
void keyboard ( unsigned char, int, int )
{ exit ( 0 ) ;}


int
main(int argc, char **argv)
{
  char *fake_argv[3] ;
  fake_argv[0] = "ac3dto3ds" ;
  fake_argv[1] = "converter" ;
  fake_argv[2] = NULL ;


  init_args(argc, argv);
  extern int loadACo( char * inputFilename, char * outputFilename, int saveIn); 
  extern int loadAC( char * inputFilename, char * outputFilename, int saveIn);
#ifdef _3DS
  extern int load3ds( char * inputFilename, char * outputFilename);
#endif

  if (typeConvertion==_AC3DTO3DS)
    {
      loadAC( InputFileName, OutputFileName,0);
    }
  else if (typeConvertion==_3DSTOAC3D)
    {
#ifdef _3DS
      load3ds( InputFileName, OutputFileName);
#endif
    }
  else if (typeConvertion==_AC3DTOAC3D)
    {
      loadACo( InputFileName, OutputFileName,0);
    }
  else  if (typeConvertion==_AC3DTOOBJ)
    {
      loadAC( InputFileName, OutputFileName,1);
    }
  else  if (typeConvertion==_AC3DTOAC3DM)
    {
      loadAC( InputFileName, OutputFileName,2);
    }
  else  if (typeConvertion==_AC3DTOAC3DS)
    {
      loadAC( InputFileName, OutputFileName,3);
    }
  else if (typeConvertion==_AC3DTOAC3DGROUP)
    {
     loadAndGroup( OutputFileName);
    }


  return 0;
}

