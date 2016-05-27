/***************************************************************************

    file                 : differential.h
    created              : Sun Mar 19 00:06:46 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: differential.h,v 1.4 2005/03/31 16:01:00 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DIFFERENTIAL_H_
#define _DIFFERENTIAL_H_

typedef struct
{
    tdble	spinVel;
    tdble	Tq;
    tdble	brkTq;
    tdble	I;
} tDynAxis;

typedef struct
{
    /* caracteristics */
    int		type;
#define DIFF_NONE		0
#define DIFF_SPOOL		1
#define DIFF_FREE		2
#define DIFF_LIMITED_SLIP	3
#define DIFF_VISCOUS_COUPLER	4
    tdble	ratio;
    tdble	I;
    tdble	efficiency;
    tdble	bias;
    tdble	dTqMin;
    tdble	dTqMax;
    tdble	dSlipMax;
    tdble	lockInputTq;
    tdble	viscosity;
    tdble	viscomax;

    /* dynamic data */
    tDynAxis	in;
    tDynAxis	feedBack;
    tDynAxis	*inAxis[2];
    tDynAxis	*outAxis[2];
} tDifferential;



#endif /* _DIFFERENTIAL_H_ */ 



