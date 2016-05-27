/***************************************************************************

    file                 : trackitf.cpp
    created              : Sun Jan 30 22:57:50 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: trackitf.cpp,v 1.4 2003/11/08 16:37:18 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <stdlib.h>
#include <tgf.h>
#include <track.h>
#include "trackinc.h"

#ifdef _WIN32
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
    return TRUE;
}
#endif

/*
 * Function
 *	trackInit
 *
 * Description
 *	init the menus
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
static int
trackInit(int /* index */, void *pt)
{
    tTrackItf	*ptf = (tTrackItf*)pt;
    
    ptf->trkBuild         = TrackBuildv1;
    ptf->trkBuildEx       = TrackBuildEx;
    ptf->trkHeightG       = TrackHeightG;
    ptf->trkHeightL       = TrackHeightL;
    ptf->trkGlobal2Local  = TrackGlobal2Local;
    ptf->trkLocal2Global  = TrackLocal2Global;
    ptf->trkSideNormal    = TrackSideNormal;
    ptf->trkSurfaceNormal = TrackSurfaceNormal;
    ptf->trkShutdown      = TrackShutdown;
    
    return 0;
}


/*
 * Function
 *	trackv1
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 *
 * Remarks
 *	
 */
extern "C" int
track(tModInfo *modInfo)
{
    modInfo->name = "trackv1";		/* name of the module (short) */
    modInfo->desc = "Track V1.0";	/* description of the module (can be long) */
    modInfo->fctInit = trackInit;	/* init function */
    modInfo->gfId = TRK_IDENT;		/* always loaded  */
    modInfo->index = 0;

    return 0;
}



