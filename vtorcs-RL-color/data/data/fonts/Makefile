##############################################################################
#
#    file                 : Makefile
#    created              : Sat Mar 18 23:06:20 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.3 2003/05/18 20:41:07 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


DATADIR   = data/fonts

DATA      = $(shell find *.glf -maxdepth 0 -type f -print)

data_PKGFILES	= $(DATA)

data_PKGDIR	= $(DATADIR)

include ${MAKE_DEFAULT}


