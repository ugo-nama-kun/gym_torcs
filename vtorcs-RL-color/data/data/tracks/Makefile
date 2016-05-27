##############################################################################
#
#    file                 : Makefile
#    created              : Sun Mar 19 00:15:04 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.3 2002/06/25 21:47:28 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


DATADIR   = data/tracks

DATA      = $(shell find *.xml -maxdepth 0 -type f -print)


data_PKGFILES	= $(DATA)

data_PKGDIR	= $(DATADIR)


include ${MAKE_DEFAULT}


