/***************************************************************************

    file        : fg_gm.h
    created     : Sat Mar  8 15:28:15 CET 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: fg_gm.h,v 1.2 2003/06/24 21:02:25 torcs Exp $                                  

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
    @version	$Id: fg_gm.h,v 1.2 2003/06/24 21:02:25 torcs Exp $
*/

#ifndef _FG_GM_H_
#define _FG_GM_H_

extern void fglutGameModeString( const char* string );
extern int fglutEnterGameMode( void );
extern void fglutLeaveGameMode( void );

#endif /* _FG_GM_H_ */ 



