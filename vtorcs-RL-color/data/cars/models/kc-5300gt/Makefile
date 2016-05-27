##############################################################################
#
#    file                 : Makefile
#    created              : Sun Feb 18 21:02:29 /etc/localtime 2001
#    copyright            : (C) 2000 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.2 2004/11/06 14:41:46 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

CAR       = kc-5300gt

DATADIR   = cars/$(CAR)

DATA      = $(shell find * -maxdepth 0 -type f -print)

DATA_PKG  = 	$(shell find *.acc -maxdepth 0 -type f -print) \
		$(shell find *.rgb -maxdepth 0 -type f -print) \
		$(shell find *.xml -maxdepth 0 -type f -print) \
		$(shell find *.txt -maxdepth 0 -type f -print)

data-cars-kcendra-gt_PKGFILES	= $(DATA_PKG) $(shell find *.xcf -maxdepth 0 -type f -print)

data-cars-kcendra-gt_PKGDIR	= $(DATADIR)

include ${MAKE_DEFAULT}


