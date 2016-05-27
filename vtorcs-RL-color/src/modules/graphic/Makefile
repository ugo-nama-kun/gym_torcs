##############################################################################
#
#    file                 : Makefile<3>
#    created              : Sat Mar 18 23:55:17 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.5 2004/06/09 17:53:56 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


#SUBDIRS		= $(shell find * -maxdepth 0 -type d -print | grep -v CVS)

SUBDIRS		= ssggraph

SHIPSUBDIRS     = $(SUBDIRS)

PKGSUBDIRS	= $(SUBDIRS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


