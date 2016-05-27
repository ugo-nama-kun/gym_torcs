##############################################################################
#
#    file                 : Makefile.orig
#    created              : Sun Apr  1 15:25:55 CEST 2001
#    copyright            : (C) 2001 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.3 2002/10/13 22:03:38 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


LIBRARY      =	libsolid.a

SOURCES      = 	Transform.cpp Convex.cpp Box.cpp Cone.cpp Cylinder.cpp Sphere.cpp \
		Simplex.cpp Polygon.cpp Polyhedron.cpp Complex.cpp \
		BBoxTree.cpp Endpoint.cpp Object.cpp Response.cpp RespTable.cpp \
		C-api.cpp

LIBDIR	     =	lib

COMPILFLAGS  =	-DNDEBUG -Wno-deprecated

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

