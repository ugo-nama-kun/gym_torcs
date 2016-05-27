##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:11:09 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.2.2.2 2008/06/12 23:09:47 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


ROBOT = inferno

SHIPDIR   = drivers/${ROBOT}/7

SHIP      = car5-trb1.rgb $(shell find *.xml -maxdepth 0 -type f -print)

src-robots-base_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src-robots-base_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}




