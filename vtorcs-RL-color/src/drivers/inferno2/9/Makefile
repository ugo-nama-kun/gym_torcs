##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:22:26 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.1 2003/10/19 19:28:21 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

SHIPDIR   = drivers/inferno2/9

SHIP      = $(shell find * -maxdepth 0 -type f -print)

src-robots-base_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src-robots-base_PKGDIR		= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

# Recursion

SHIPSUBDIRS = $(shell find * -maxdepth 0 -type d -print | grep -v CVS)

PKGSUBDIRS  = ${SHIPSUBDIRS}

include ${MAKE_DEFAULT}


