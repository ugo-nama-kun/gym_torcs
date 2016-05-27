/***************************************************************************

    file                 : telemetryitf.cpp
    created              : Mon Feb 28 21:18:49 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: telemetryitf.cpp,v 1.3 2002/10/13 22:03:38 torcs Exp $

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
#include <telemetry.h>

#include "tlm.h"

#ifdef _WIN32
BOOL WINAPI DllEntryPoint (HINSTANCE hDLL, DWORD dwReason, LPVOID Reserved)
{
    return TRUE;
}
#endif

/*
 * Function
 *	telemInit
 *
 * Description
 *	
 *
 * Parameters
 *	
 *
 * Return
 *	
 */
static int
telemInit(int index, void *pt)
{
    tTelemItf	*ptf = (tTelemItf*)pt;
    
    ptf->init			= TlmInit;
    ptf->newChannel		= TlmNewChannel;
    ptf->startMonitoring	= TlmStartMonitoring;
    ptf->stopMonitoring		= TlmStopMonitoring;
    ptf->update			= TlmUpdate;    
    ptf->shutdown		= TlmShutdown;

    return 0;
}


/*
 * Function
 *	telemetry
 *
 * Description
 *	module entry point
 *
 * Parameters
 *	modinfo : administrative info to be filled by the module
 *
 * Return
 *	0  Ok
 *	-1 NOk
 *
 * Remarks
 *	all the logical modules should be linked in a ring
 */
extern "C" int
telemetry(tModInfo *modInfo)
{
    modInfo->name = "telemetry";	/* name of the module (short) */
    modInfo->desc = "Telemetry module, used to store telemetry information";	/* description of the module (can be long) */
    modInfo->fctInit = telemInit;	/* init function */
    modInfo->gfId = TLM_IDENT;		/* always loaded  */
    modInfo->index = 0;

    return 0;
}



