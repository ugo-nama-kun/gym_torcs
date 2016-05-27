##############################################################################
#
#    file                 : Makefile
#    created              : Sun Feb 18 21:02:29 /etc/localtime 2001
#    copyright            : (C) 2000 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.1 2003/09/21 10:01:48 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################



DATADIR   = cars/pw-206wrc

DATA      = $(shell find *.xml -maxdepth 0 -type f -print) \
	    $(shell find *.rgb -maxdepth 0 -type f -print) \
	    $(shell find *.acc -maxdepth 0 -type f -print)  \
	    $(shell find *.txt -maxdepth 0 -type f -print)

data-cars-Patwo-Design_PKGFILES	= $(DATA)

data-cars-Patwo-Design_PKGDIR	= $(DATADIR)

include ${MAKE_DEFAULT}


