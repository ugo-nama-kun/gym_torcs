##############################################################################
#
#    file        : Makefile
#    created     : Fri Jul 15 11:16:37 CET 2005
#    copyright   : (C) 2005 Bernhard Wymann
#    email       : berniw@bluewin.ch
#    version     : $Id: Makefile,v 1.2 2005/08/05 09:04:53 berniw Exp $
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

EXPORTS      = linalg_t.h  v2_t.h  v3_t.h  v4_t.h straight2_t.h

EXPDIR       = include/tmath

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))


include ${MAKE_DEFAULT}


