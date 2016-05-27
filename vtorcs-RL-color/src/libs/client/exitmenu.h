/***************************************************************************

    file                 : exitmenu.h
    created              : Sat Mar 18 23:42:22 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: exitmenu.h,v 1.3 2006/10/05 21:25:55 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 

#ifndef _EXITMENU_H_
#define _EXITMENU_H_

#if _WIN32
#include <windows.h>
#endif

extern void *TorcsExitMenuInit(void *menu);
extern void *TorcsMainExitMenuInit(void *mainMenu);

#endif /* _EXITMENU_H_ */ 



