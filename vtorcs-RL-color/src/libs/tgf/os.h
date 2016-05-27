/***************************************************************************
                        Os.h --- os specific functions interface                                
                             -------------------                                         
    created              : Fri Aug 13 22:27:29 CEST 1999
    copyright            : (C) 1999 by Eric Espie                         
    email                : torcs@free.fr   
    version              : $Id: os.h,v 1.2 2004/03/05 21:25:36 torcs Exp $                                  
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _OS__H_
#define _OS__H_

#include <tgf.h>
     
/* dynamic module (dll) interfaces */
typedef int (*tfModLoad)(unsigned int, char*, tModList **);
typedef int (*tfModLoadDir)(unsigned int, char*, tModList **);
typedef int (*tfModUnloadList)(tModList **);
typedef int (*tfModGetInfo)(unsigned int, char*, tModList **);
typedef int (*tfModGetInfoDir)(unsigned int, char*, int, tModList **);
typedef int (*tfModFreeInfoList)(tModList **);
/* directory interface */
typedef tFList *(*tfDirGetList)(char *);
typedef tFList *(*tfDirGetListFiltered)(char *, char *);
/* time interface */
typedef double (*tfTimeClock)(void);

typedef struct {
    tfModLoad			modLoad;
    tfModLoadDir		modLoadDir;
    tfModUnloadList		modUnloadList;
    tfModGetInfo		modInfo;
    tfModGetInfoDir		modInfoDir;
    tfModFreeInfoList		modFreeInfoList;
    tfDirGetList		dirGetList;
    tfDirGetListFiltered	dirGetListFiltered;
    tfTimeClock			timeClock;
} tGfOs;

#ifdef WIN32
#ifdef TGF_EXPORTS
__declspec(dllexport)
#else // TGF_EXPORTS
__declspec(dllimport)
#endif // TGF_EXPORTS
#endif // WIN32
extern tGfOs GfOs;

#endif /* _OS__H_ */
