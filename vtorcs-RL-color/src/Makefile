##############################################################################
#
#    file                 : Makefile
#    created              : Mon Aug 30 22:10:08 CEST 1999
#    copyright            : (C) 1999 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Makefile,v 1.7 2003/08/20 05:48:42 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


# #### The Open Racing Car Simulator ####


SHIPSUBDIRS	= drivers modules libs raceman

EXPINCDIRS	= interfaces libs linux modules

TOOLSUBDIRS	= libs tools linux

SUBDIRS		= libs linux modules drivers tools

SRCDOC		= yes

PKGSUBDIRS	= interfaces libs linux modules windows tools drivers doc raceman

src_PKGFILES	= Makefile

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


