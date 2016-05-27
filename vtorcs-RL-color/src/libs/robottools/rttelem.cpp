/***************************************************************************

    file                 : rttelem.cpp
    created              : Mon Feb 28 22:38:31 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: rttelem.cpp,v 1.8.2.1 2008/11/09 17:50:22 berniw Exp $

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
    		This is a collection of useful functions for using telemetry in a robot.
		You can see an example of telemetry usage in the human driver.
    @author	<a href=mailto:torcs@free.fr>Eric Espie</a>
    @version	$Id: rttelem.cpp,v 1.8.2.1 2008/11/09 17:50:22 berniw Exp $
    @ingroup	robottools
*/

/** @defgroup telemetry		Telemetry tools for robots.
    The telemetry is only working with Linux.
    <br>The graphs are build by gnuplot.
    @ingroup	robottools
*/
    
#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#endif
#include <telemetry.h>
#include <robottools.h>
#include <tgf.h>

#if 0
static tTelemItf	tlm;
static tModList		*modlist = (tModList*)NULL;
#endif

/** Initialization of a telemetry session.
    @ingroup	telemetry
    @param	ymin	Minimum value for Y.
    @param	ymax	Maximum value for Y.
    @return	None
 */
void RtTelemInit(tdble ymin, tdble ymax)
{
#ifdef later
    char	buf[256];
    tModInfo	*curModInfo;

    memset(&tlm, 0, sizeof(tTelemItf));
    sprintf(buf, "%smodules/telemetry/%s.%s", "telemetry", GetLibDir (), DLLEXT);
    if (GfModLoad(TLM_IDENT, buf, &modlist)) return;
    GfOut("--- %s loaded ---\n", modlist->modInfo->name);
    curModInfo = modlist->modInfo;
    curModInfo->fctInit(curModInfo->index, &tlm);

    tlm.init(ymin, ymax);
#endif
}


/** Create a new telemetry channel.
    @ingroup	telemetry
    @param	name	Name of the channel.
    @param	var	Address of the variable to monitor.
    @param	min	Minimum value of this variable.
    @param	max	Maximum value of this variable.
    @return	None
 */
void RtTelemNewChannel(const char * name, tdble * var, tdble min, tdble max)
{
#if 0
    tlm.newChannel(name, var, min, max);
#endif
}

/** Start recording into a file.
    @ingroup	telemetry
    @param	filename	name of the file to use.
    @return	none.
 */
void RtTelemStartMonitoring(const char * filename)
{
#if 0
    tlm.startMonitoring(filename);
#endif
}

/** Stop recording and close the file.
    @ingroup	telemetry
    @return	none
 */
void RtTelemStopMonitoring(void)
{
#if 0
    tlm.stopMonitoring();
#endif
}

/** Record a new set of values.
    @ingroup	telemetry
    @param	time	current time.
    @return	None.
 */
void RtTelemUpdate(double time)
{
#if 0
    tlm.update(time);
#endif
}

/** Deinstall the telemetry module.
    @ingroup	telemetry
    @return	none.
 */
void RtTelemShutdown(void)
{
#if 0
    tlm.shutdown();
    GfModUnloadList(&modlist);
#endif
}

