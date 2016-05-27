/***************************************************************************

    file        : accc.h
    created     : Fri Apr 18 23:09:53 CEST 2003
    copyright   : (C) 2003 by Christophe Guionneau
    version     : $Id: accc.h,v 1.7 2004/02/06 23:08:54 torcs Exp $                                  

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
    @version	$Id: accc.h,v 1.7 2004/02/06 23:08:54 torcs Exp $
*/

#ifndef _ACCC_H_
#define _ACCC_H_

#define FALSE 0
#define TRUE 1

extern char	*OutputFileName;
extern char	*ReliefFileName;
extern char * fileL0;
extern char * fileL1;
extern char * fileL2;
extern char * fileL3;
extern int d1;
extern int d2;
extern int d3;
extern int useStrip;
extern int extendedStrips;
extern int extendedTriangles;
extern int extendedEnvCoord;
extern int notexturesplit;
extern int isobjectacar;
extern int normalMapping;
extern char *OrderString;
extern int  collapseObject;
extern void loadAndGroup( char *OutputFileName);
extern int loadAC( char * inputFilename, char * outputFilename, int saveIn);

#define _AC3DTO3DS 1
#define _3DSTOAC3D 2
/** optimized version of ac3d using groups by section */
#define _AC3DTOAC3D 3
#define _AC3DTOOBJ 4
#define _AC3DTOAC3DM 5
#define _AC3DTOAC3DS 6
#define _AC3DTOAC3DGROUP 7



typedef struct point {
  double x;
  double y;
  double z;
} point_t;


typedef struct tcoord {
  int indice;
  double u;
  double v;
  int saved;
} tcoord_t;


typedef struct ob {
  char * name ;
  char * type ;
  int kids;
  point_t loc;
  int attrSurf;
  char * texture;
  char * texture1;
  char * texture2;
  char * texture3;
  char * data;
  double  texrep_x;
  double  texrep_y;
  int    numvert;
  int    numsurf;
  int numvertice ; /* the real number of textcoord */
  point_t  * vertex;
  point_t  * norm;
  point_t  * snorm;
  tcoord_t      * vertexarray;
  tcoord_t      * vertexarray1;
  tcoord_t      * vertexarray2;
  tcoord_t      * vertexarray3;
  int      * va;
  double    * textarray;
  double    * textarray1;
  double    * textarray2;
  double    * textarray3;
  int      * surfrefs;
  struct ob * next;
  double x_min;
  double y_min;
  double z_min;
  double x_max;
  double y_max;
  double z_max;
  double dist_min;
  struct ob* ob1;
  struct ob* ob2;
  struct ob* ob3;
  int saved;
  int kids_o;
  int inkids_o;
} ob_t;

typedef struct ob_groups
{
  struct ob * kids;
  int numkids;
  struct ob * tkmn;
  char * name;
  int    tkmnlabel;
  struct ob * kids0;
  int numkids0;
  struct ob * kids1;
  int numkids1;
  struct ob * kids2;
  int numkids2;
  struct ob * kids3;
  int numkids3;

}ob_groups_t;

typedef struct col
{
  double r;
  double g;
  double b;
} color_t;

typedef struct mat {
  char * name;
  color_t rgb;
  color_t amb;
  color_t emis;
  color_t spec;
  int shi;
  double trans;
  struct mat * next;
} mat_t;

extern int typeConvertion;
extern ob_t * root_ob;
extern int terrainSplitOb (ob_t **object);
extern int mergeSplitted (ob_t **object);
extern int distSplit;
extern int freeobject(ob_t *o);
double  findDistmin(ob_t * ob1, ob_t *ob2);

#define freez(x) {if ((x)) free((x)); }
#define SPLITX 75
#define SPLITY 75
#define MINVAL 0.001

#ifndef WIN32
#define stricmp strcasecmp
#define strnicmp strncasecmp
#else
#include <windows.h>
#include <float.h>
#define isnan _isnan
#endif


#endif /* _ACCC_H_ */ 

