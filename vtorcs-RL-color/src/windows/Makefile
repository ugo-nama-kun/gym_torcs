##############################################################################
#
#    file                 : Makefile
#    created              : Tue Jun 25 00:02:31 CEST 2002
#    copyright            : (C) 2001 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.2 2002/06/30 14:11:17 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


PKGSUBDIRS	= $(shell find * -maxdepth 0 -type d -print | grep -v CVS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


