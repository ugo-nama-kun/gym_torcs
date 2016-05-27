##############################################################################
#
#    file                 : Makefile
#    created              : Mon Apr 10 22:13:04 CEST 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.4 2003/03/05 01:16:13 torcs Exp $                                  
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

SHIPDIR   = drivers/${ROBOT}/tracks/dirt-2

SHIP      = $(shell find *.xml -maxdepth 0 -type f -print)

CONFIGCREATEDIRS = ${SHIPDIR}

CONFIG    = $(SHIP)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


