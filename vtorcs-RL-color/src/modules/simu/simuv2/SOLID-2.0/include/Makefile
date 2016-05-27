##############################################################################
#
#    file                 : Makefile
#    created              : Sun Apr  1 15:20:27 CEST 2001
#    copyright            : (C) 2001 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.3 2002/06/28 20:13:33 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################



EXPINCDIRS	= 3D SOLID

PKGSUBDIRS	= $(EXPINCDIRS)

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


