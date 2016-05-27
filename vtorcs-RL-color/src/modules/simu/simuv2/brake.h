/***************************************************************************

    file                 : brake.h
    created              : Sun Mar 19 00:05:34 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: brake.h,v 1.4 2005/03/31 16:01:00 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _BRAKE_H_
#define _BRAKE_H_

typedef struct
{
    tdble	pressure;
    tdble	Tq;
    tdble	coeff;
    tdble	I;
    tdble	radius;
    tdble	temp;
} tBrake;

typedef struct
{
    tdble	rep;	/* front/rear repartition */ 
    tdble	coeff;
} tBrakeSyst;



#endif /* _BRAKE_H_ */ 



