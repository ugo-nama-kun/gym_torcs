/***************************************************************************

    file        : controlconfig.h
    created     : Wed Mar 12 22:09:01 CET 2003
    copyright   : (C) 2003 by Eric Espié                        
    email       : eric.espie@torcs.org   
    version     : $Id: controlconfig.h,v 1.3 2003/11/08 16:37:18 torcs Exp $                                  

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
    @version	$Id: controlconfig.h,v 1.3 2003/11/08 16:37:18 torcs Exp $
*/

#ifndef _CONTROLCONFIG_H_
#define _CONTROLCONFIG_H_

extern void *TorcsControlMenuInit(void *prevMenu, int index);


typedef struct
{
    char	*name;
    tCtrlRef	ref;
    int		Id;
    char	*minName;
    float	min;
    char	*maxName;
    float	max;
    char	*powName;
    float	pow;
    int		keyboardPossible;
} tCmdInfo;

#endif /* _CONTROLCONFIG_H_ */ 



