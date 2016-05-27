 ##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:32:07 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.5 2005/06/03 23:51:19 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY = libconfscreens.so

SOLIBDIR  = .

SOURCES   = controlconfig.cpp  graphconfig.cpp mouseconfig.cpp driverconfig.cpp joystickconfig.cpp\
			simuconfig.cpp soundconfig.cpp openglconfig.cpp

EXPDIR    = include

EXPORTS   = confscreens.h driverconfig.h joystickconfig.h mouseconfig.h controlconfig.h graphconfig.h\
			simuconfig.h soundconfig.h openglconfig.h

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


