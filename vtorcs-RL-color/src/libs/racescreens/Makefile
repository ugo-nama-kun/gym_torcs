 ##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:32:07 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.2 2003/06/24 21:02:24 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY = libracescreens.so

SOLIBDIR  = .

SOURCES   = trackselect.cpp driverselect.cpp results.cpp pitmenu.cpp loadingscreen.cpp racemenu.cpp miscscreens.cpp fileselect.cpp

EXPDIR    = include

EXPORTS   = racescreens.h

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


