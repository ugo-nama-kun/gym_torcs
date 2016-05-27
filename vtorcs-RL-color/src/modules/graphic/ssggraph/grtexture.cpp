/***************************************************************************

    file                 : grtexture.cpp
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: grtexture.cpp,v 1.2 2007/11/06 20:43:32 torcs Exp $

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

#include "grtexture.h"

bool doMipMap(const char *tfname, int mipmap)
{
	char *buf = (char *) malloc(strlen(tfname)+1);
	strcpy(buf, tfname);

	// find the filename extension.
	char *s = strrchr(buf, '.');
	if (s) {
		*s = 0;
	}

	// search for the texture parameters.
	s = strrchr(buf, '_');

	// no mipmap for "_n" and "shadow".
	if (s) {
		// check the "_n".
		if (strncmp(s, "_n", 4) == 0) {
			mipmap = FALSE;
		}
	}

	if (mipmap == TRUE) {
		// Check the shadow.
		s = strrchr((char *)tfname, '/');
		if (s == NULL) {
			s = (char *) tfname;
		} else {
			s++;
		}
		if (strstr(s, "shadow") != NULL) {
			mipmap = FALSE;
		}
	}
	free(buf);
	return mipmap;
}


/*
	The latter parts are derived from plib (plib.sf.net) and have this license:

	PLIB - A Suite of Portable Game Libraries
	Copyright (C) 1998,2002  Steve Baker

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

	For further information visit http://plib.sourceforge.net
*/

/*
	Modifications:
		Copyright (c) 2005 Bernhard Wymann
*/


// SGI texture loading function.
bool grLoadSGI(const char *fname, ssgTextureInfo* info)
{
	grSGIHeader *sgihdr = new grSGIHeader(fname, info);
	bool returnval = sgihdr->loadSGI_bool;
	delete sgihdr;
	return returnval;
}


// Register customized loader in plib.
void grRegisterCustomSGILoader(void)
{
	ssgAddTextureFormat(".rgb", grLoadSGI);
	ssgAddTextureFormat(".rgba", grLoadSGI);
	ssgAddTextureFormat(".int", grLoadSGI);
	ssgAddTextureFormat(".inta", grLoadSGI);
	ssgAddTextureFormat(".bw", grLoadSGI);
}


grSGIHeader::grSGIHeader(const char *fname, ssgTextureInfo* info)
{
	grSGIHeader *sgihdr = this;

	start = NULL;
	leng = NULL;

	bool success = openFile(fname);

	int mipmap = doMipMap(fname, TRUE);

	if (!success) {
		loadSGI_bool = false;
		return;
	}

	GLubyte *image = new GLubyte [sgihdr->xsize*sgihdr->ysize*sgihdr->zsize];
	GLubyte *ptr = image;

	unsigned char *rbuf = new unsigned char[sgihdr->xsize];
	unsigned char *gbuf = (sgihdr->zsize>1) ? new unsigned char[sgihdr->xsize] : (unsigned char *) NULL ;
	unsigned char *bbuf = (sgihdr->zsize>2) ? new unsigned char[sgihdr->xsize] : (unsigned char *) NULL ;
	unsigned char *abuf = (sgihdr->zsize>3) ? new unsigned char[sgihdr->xsize] : (unsigned char *) NULL ;

	for (int y = 0 ; y < sgihdr->ysize ; y++) {
		int x ;

		switch (sgihdr->zsize) {
			case 1:
				sgihdr->getRow(rbuf, y, 0);
				for (x = 0; x < sgihdr->xsize; x++) {
					*ptr++ = rbuf[x];
				}
				break;

			case 2:
				sgihdr->getRow (rbuf, y, 0);
				sgihdr->getRow (gbuf, y, 1);

				for (x = 0; x < sgihdr->xsize; x++) {
					*ptr++ = rbuf[x];
					*ptr++ = gbuf[x];
				}
				break;

			case 3:
				sgihdr->getRow(rbuf, y, 0);
				sgihdr->getRow(gbuf, y, 1);
				sgihdr->getRow(bbuf, y, 2);

				for (x = 0; x < sgihdr->xsize; x++) {
					*ptr++ = rbuf[x];
					*ptr++ = gbuf[x];
					*ptr++ = bbuf[x];
				}
				break;

			case 4:
				sgihdr->getRow(rbuf, y, 0);
				sgihdr->getRow(gbuf, y, 1);
				sgihdr->getRow(bbuf, y, 2);
				sgihdr->getRow(abuf, y, 3);

				for (x = 0; x < sgihdr->xsize; x++ ) {
					*ptr++ = rbuf[x];
					*ptr++ = gbuf[x];
					*ptr++ = bbuf[x];
					*ptr++ = abuf[x];
				}
				break;
		}
	}

	fclose(image_fd);
	image_fd = NULL ;
	delete [] rbuf;
	delete [] gbuf;
	delete [] bbuf;
	delete [] abuf;

	if (info != NULL) {
		info->width = sgihdr->xsize;
		info->height = sgihdr->ysize;
		info->depth = sgihdr->zsize;
		info->alpha = (sgihdr->zsize == 2 || sgihdr->zsize == 4);
	}

	//printf("%s: ", jn);

	bool result = grMakeMipMaps(image, sgihdr->xsize, sgihdr->ysize, sgihdr->zsize, mipmap);

	loadSGI_bool= result ;
}


