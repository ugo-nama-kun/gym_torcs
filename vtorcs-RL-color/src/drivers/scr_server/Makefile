##############################################################################
#
#    file                 : Makefile
#    created              : Tue Dec 18 23:40:54 CET 2007
#    copyright            : (C) 2002 Daniele Loiacono
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

ROBOT       = scr_server
MODULE      = ${ROBOT}.so
MODULEDIR   = drivers/${ROBOT}
#SOURCES     = ${ROBOT}.cpp sensors.cpp driver.cpp pit.cpp strategy.cpp cardata.cpp opponent.cpp spline.cpp learn.cpp SimpleParser.cpp CarControl.cpp 
SOURCES     = ${ROBOT}.cpp sensors.cpp SimpleParser.cpp CarControl.cpp spline.cpp trackdesc.cpp mycar.cpp pathfinder.cpp ObstacleSensors.cpp 

SHIPDIR     = drivers/${ROBOT}
SHIP        = ${ROBOT}.xml 
SHIPSUBDIRS = 1 2 3 4 5 6 7 8 9 10

PKGSUBDIRS  = ${SHIPSUBDIRS}
src-robots-scr_server_PKGFILES = $(shell find * -maxdepth 0 -type f -print)
src-robots-scr_server_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

include ${MAKE_DEFAULT}

