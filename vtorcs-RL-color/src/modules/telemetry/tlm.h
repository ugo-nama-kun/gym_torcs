/***************************************************************************

    file                 : tlm.h
    created              : Sun Mar 19 00:10:19 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: tlm.h,v 1.2 2002/11/30 19:00:57 torcs Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _TLM_H_
#define _TLM_H_

extern void TlmInit(tdble ymin, tdble ymax);
extern void TlmNewChannel(const char *name, tdble *var, tdble min, tdble max);
extern void TlmStartMonitoring(const char *filename);
extern void TlmUpdate(double time);
extern void TlmStopMonitoring(void);
extern void TlmShutdown(void);

#endif /* _TLM_H_ */ 



