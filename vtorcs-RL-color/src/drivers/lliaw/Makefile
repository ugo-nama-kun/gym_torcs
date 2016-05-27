##############################################################################
#
#    file                 : Makefile
#    created              : Wed Jan 8 18:31:16 CET 2003
#    copyright            : (C) 2002-2008 Bernhard Wymann
#    email                : berniw@bluewin.ch
#    version              : $Id: Makefile,v 1.6.6.2 2008/05/31 09:25:28 berniw Exp $
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT = lliaw

MODULE = ${ROBOT}.so

MODULEDIR = drivers/${ROBOT}

SOURCES   = ${ROBOT}.cpp spline.cpp trackdesc.cpp pathfinder.cpp mycar.cpp

SHIPDIR   = drivers/${ROBOT}

SHIP      = ${ROBOT}.xml logo.rgb

SHIPSUBDIRS = 1 2 3 4 5 6 7 8 9 10

PKGSUBDIRS	= $(SHIPSUBDIRS)

src-robots-base_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src-robots-base_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

# Mandatory Line Do Not Remove
include ${MAKE_DEFAULT}
