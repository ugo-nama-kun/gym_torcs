ROBOT = berniw3

MODULE = ${ROBOT}.so

MODULEDIR = drivers/${ROBOT}

SOURCES   = ${ROBOT}.cpp spline.cpp trackdesc.cpp pathfinder.cpp mycar.cpp

SHIPDIR   = drivers/${ROBOT}

SHIP      = ${ROBOT}.xml logo.rgb

SHIPSUBDIRS = 1 2 3 4 5 6 7 8 9 10

PKGSUBDIRS	= $(SHIPSUBDIRS)

src-robots-base_PKGFILES	= $(shell find * -maxdepth 0 -type f -print)

src-robots-base_PKGDIR	= ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))

# Mandatory Line Do Not Remove
include ${MAKE_DEFAULT}




