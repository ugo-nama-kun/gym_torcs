##############################################################################
#
#    file                 : Makefile
#    created              : Sun Mar 12 14:46:15 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.2.2.1 2008/09/03 21:51:22 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


DATADIR   = categories

DATA      = $(shell find * -maxdepth 0 -type f -name '*.xml')

data_PKGFILES	= $(DATA)

data_PKGDIR		= $(DATADIR)

include ${MAKE_DEFAULT}


