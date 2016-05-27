##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:38:24 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.10 2003/05/18 20:41:27 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY = libtgf.so

SOLIBDIR  = .

SOURCES   = 	module.cpp \
		os.cpp \
		params.cpp \
		tgf.cpp \
		trace.cpp \
		directory.cpp \
		profiler.cpp \
		hash.cpp


EXPDIR  = include

EXPORTS = tgf.h os.h

SHIPDIR = config

SHIP    = params.dtd

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}



