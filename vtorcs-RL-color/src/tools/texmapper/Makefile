##############################################################################
#
#    file                 : Makefile
#    created              : Mon Nov 13 21:48:27 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.7 2003/11/23 20:21:15 torcs Exp $                                  
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

TOOLSCRIPT	= texmapper

PROGRAM   	= texmapper-bin

SOURCES   	= maintexmapper.cpp ssgSaveAC.cpp

SOLIBS    	= -lplibssg -lplibsg -lplibul -ltgf -ltgfclient -ltxml -lpng

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
