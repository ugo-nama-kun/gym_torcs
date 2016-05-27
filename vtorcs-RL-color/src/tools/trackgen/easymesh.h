/***************************************************************************

    file                 : easymesh.h
    created              : Sun Feb 25 22:50:07 /etc/localtime 2001
    copyright            : Bojan NICENO & Eric Espié
    email                : niceno@univ.trieste.it Eric.Espie@torcs.org
    version              : $Id: easymesh.h,v 1.3 2002/06/09 20:48:59 torcs Exp $

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
    @version	$Id: easymesh.h,v 1.3 2002/06/09 20:48:59 torcs Exp $
*/

#ifndef _EASYMESH_H_
#define _EASYMESH_H_

struct nod
{
    double x, y, z, F;
			
    double sumx, sumy;
    int    Nne;

    int mark;             /* is it off */

    int next;             /* next node in the boundary chain */
    int chain;            /* on which chains is the node */
    int inserted;

    int new_numb;         /* used for renumeration */
};

extern struct nod *point;

struct seg
{
    int n0, n1;
    int N;
    int chain;
    int bound;
    int mark;
};

extern struct seg *segment;

extern void GenerateTerrain(tTrack *track, void *TrackHandle, char *outfile, FILE *AllFd, int noElevation);

extern int Nc;
extern int Fl;

#endif /* _EASYMESH_H_ */ 



