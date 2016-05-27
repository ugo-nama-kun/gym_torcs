##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:53:52 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.12.2.1 2011/12/28 15:05:11 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

TOOLSDIR    = 

TOOLSCRIPT  = torcs

PROGRAM = torcs-bin

SOURCES = main.cpp linuxspec.cpp

LDFLAGS = -rdynamic

SOLIBS  =  -lclient       \
	   -lconfscreens  \
	   -ltgf          \
	   -ltgfclient    \
	   -ltxml	  \
	   -lplibul	  \
	   -lraceengine   \
	   -lracescreens  \
	   -lrobottools   \
	   -llearning

EXPDIR  = include

EXPORTS = osspec.h

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}




