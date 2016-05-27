##############################################################################
#
#    file        : makefile
#    created     : Tue Jul 29 19:42:26 CEST 2003
#    copyright   : (C) 2003 by Christophe Guionneau                        
#    version     : $Id: Makefile,v 1.4 2003/11/23 20:21:12 torcs Exp $                                  
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

TOOLSCRIPT	= nfs2ac

PROGRAM		= nfs2ac-bin

SOURCES		= mainnfs2ac.cpp

src_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

