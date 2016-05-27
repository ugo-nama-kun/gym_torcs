##############################################################################
#
#    file                 : Makefile
#    created              : Sun Mar 12 14:55:13 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.9 2004/11/06 14:41:45 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

CAR       = buggy

DATADIR   = cars/$(CAR)

DATA      = $(CAR).xml \
            $(CAR).rgb tex-wheel.rgb shadow.rgb \
            $(shell find *.acc -maxdepth 0 -type f -print)

data-cars-extra_PKGFILES	= $(DATA) $(shell find *.xcf -maxdepth 0 -type f -print)

data-cars-extra_PKGDIR		= $(DATADIR)

include ${MAKE_DEFAULT}


