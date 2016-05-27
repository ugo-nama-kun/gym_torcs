##############################################################################
#
#    file        : Makefile
#    created     : Wed Nov  3 20:31:16 CET 2004
#    copyright   : (C) 2004 by Eric Espié                        
#    email       : Eric.Espie@free.fr   
#    version     : $Id: Makefile,v 1.5 2005/08/05 09:02:58 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


SOLIBRARY    = liblearning.so

SOLIBDIR     = .

SOURCES      = policy.cpp ann_policy.cpp List.cpp string_utils.cpp Distribution.cpp MathFunctions.cpp ANN.cpp

EXPORTS      = policy.h ann_policy.h ANN.h learn_debug.h real.h string_utils.h List.h MathFunctions.h Distribution.h

EXPDIR       = include/learning

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))


include ${MAKE_DEFAULT}


