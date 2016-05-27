##############################################################################
#
#    file        : Makefile
#    created     : Sat Nov 16 09:38:51 CET 2002
#    copyright   : (C) 2002, 3006 by Eric Espié, Bernhard Wymann
#    email       : Eric.Espie@torcs.org   
#    version     : $Id: Makefile,v 1.5 2006/10/06 20:44:51 berniw Exp $
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY    = libraceengine.so

SOLIBDIR     = .

SOURCES      = singleplayer.cpp raceinit.cpp racemain.cpp racemanmenu.cpp racestate.cpp racegl.cpp \
	       raceengine.cpp raceresults.cpp

EXPDIR       = include

EXPORTS      = singleplayer.h raceinit.h

SHIPDIR      = config

SHIP         = raceengine.xml style.xsl

CONFIGCREATEDIRS = ${SHIPDIR}

CONFIG    = $(SHIP)


src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))


include ${MAKE_DEFAULT}
