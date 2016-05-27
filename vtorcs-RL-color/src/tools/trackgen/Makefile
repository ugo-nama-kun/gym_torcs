##############################################################################
#
#    file                 : Makefile
#    created              : Sat Dec 23 09:19:01 CET 2000
#    copyright            : (C) 2000 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.9 2003/11/23 20:21:15 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################



TOOLSDIR	= 

TOOLSCRIPT	= trackgen

PROGRAM      	= trackgen-bin

DATADIR      	= tools/trackgen

DATA         	= trackgen.xml

SOURCES      	= maintrackgen.cpp track.cpp easymesh.cpp ../../linux/linuxspec.cpp relief.cpp elevation.cpp objects.cpp ac3d.cpp util.cpp

SOLIBS       	= -ltgf -ltgfclient -lplibssg -lplibsg -lplibul -lrobottools -ltxml

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