bool grMakeMipMaps (GLubyte *image, int xsize, int ysize, int zsize, bool mipmap)
{
	if (!((xsize & (xsize-1))==0) || !((ysize & (ysize-1))==0)) {
		ulSetError ( UL_WARNING, "Map is not a power-of-two in size!" ) ;
    	return false ;
	}

	GLubyte *texels[20];   // One element per level of MIPmap.

	for (int l = 0; l < 20; l++) {
    	texels [l] = NULL;
	}

	texels[0] = image;

	int lev;

	for (lev = 0 ;((xsize >> (lev+1)) != 0 || (ysize >> (lev+1)) != 0); lev++) {
		// Suffix '1' is the higher level map, suffix '2' is the lower level.
		int l1 = lev;
		int l2 = lev+1;
		int w1 = xsize >> l1;
		int h1 = ysize >> l1;
		int w2 = xsize >> l2;
		int h2 = ysize >> l2;

		if (w1 <= 0) {
			w1 = 1;
		}
		if (h1 <= 0) {
			h1 = 1;
		}
		if (w2 <= 0) {
			w2 = 1;
		}
		if (h2 <= 0) {
			h2 = 1;
		}

		texels[l2] = new GLubyte[w2*h2*zsize];

		for (int x2 = 0; x2 < w2; x2++) {
			for (int y2 = 0; y2 < h2; y2++) {
				for (int c = 0; c < zsize; c++) {
					int x1 = x2 + x2;
					int x1_1 = (x1 + 1) % w1;
					int y1 = y2 + y2;
					int y1_1 = (y1 + 1) % h1;
					int t1 = texels[l1][(y1*w1 + x1)*zsize + c];
					int t2 = texels[l1][(y1_1*w1 + x1)*zsize + c];
					int t3 = texels[l1][(y1*w1 + x1_1)*zsize + c];
					int t4 = texels[l1][(y1_1*w1 + x1_1)*zsize + c];

					if (c == 3) { // Alpha.
						int a = t1;
						if (t2 > a) {
							a = t2;
						}
						if (t3 > a) {
							a = t3;
						}
						if (t4 > a) {
							a = t4;
						}
						texels[l2][(y2*w2 + x2)*zsize + c] = a;
					} else {
						texels[l2][(y2*w2 + x2)*zsize + c] = ( t1 + t2 + t3 + t4 )/4;
					}
				}
			}
		}
	}

	texels[lev + 1] = NULL;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST);
	int map_level = 0;

	GLint ww;

	GLint textureTargetFormat;
	if (isCompressARBEnabled()) {
		//printf("COMPRESSOR: ");

		switch (zsize) {
			case 1:
				textureTargetFormat = GL_COMPRESSED_LUMINANCE_ARB;
				//printf("GL_COMPRESSED_LUMINANCE_ARB\n");
				break;
			case 2:
				textureTargetFormat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
				//printf("GL_COMPRESSED_LUMINANCE_ALPHA_ARB\n");
				break;
			case 3:
				textureTargetFormat = GL_COMPRESSED_RGB_ARB;
				//printf("GL_COMPRESSED_RGB_ARB\n");
				break;
			default:
				textureTargetFormat = GL_COMPRESSED_RGBA_ARB;
				//printf("GL_COMPRESSED_RGBA_ARB\n");
				break;
		}
	} else {
		textureTargetFormat = zsize;
		//printf("NON COMPRESSOR\n");
	}

	int tlimit = getUserTextureMaxSize();

	do {
		if (xsize > tlimit || ysize > tlimit) {
			ww = 0;
		} else {

			glTexImage2D(GL_PROXY_TEXTURE_2D, map_level, textureTargetFormat, xsize, ysize, FALSE /* Border */,
									(zsize==1)?GL_LUMINANCE:
									(zsize==2)?GL_LUMINANCE_ALPHA:
									(zsize==3)?GL_RGB:
											GL_RGBA,
									GL_UNSIGNED_BYTE, NULL);

			glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &ww);
		}

		if (ww == 0) {
			delete [] texels[0];
			xsize >>= 1;
			ysize >>= 1;
			for (int l = 0; texels [l] != NULL; l++) {
				texels [l] = texels[l+1];
			}

			if (xsize < 8 && ysize < 8) {
				//ulSetError (UL_FATAL, "SSG: OpenGL will not accept a downsized version ?!?");
			}
		}
	} while (ww == 0);


	for (int i = 0; texels[i] != NULL; i++) {
		int w = xsize >> i;
		int h = ysize >> i;

		if (w <= 0) {
			w = 1;
		}
		if (h <= 0) {
			h = 1;
		}

		if (mipmap == TRUE || i == 0) {
			glTexImage2D  ( GL_TEXTURE_2D,
						map_level, textureTargetFormat, w, h, FALSE /* Border */,
								(zsize==1)?GL_LUMINANCE:
								(zsize==2)?GL_LUMINANCE_ALPHA:
								(zsize==3)?GL_RGB:
										GL_RGBA,
								GL_UNSIGNED_BYTE, (GLvoid *) texels[i] ) ;

			/*int compressed;
			glGetTexLevelParameteriv(GL_TEXTURE_2D, map_level, GL_TEXTURE_COMPRESSED_ARB, &compressed);
			if (compressed == GL_TRUE) {
				int csize;
				glGetTexLevelParameteriv(GL_TEXTURE_2D, map_level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &csize);
				printf("compression ratio: %d to %d\n", csize, w*h*zsize);
			} else {
				printf("not compressed\n");
			}*/
		}

		map_level++ ;
		delete [] texels[i];
	}

	return true;
}


