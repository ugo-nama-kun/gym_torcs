/***************************************************************************

    file                 : telemetry.cpp
    created              : Sat Feb 26 16:48:43 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: telemetry.cpp,v 1.7 2002/12/30 11:30:22 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <tgf.h>
#include <telemetry.h>

#include "tlm.h"

typedef struct Channel 
{
    struct Channel	*next;
    const char		*name;	/* channel name */
    tdble		*val;	/* monitored value */
    tdble		scale;
} tChannel;

typedef struct Tlm
{
    FILE	*file;	/* associated file */
    char	*cmdfile;
    int		state;
    tdble	ymin;
    tdble	ymax;
    tChannel	*chanList;
} tTlm;
    
static tTlm	TlmData;


/*
 * Function
 *	TlmInit
 *
 * Description
 *	Init telemetry internal structures.
 *
 * Parameters
 *	none
 *
 * Return
 *	none
 */
void
TlmInit(tdble ymin, tdble ymax)
{
    TlmData.file	= (FILE*)NULL;
    TlmData.state	= 0;
    TlmData.ymin	= ymin;
    TlmData.ymax	= ymax;
    TlmData.chanList	= (tChannel*)NULL;
}


/*
 * Function
 *	TlmNewChannel
 *
 * Description
 *	Create a new channel
 *
 * Parameters
 *
 * Return
 *	channel ID or -1 if error
 */
void
TlmNewChannel(const char *name, tdble *var, tdble min, tdble max)
{
    tChannel	*curChan;

    curChan = (tChannel *)calloc(sizeof(tChannel), 1);
    if (TlmData.chanList == NULL) {
	TlmData.chanList = curChan;
	curChan->next = curChan;
    } else {
	curChan->next = TlmData.chanList->next;
	TlmData.chanList->next = curChan;
	TlmData.chanList = curChan;
    }

    curChan->name = name;
    curChan->val = var;
    if ((min == 0) && (max == 0)) {
	curChan->scale = 1.0;
    } else {
	curChan->scale = TlmData.ymax / max;
    }
}

void 
TlmStartMonitoring(const char *filename)
{
    char	buf[1024];
    FILE	*fout;
    FILE	*fcmd;
    tChannel	*curChan;
    int		i;
    
    GfOut("Telemetry: start monitoring\n");

    sprintf(buf, "telemetry/%s.cmd", filename);
    fcmd = fopen(buf, "w");
    if (fcmd == NULL) {
	return;
    }
    fprintf(fcmd, "#!/bin/sh\n");
    fprintf(fcmd, "gnuplot -persist > telemetry/%s.png <<!!\n", filename);
    fprintf(fcmd, "#    set yrange [%f:%f]\n", TlmData.ymin, TlmData.ymax);
    fprintf(fcmd, "    set grid\n");
    fprintf(fcmd, "    set size 2.5,1.5\n");
    fprintf(fcmd, "    set terminal png color\n");
    fprintf(fcmd, "    set data style lines\n");
    curChan = TlmData.chanList;
    if (curChan != NULL) {
	i = 2;
	do {
	    curChan = curChan->next;
	    if (i == 2) {
		fprintf(fcmd, "plot 'telemetry/%s.dat' using %d title '%s'", filename, i, curChan->name);
	    } else {
		fprintf(fcmd, ", '' using %d title '%s'", i, curChan->name);
	    }
	    i++;
	} while (curChan != TlmData.chanList);
	fprintf(fcmd, "\n");
    }
    fprintf(fcmd, "!!\n");
    fclose(fcmd);
    
    TlmData.cmdfile = strdup(buf);
    
    sprintf(buf, "telemetry/%s.dat", filename);
    fout = TlmData.file = fopen(buf, "w");
    if (fout == NULL) {
	return;
    }
    curChan = TlmData.chanList;
    fprintf(fout, "time");
    if (curChan != NULL) {
	do {
	    curChan = curChan->next;
	    fprintf(fout, "	%s", curChan->name);
	} while (curChan != TlmData.chanList);
	fprintf(fout, "\n");
    }
    
    TlmData.state = 1;
}

void 
TlmUpdate(double time)
{
    FILE	*fout;
    tChannel	*curChan;

    if (TlmData.state == 0) {
	return;
    }
    fout = TlmData.file;
    fprintf(fout, "%f ", time);
    
    curChan = TlmData.chanList;
    if (curChan != NULL) {
	do {
	    curChan = curChan->next;
	    fprintf(fout, "%f ", curChan->scale * (*curChan->val));
	} while (curChan != TlmData.chanList);
    }
    fprintf(fout, "\n");
}


void 
TlmStopMonitoring(void)
{
    char	buf[256];
    
    if (TlmData.state == 1) {
	fclose(TlmData.file);
    }
    TlmData.file = (FILE*)NULL;
    TlmData.state = 0;
    GfOut("Telemetry: stop monitoring\n");

    sprintf(buf, "sh %s", TlmData.cmdfile);
    system(buf);
    free(TlmData.cmdfile);
}

/*
 * Function
 *	TlmShutdown
 *
 * Description
 *	release all the channels
 *
 * Parameters
 *	none
 *
 * Return
 *	none
 */
void
TlmShutdown(void)
{
    if (TlmData.state == 1) {
	TlmStopMonitoring();
    }
    /* GfRlstFree((tRingList **)&TlmData.chanList); */
}
