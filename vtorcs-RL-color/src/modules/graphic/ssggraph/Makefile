##############################################################################
#
#    file                 : Makefile
#    created              : Mon Aug 30 21:40:19 CEST 1999
#    copyright            : (C) 1999 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.19 2005/07/20 01:52:30 olethros Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOURCES   = grmain.cpp grboard.cpp ssggraph.cpp grcam.cpp grcar.cpp grscreen.cpp\
	grscene.cpp grutil.cpp grshadow.cpp grsmoke.cpp grskidmarks.cpp grloadac.cpp\
	grmultitexstate.cpp grvtxtable.cpp grtrackmap.cpp grtexture.cpp grcarlight.cpp CarSoundData.cpp TorcsSound.cpp PlibSoundInterface.cpp OpenalSoundInterface.cpp grsound.cpp SoundInterface.cpp

MODULE    = ssggraph.so

MODULEDIR = modules/graphic

LIBS      = -lplibsl -lplibsm -lplibsg -lplibssg -lplibul -lplibssgaux -lopenal

SHIPDIR   = config

SHIP      = graph.xml sound.xml

CONFIGCREATEDIRS = ${SHIPDIR}

CONFIG    = $(SHIP)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


