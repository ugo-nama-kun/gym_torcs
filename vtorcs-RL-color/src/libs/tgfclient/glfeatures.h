/***************************************************************************

    file                 : glfeatures.h
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: glfeatures.h,v 1.2 2005/08/05 09:26:39 berniw Exp $

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/*
	Functions to check if features seems to be available and requested by the
	user. The isAvailable functions should return if a feature is working on
	the system, the isEnabled feature should check if the user wants to enable
	it as well.
	It should NOT check if the features are really working, that is subject
	to another part eventually.
*/

#ifndef _GRFEATURES_H_
#define _GRFEATURES_H_

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glext.h>
#else
#include <GL/gl.h>
#endif // WIN32

#include <tgfclient.h>
#include <graphic.h>

// Initialize
extern void checkGLFeatures(void);

// GL_ARB_texture_compression
extern bool isCompressARBAvailable(void);
extern bool isCompressARBEnabled(void);
extern void updateCompressARBEnabled(void);

extern int getUserTextureMaxSize(void);
extern int getGLTextureMaxSize(void);
extern void updateUserTextureMaxSize(void);

#endif // _GRFEATURES_H_

