/***************************************************************************

    file                 : grtexture.h
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: grtexture.h,v 1.2.2.1 2008/08/16 14:12:08 berniw Exp $

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
	This classes/methods are used to handle texture compression and
	textures which are shared among multiple objects. In the long term
	they should obsolete parts of grutil.cpp.
*/

#ifndef _GRTEXTURE_H_
#define _GRTEXTURE_H_

#include <stdio.h>
#include <plib/ssg.h>
#include <glfeatures.h>
#include "grloadsgi.h"

bool doMipMap(const char *tfname, int mipmap);


// This state does currently not manage anything!
// TODO: manage shared textures, obsolete grutil.cpp parts.
class grManagedState : public ssgSimpleState {
	public:
		
		virtual void setTexture(ssgTexture *tex) {
			ssgSimpleState::setTexture(tex);
		}

		virtual void setTexture(const char *fname, int _wrapu = TRUE, int _wrapv = TRUE, int _mipmap = TRUE) {
			_mipmap = doMipMap(fname, _mipmap);
			ssgSimpleState::setTexture(fname, _wrapu, _wrapv, _mipmap);
		}

		virtual void setTexture(GLuint tex) {
			printf("Obsolete call: setTexture(GLuint tex)\n");
			ssgSimpleState::setTexture(tex);
		}
};


// Managed state factory.
inline grManagedState* grStateFactory(void) { return new grManagedState(); }

// Prototype for mipmap generation.
bool grMakeMipMaps(GLubyte *image, int xsize, int ysize, int zsize, bool mipmap);

// Prototype for SGI texture loading function.
bool grLoadSGI(const char *fname, ssgTextureInfo* info);

// Register customized loader in plib.
void grRegisterCustomSGILoader(void);

// SGI loader class to call customized ssgMakeMipMaps. This is necessary because
// of plib architecture which does not allow to customize the mipmap
// generation.
class grSGIHeader : public ssgSGIHeader {
	public:
		grSGIHeader(const char *fname, ssgTextureInfo* info);
};



#endif // _GRTEXTURE_H_
