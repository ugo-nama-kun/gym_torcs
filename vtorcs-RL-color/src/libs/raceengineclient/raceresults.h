/***************************************************************************

    file        : raceresults.h
    created     : Thu Jan  2 12:43:28 CET 2003
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: raceresults.h,v 1.3 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: raceresults.h,v 1.3 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACERESULTS_H_
#define _RACERESULTS_H_


extern void ReInitResults(void);
extern void ReStoreRaceResults(char *race);
extern int  ReDisplayResults(void);
extern void ReDisplayStandings(void);
extern void ReSavePracticeLap(tCarElt *car);
extern void ReUpdateQualifCurRes(tCarElt *car);
extern void ReEventInitResults(void);
extern void ReUpdateStandings();

#endif /* _RACERESULTS_H_ */ 



