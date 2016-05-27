##############################################################################
#
#    file                 : Makefile
#    created              : Sun Mar 12 14:55:13 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.4.2.1 2008/09/03 21:51:22 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


DATADIR   = cars/p406

DATA      = 	$(shell find *.xml -maxdepth 0 -type f -print) \
		$(shell find *.acc -maxdepth 0 -type f -print) \
		$(shell find *.rgb -maxdepth 0 -type f -print)

data_PKGFILES	= $(DATA) $(shell find *.xcf -maxdepth 0 -type f -print)

data_PKGDIR		= $(DATADIR)

include ${MAKE_DEFAULT}


