/***************************************************************************

    file                 : steer.h
    created              : Sun Mar 19 00:08:32 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: steer.h,v 1.4 2005/03/31 16:01:01 olethros Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _STEER_H_
#define _STEER_H_

typedef struct
{
    tdble	steerLock;	/* in rad */
    tdble	maxSpeed;	/* in rad/s */
    tdble	steer;		/* current steer value */
    
} tSteer;




#endif /* _STEER_H_ */ 



