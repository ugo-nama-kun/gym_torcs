/***************************************************************************

    file        : raceinit.h
    created     : Sat Nov 16 12:24:26 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: raceinit.h,v 1.4 2004/11/03 22:47:06 torcs Exp $                                  

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
    @version	$Id: raceinit.h,v 1.4 2004/11/03 22:47:06 torcs Exp $
*/

#ifndef _RACEINIT_H_
#define _RACEINIT_H_

#define RACE_ENG_CFG	"config/raceengine.xml"

extern void ReInit(void);
extern void ReShutdown(void);
extern void ReStartNewRace(void * /* dummy */);
extern void ReAddRacemanListButton(void *menuHandle);
extern int  ReInitCars(void);
extern int  ReInitTrack(void);
extern void ReRaceCleanup(void);
extern void ReRaceCleanDrivers(void);
extern char *ReGetCurrentRaceName(void);
extern char *ReGetPrevRaceName(void);

extern tModList *ReRaceModList;



#endif /* _RACEINIT_H_ */ 



