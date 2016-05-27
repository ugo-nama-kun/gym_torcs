/***************************************************************************

    file                 : windowsspec.h
    created              : Sat Sep  2 10:48:41 CEST 2000
    copyright            : (C) 2000 by Patrice & Eric Espie
    email                : torcs@free.fr
    version              : $Id: windowsspec.h,v 1.1.1.1 2001/06/24 18:31:30 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _WINDOWSSPEC_H_
#define _WINDOWSSPEC_H_

#ifndef _WIN32
#error Hey ! Where Is _WIN32 ??
#endif

#include <windows.h>

extern void WindowsSpecInit(void);

#endif /* _WINDOWSSPEC_H_ */ 

