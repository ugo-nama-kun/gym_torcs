##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:15:13 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.7 2003/03/05 01:16:12 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


ROBOT = human

MODULE    = ${ROBOT}.so

MODULEDIR = drivers/${ROBOT}

LIBS      =  -lplibul

SOURCES   = ${ROBOT}.cpp pref.cpp

SHIPDIR   = drivers/${ROBOT}

SHIP      = $(shell find *.xml -maxdepth 0 -type f -print) $(shell find *.rgb -maxdepth 0 -type f -print)

CONFIGCREATEDIRS = ${SHIPDIR}

CONFIG    = $(shell find *.xml -maxdepth 0 -type f -print)

SHIPSUBDIRS = tracks

PKGSUBDIRS	= $(SHIPSUBDIRS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


