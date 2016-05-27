##############################################################################
#
#    file                 : Makefile
#    created              : Tue Oct 9 16:49:45
#    copyright            : (C) 2007 Eric Espié, Bernhard Wymann
#    email                : berniw@bluewin.ch
#    version              : $Id: Makefile,v 1.1.2.1 2008/09/03 21:51:22 berniw Exp $
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

CAR	  = car6-trb1

DATADIR   = cars/$(CAR)

DATA      = $(CAR).xml \
            $(CAR).rgb shadow.rgb tex-wheel.rgb driver.rgb \
            $(shell find *.acc -maxdepth 0 -type f -print) \
	    	readme.txt 

data_PKGFILES	= $(DATA) $(shell find *.xcf -maxdepth 0 -type f -print)

data_PKGDIR		= $(DATADIR)

include ${MAKE_DEFAULT}


