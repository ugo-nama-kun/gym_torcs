##############################################################################
#
#    file                 : Makefile
#    created              : Tue Jun 12 13:41:04 CEST 2012
#    copyright            : (C) 2002 Giuseppe Cuccu
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = img_server
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
SOURCES     = ${ROBOT}.cpp driver.cpp #SimpleParser.cpp # TAKEN FROM SCR_SERVER!!

SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml cg-nascar-rwd.rgb logo.rgb
SHIPSUBDIRS = 

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-img_server_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-img_server_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}
