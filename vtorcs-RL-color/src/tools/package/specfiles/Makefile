##############################################################################
#
#    file                 : Makefile<4>
#    created              : Mon Jul 15 21:21:27 CEST 2002
#    copyright            : (C) 2001 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.1 2002/07/15 23:02:49 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SPECFILES = $(shell find *.spec -maxdepth 0 -type f -print) build_rpms.sh build_data_rpms.sh

GARBAGE = $(SPECFILES)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


