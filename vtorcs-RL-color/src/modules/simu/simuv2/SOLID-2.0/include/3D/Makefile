##############################################################################
#
#    file                 : Makefile
#    created              : Sun Apr  1 15:22:35 CEST 2001
#    copyright            : (C) 2001 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.2 2002/06/25 21:47:33 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################



EXPORTS      = Basic.h  Matrix.h  Point.h  Quaternion.h  Tuple3.h  Tuple4.h  Vector.h

EXPDIR       = include/3D

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


