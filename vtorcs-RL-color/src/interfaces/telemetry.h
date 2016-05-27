/***************************************************************************

    file                 : telemetry.h
    created              : Sun Jan 30 22:59:54 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: telemetry.h,v 1.3 2004/10/07 17:05:04 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef _TELEMETRY_H_
#define _TELEMETRY_H_

#define TLM_IDENT	1

#include <tgf.h>

typedef void(*tftlmInit)(tdble /* ymin */, tdble /* ymax */);
typedef void(*tftlmNewChannel)(const char * /* name */, tdble * /* var */, tdble /* min */, tdble /* max */);
typedef void(*tftlmStartMonitoring)(const char * /* filename */);
typedef void(*tftlmStopMonitoring)(void);
typedef void(*tftlmUpdate)(double /* time */);
typedef void(*tftlmShutdown)(void);


typedef struct 
{
    tftlmInit			init;			/* Init the telemetry */
    tftlmNewChannel		newChannel;		/* Create a new channel */
    tftlmStartMonitoring	startMonitoring;	/* Start monitoring */
    tftlmStopMonitoring		stopMonitoring;		/* Stop monitoring */
    tftlmUpdate			update;			/* Update telemetry */
    tftlmShutdown		shutdown;		/* close the telemetry */
} tTelemItf;



#endif /* _TELEMETRY_H_ */ 



