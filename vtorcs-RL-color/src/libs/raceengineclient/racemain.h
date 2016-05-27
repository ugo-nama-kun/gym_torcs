/***************************************************************************

    file        : racemain.h
    created     : Sat Nov 16 12:14:57 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: racemain.h,v 1.3 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: racemain.h,v 1.3 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACEMAIN_H_
#define _RACEMAIN_H_

extern int  ReRaceEventInit(void);
extern int  RePreRace(void);
extern int  ReRaceStart(void);
extern int  ReRaceStop(void);
extern int  ReRaceEnd(void);
extern int  RePostRace(void);
extern int  ReEventShutdown(void);

#endif /* _RACEMAIN_H_ */ 



