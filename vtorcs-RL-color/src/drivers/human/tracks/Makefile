##############################################################################
#
#    file                 : Makefile
#    created              : Mon Apr 10 22:11:53 CEST 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.3 2002/06/30 14:11:15 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

SHIPSUBDIRS = $(shell find * -maxdepth 0 -type d -print | grep -v CVS)

PKGSUBDIRS	= $(SHIPSUBDIRS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


