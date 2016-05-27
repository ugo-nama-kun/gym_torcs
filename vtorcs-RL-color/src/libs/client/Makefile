##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:41:33 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.3 2004/05/30 06:40:54 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY = libclient.so

SOLIBDIR  = .

SOURCES   = entry.cpp mainmenu.cpp splash.cpp exitmenu.cpp \
	    optionmenu.cpp

LIBS      =  -lplibssg -lplibsg -lplibul

EXPDIR    = include

EXPORTS   = client.h exitmenu.h

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}






