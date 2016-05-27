##############################################################################
#
#    file                 : Makefile
#    created              : Sun Feb 18 21:02:29 /etc/localtime 2001
#    copyright            : (C) 2000 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.6 2004/11/06 14:41:45 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

CAR       = baja-bug

DATADIR   = cars/baja-bug

DATA      = $(CAR).xml readme.txt \
            $(CAR).rgb tex-wheel.rgb shadow.rgb \
            $(shell find *.acc -maxdepth 0 -type f -print)

data-cars-extra_PKGFILES	= $(DATA) $(shell find *.xcf -maxdepth 0 -type f -print)

data-cars-extra_PKGDIR		= $(DATADIR)

include ${MAKE_DEFAULT}


