/***************************************************************************

    file                 : trackgen.h
    created              : Sun Dec 24 16:00:03 CET 2000
    copyright            : (C) 2000 by Eric Espié
    email                : Eric.Espie@torcs.org
    version              : $Id: trackgen.h,v 1.7 2005/07/19 19:55:57 berniw Exp $

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
    @version	$Id: trackgen.h,v 1.7 2005/07/19 19:55:57 berniw Exp $
*/

#ifndef _TRACKGEN_H_
#define _TRACKGEN_H_

#define CLOCKWISE	0
#define ANTICLOCKWISE	1

#define CFG_FILE	"tools/trackgen/trackgen.xml"


extern void GenerateTrack(tTrack * Track, void *TrackHandle, char *outFile, FILE *AllFd, int bump);
extern void CalculateTrack(tTrack * Track, void *TrackHandle, int bump);

extern char	*OutputFileName;
extern int	HeightSteps;
extern int	UseBorder;

#endif /* _TRACKGEN_H_ */ 



