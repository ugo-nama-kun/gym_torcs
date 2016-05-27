##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:11:09 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.6 2002/12/01 00:47:01 scr_server Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


ROBOT = scr_server

SHIPDIR   = drivers/${ROBOT}/8

SHIP      = car1-trb1.rgb $(shell find *.xml -maxdepth 0 -type f -print)

src-robots-scr_server_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src-robots-scr_server_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}




