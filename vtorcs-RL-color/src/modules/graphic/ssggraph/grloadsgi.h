/***************************************************************************

    file                 : grloadsgi.h
    created              : Wed Jun 1 14:56:31 CET 2005
    copyright            : (C) 2005 by Bernhard Wymann
    version              : $Id: grloadsgi.h,v 1.1 2005/06/03 23:51:20 berniw Exp $

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
	Contains the header of the ssgSGIHeader class to allow me to overload,
	the source is the file ssgLoadSGI.cxx. This is a stupid solution, but
	the architecture of plib forces that... really.
*/

#ifndef _GRLOADSGI_H_
#define _GRLOADSGI_H_

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

class ssgSGIHeader {
	public:
		unsigned short magic ;
		int            max ;
		int            min ;
		int            colormap ;
		char           type ;
		char           bpp ;
		unsigned int  *start ;
		int           *leng ;
		unsigned short dim ;
		unsigned short xsize ;
		unsigned short ysize ;
		unsigned short zsize ;
		int           tablen ;

		/* added by J. Nathan Matias */
		FILE          *image_fd;
		char          image_fname [ 512 ];
		int           isSwapped;
		unsigned char *rle_temp;
		bool          loadSGI_bool;
		ssgSGIHeader () ;
		ssgSGIHeader(const char *fname, ssgTextureInfo* info );
		~ssgSGIHeader () ;

		bool openFile(const char *fname);
		void makeConsistant () ;
		void getRow   ( unsigned char *buf, int y, int z ) ;
		void getPlane ( unsigned char *buf, int z ) ;
		void getImage ( unsigned char *buf ) ;
		void readHeader () ;

	protected:
		void swab_short ( unsigned short *x );
		void swab_int ( unsigned int *x );
		void swab_int_array ( int *x, int leng );
		unsigned char readByte ();
		unsigned short readShort ();
		unsigned int readInt ();
};

#endif // _GRLOADSGI_H_
