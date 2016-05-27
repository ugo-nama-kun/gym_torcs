/***************************************************************************

    file        : racestate.h
    created     : Sat Nov 16 14:05:06 CET 2002
    copyright   : (C) 2002 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: racestate.h,v 1.3 2004/04/05 18:25:00 olethros Exp $                                  

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
    @version	$Id: racestate.h,v 1.3 2004/04/05 18:25:00 olethros Exp $
*/

#ifndef _RACESTATE_H_
#define _RACESTATE_H_

extern void ReStateInit(void *prevMenu);
extern void ReStateManage(void);
extern void ReStateApply(void *state);

#endif /* _RACESTATE_H_ */ 



