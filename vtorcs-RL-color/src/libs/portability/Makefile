##############################################################################
#
#    file        : Makefile
#    created     : Fri Jul 8 15:19:34 CET 2005
#    copyright   : (C) 2005 Bernhard Wymann
#    email       : berniw@bluewin.ch
#    version     : $Id: Makefile,v 1.1 2005/07/08 14:18:16 berniw Exp $
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


#SOLIBRARY    =

#SOLIBDIR     = .

#SOURCES      =

EXPORTS      = portability.h

EXPDIR       = include

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))


include ${MAKE_DEFAULT}


