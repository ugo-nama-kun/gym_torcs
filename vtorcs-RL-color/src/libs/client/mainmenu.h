/***************************************************************************

    file                 : mainmenu.h
    created              : Sat Mar 18 23:42:51 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: mainmenu.h,v 1.2 2003/06/24 21:02:23 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef _MAINMENU_H_
#define _MAINMENU_H_

#if _WIN32
#include <windows.h>
#endif

extern void *menuHandle;
extern tModList *RacemanModLoaded;

extern int TorcsMainMenuInit(void);
extern int TorcsMainMenuRun(void);

#endif /* _MAINMENU_H_ */ 



