##############################################################################
#
#    file                 : Makefile
#    created              : Sun Jan 30 22:43:26 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.5 2003/01/05 20:22:33 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

SUBDIRS		= simu track graphic telemetry

SHIPSUBDIRS	= telemetry graphic

EXPINCDIRS      = simu

PKGSUBDIRS	= track simu graphic telemetry

src_PKGFILES	= Makefile

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


