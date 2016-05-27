/***************************************************************************

    file                 : linalg_t.h
    created              : Mo Mar 11 13:51:00 CET 2002
    copyright            : (C) 2002-2005 by Bernhard Wymann
    email                : berniw@bluewin.ch
    version              : $Id: linalg_t.h,v 1.2 2005/08/05 09:04:53 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _LINALG_T_H_
#define _LINALG_T_H_

#include <tmath/v2_t.h>
#include <tmath/v3_t.h>
#include <tmath/v4_t.h>

#include <tmath/straight2_t.h>

typedef v4t<float> vec4f;
typedef v4t<double> vec4d;

typedef v3t<float> vec3f;
typedef v3t<double> vec3d;

typedef v2t<float> vec2f;
typedef v2t<double> vec2d;

typedef straight2t<float> straight2f;
typedef straight2t<double> straight2d;

#endif // _LINALG_T_H_

