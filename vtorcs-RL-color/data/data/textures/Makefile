##############################################################################
#
#    file                 : Makefile
#    created              : Tue Aug 31 22:26:20 CEST 1999
#    copyright            : (C) 1999 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.2 2002/06/25 21:47:28 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################



DATADIR   = data/textures

DATA      = $(shell find *.png -maxdepth 0 -print) $(shell find *.rgb -maxdepth 0 -print)

data_PKGFILES	= $(DATA)

data_PKGDIR	= $(DATADIR)


include ${MAKE_DEFAULT}


