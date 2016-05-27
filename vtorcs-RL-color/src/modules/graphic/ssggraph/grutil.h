/***************************************************************************

    file                 : grutil.h
    created              : Wed Nov  1 22:35:08 CET 2000
    copyright            : (C) 2000 by Eric Espie
    email                : torcs@free.fr
    version              : $Id: grutil.h,v 1.11 2005/08/05 09:48:30 berniw Exp $

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 

#ifndef _GRUTIL_H_
#define _GRUTIL_H_

#include <stdio.h>
#include "grtexture.h"

#if 1
#define TRACE_GL(msg) { GLenum rc; if ((rc = glGetError()) != GL_NO_ERROR) printf("%s %s\n", msg, gluErrorString(rc)); }
#else
#define TRACE_GL(msg)
#endif

#ifdef DEBUG
#define DBG_SET_NAME(base, name, index, subindex)		\
{								\
    char __buf__[256];						\
    if (subindex != -1) {					\
        sprintf(__buf__, "%s-%d-%d", name, index, subindex);	\
    } else {							\
	sprintf(__buf__, "%s-%d", name, index);			\
    }								\
    (base)->setName((const char *)__buf__);			\
}
#else
#define DBG_SET_NAME(base, name, index, subindex)
#endif
 

/* Vars to set before calling grSsgLoadTexCb */
extern float	grGammaValue;
extern int	grMipMap;

extern char *grFilePath;	/* Multiple path (: separated) used to search for files */
extern char *grTexturePath;

extern int grGetFilename(char *filename, char *filepath, char *buf);
ssgState * grSsgEnvTexState(char *img);
extern ssgState *grSsgLoadTexState(char *img);
extern ssgState *grSsgLoadTexStateEx(char *img, char *filepath, int wrap, int mipmap);
extern bool grLoadPngTexture (const char *fname, ssgTextureInfo* info);
extern void grShutdownState(void);
extern void grWriteTime(float *color, int font, int x, int y, tdble sec, int sgn);
extern float grGetHOT(float x, float y);

#endif /* _GRUTIL_H_ */ 



