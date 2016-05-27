##############################################################################
#
#    file                 : Makefile
#    created              : Sat Dec 23 09:19:01 CET 2000
#    copyright            : (C) 2000 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: Makefile,v 1.2 2003/11/23 20:21:14 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


TOOLSDIR	= 

TOOLSCRIPT	= nfsperf

PROGRAM		= nfsperf-bin

SOURCES		= mainnfsperf.cpp

SOLIBS		= -ltgf -ltxml

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}


