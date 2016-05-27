##############################################################################
#
#    file                 : Make-default.mk
#    created              : Sat Mar 18 23:53:11 CET 2000
#    copyright            : (C) 2000 by Eric Espie                         
#    email                : torcs@free.fr   
#    version              : $Id: Make-default.mk,v 1.33 2003/09/07 08:13:56 torcs Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################

# targets:
#	 distclean
#	 clean
#	 default
#	 install
#	 datainstall
#	 win32setup
#	 packages

ifndef TORCS_BASE

-include ${HOME}/.torcs.rc

ifndef TORCS_BASE

error:
	@echo "TORCS_BASE should be defined"
	@exit 1

endif
endif

-include ${TORCS_BASE}/Make-config

SOURCEBASE  = ${TORCS_BASE}/src
EXPORTBASE  = ${TORCS_BASE}/export
DOCBASE     = ${TORCS_BASE}/doc

#data
INSTDATABASE = ${DESTDIR}${datadir}
#launch scripts
INSTBINBASE  = ${DESTDIR}${bindir}
#binaries
INSTLIBBASE  = ${DESTDIR}${libdir}
#var
INSTVARBASE  = ${DESTDIR}${datadir}


PACKAGEBASE   = ${TORCS_BASE}/package
PACKAGESBASE  = ${TORCS_BASE}/RPM/SOURCES
SPECFILESBASE = ${TORCS_BASE}/RPM/SPECS

# linux user setup
SETUP_LINUX	 = ${TORCS_BASE}/setup_linux.sh

# win32
INIT_WIN32       = ${TORCS_BASE}/setup_win32.bat
INIT_WIN32_D     = ${TORCS_BASE}/setup_win32_debug.bat
DATA_WIN32       = ${TORCS_BASE}/setup_win32-data-from-CVS.bat
DATA_WIN32_D     = ${TORCS_BASE}/setup_win32-data-from-CVS_debug.bat


define create_dir_win32_
TotDir=`echo $$createdir | sed -e "s:${TORCS_BASE}/::g" ` ; \
CurDir='.' ; \
echo "" >> ${INIT_WIN32} ; \
for Dir in `echo $$TotDir | sed -e 's:/: :g' ` ; \
do CurDir=$$CurDir/$$Dir ; \
echo "call .\\create_dir $$CurDir" >> ${INIT_WIN32} ; \
done
endef

define create_dir_win32
TotDir=`echo $$createdir | sed -e "s:${TORCS_BASE}/::g" ` ; \
CurDir='.' ; \
echo "" >> ${INIT_WIN32} ; \
for Dir in `echo $$TotDir | sed -e 's:/: :g' ` ; \
do CurDir=$$CurDir/$$Dir ; \
echo "if exist $$D\\*.* call .\\create_dir $$CurDir" >> ${INIT_WIN32} ; \
done
endef

define create_dir_win32_data
TotDir=`echo $$createdir | sed -e "s:${TORCS_BASE}/::g" ` ; \
CurDir='.' ; \
echo "" >> ${DATA_WIN32} ; \
for Dir in `echo $$TotDir | sed -e 's:/: :g' ` ; \
do CurDir=$$CurDir/$$Dir ; \
echo "if exist $$D\\*.* call .\\create_dir $$CurDir" >> ${DATA_WIN32} ; \
done
endef


OB1 = $(SOURCES:.cpp=.o)
OBJECTS = $(OB1:.c=.o)

define recursedirs
for Dir in $$RecurseDirs ;\
do ${MAKE} -C $$Dir $$RecurseFlags TORCS_BASE=${TORCS_BASE} MAKE_DEFAULT=${MAKE_DEFAULT}; \
if [ $$? != 0 ]; then exit 1; fi ; \
done
endef

COMPILATION=
ifdef LIBRARY
COMPILATION=true
endif
ifdef PROGRAM
COMPILATION=true
endif
ifdef MODULE
COMPILATION=true
endif
ifdef SOLIBRARY
COMPILATION=true
endif
ifdef TOOLS
COMPILATION=true
endif

ifdef COMPILATION

default: exports tools compil

ifeq (.depend,$(wildcard .depend))
include .depend
endif

.depend:
	$(CPP) $(INCFLAGS) $(CFLAGSD) $(COMPILFLAGS)  $(SOURCES) > .depend

dep:	.depend

compil: subdirs dep ${LIBRARY} ${SOLIBRARY} ${MODULE} ${PROGRAM}

else

default: exports tools subdirs

compil: subdirs ${LIBRARY} ${SOLIBRARY} ${MODULE} ${PROGRAM}

endif

win32start:
	@rm -f ${INIT_WIN32}
	@echo '@echo off' > ${INIT_WIN32}
	@echo 'echo Checking directories ...' >> ${INIT_WIN32}
	@echo '' >> ${INIT_WIN32}
	@echo 'call .\create_dir .\export' >> ${INIT_WIN32}
	@echo 'call .\create_dir .\export\lib' >> ${INIT_WIN32}
	@echo 'call .\create_dir .\export\libd' >> ${INIT_WIN32}
	@rm -f ${DATA_WIN32}
	@echo '@echo off' > ${DATA_WIN32}
	@echo '' >> ${DATA_WIN32}


win32end:
	@sed -e "s:${TORCS_BASE}:\.:g"  -e 's/$$//' ${INIT_WIN32} > ${INIT_WIN32}.eee
	@mv ${INIT_WIN32}.eee ${INIT_WIN32}
	@sed -e "s:/src/linux/:/src/windows/:g" ${INIT_WIN32} > ${INIT_WIN32}.eee
	@mv ${INIT_WIN32}.eee ${INIT_WIN32}
	@sed -e "s:/:\\\:g" ${INIT_WIN32} > ${INIT_WIN32}.eee
	@mv ${INIT_WIN32}.eee ${INIT_WIN32}
	@sed -e "s:runtime:runtimed:g" ${INIT_WIN32} > ${INIT_WIN32_D}
	@sed -e "s:${TORCS_BASE}:\.:g" ${DATA_WIN32} > ${DATA_WIN32}.eee
	@mv ${DATA_WIN32}.eee ${DATA_WIN32}
	@sed -e "s:/:\\\:g" ${DATA_WIN32} > ${DATA_WIN32}.eee
	@mv ${DATA_WIN32}.eee ${DATA_WIN32}
	@sed -e "s:runtime:runtimed:g" ${DATA_WIN32} > ${DATA_WIN32_D}


win32setup: win32start exportswin32 installshipswin32 installwin32 win32datainstall win32end


linuxconfstart:
	@rm -f ${SETUP_LINUX}
	@echo '#! /bin/bash' >> ${SETUP_LINUX}
	@echo '' >> ${SETUP_LINUX}
	@echo '[ -z "$$1" ] && exit 1' >> ${SETUP_LINUX}
	@echo '[ ! -d "$$1" ] && exit 1' >> ${SETUP_LINUX}
	@echo '' >> ${SETUP_LINUX}

linuxconfend:
	@sed -e "s:${TORCS_BASE}:\.:g" ${SETUP_LINUX} > ${SETUP_LINUX}.eee
	@mv ${SETUP_LINUX}.eee ${SETUP_LINUX}

linuxsetup: linuxconfstart installconfdirs linuxconfend

userconfinstall: installconfdirs installconfmkdir installconf

.PHONY : clean tools toolsdirs subdirs expincdirs exports export compil cleantools cleancompil \
 datadirs shipdirs doc win32start win32end installship installships installshipdirs installshipexecdirs installshipexec \
 installshipexecwin32dirs installshipexecwin32

# Recursive targets

exports: expincdirs export

installships: installshipdirs installshipexecdirs installship installshipexec installshipmkdir

exportswin32: expincwin32dirs exportwin32

installshipswin32: installshipwin32dirs installshipexecwin32dirs installshipwin32 installshipexecwin32 installshipmkdirwin32

tools: toolsdirs ${TOOLS} ${LOCALTOOLS} toolsdata

clean: cleancompil cleantools
	-rm -f ${LIBRARY} ${OBJECTS} ${PROGRAM} .depend ${SOLIBRARY} ${MODULE} ${GARBAGE} *~

cleantools: cleantoolsdirs
	-rm -f  ${TOOLS} ${LOCALTOOLS} .depend ${GARBAGE} *~

cleancompil: cleansubdirs
	-rm -f ${LIBRARY} ${OBJECTS} ${PROGRAM} .depend ${SOLIBRARY} ${MODULE} ${GARBAGE} *~

install: installdirs installship installshipexec installshipmkdir installsolibrary installmodule installprogram installtools installtoolsdata

installwin32: installwin32dirs installsolibrarywin32 installmodulewin32

datainstall: installdatadirs installdata

win32datainstall: installwin32datadirs installwin32data

packages: win32setup linuxsetup specfiles packagelist

onepackage: packagedirs packagefiles

specfiles: installspecfiles specfilesdirs

.SUFFIXES: .cpp

.cpp.o:
	${CXX} $(INCFLAGS) $(CXXFLAGS) $(COMPILFLAGS) -c $<

.SUFFIXES: .c

.c.o:
	${CXX} $(INCFLAGS) $(CXXFLAGS) $(COMPILFLAGS) -c $<


#	${CC} $(INCFLAGS) $(CFLAGS) $(COMPILFLAGS) -c $<


ifdef LIBRARY

${LIBRARY}: ${OBJECTS}
	${AR} ${ARFLAGS} ${LIBRARY} ${OBJECTS}
	${RANLIB} ${LIBRARY}
	@D=`pwd` ; \
	createdir="${EXPORTBASE}/${LIBDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${LIBRARY}" ; \
	echo " Exporting $$X to $$createdir/$$X"; \
	ln -sf $$D/$$X $$createdir/$$X

endif

ifdef DATA

installdata: $(DATA)
	@D=`pwd` ; \
	createdir="${INSTDATABASE}/${DATADIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	for X in $? ; \
	do echo " $(INSTALL_DATA) $$X $$createdir/$$X"; \
	$(INSTALL_DATA) $$X $$createdir/$$X ; \
	done

installwin32data: $(DATA)
	@D=`pwd` ; \
	createdir="runtime/${DATADIR}" ; \
	ext="0" ; \
	${create_dir_win32_data} ; \
	for X in $? ; \
	do echo "copy $$D/$$X ./runtime/${DATADIR}/$$X"; \
	echo "if exist $$D/$$X copy $$D/$$X ./runtime/${DATADIR}/$$X" >> ${DATA_WIN32} ; \
	done ;

else

installdata: ;

installwin32data: ;

endif

ifdef SHIP

installship: $(SHIP)
	@D=`pwd` ; \
	createdir="${INSTDATABASE}/${SHIPDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	for X in $? ; \
	do echo " $(INSTALL_DATA) $$X $$createdir/$$X"; \
	$(INSTALL_DATA) $$X $$createdir/$$X ; \
	done

installshipwin32: $(SHIP)
	@D=`pwd` ; \
	createdir="runtime/${SHIPDIR}" ; \
	${create_dir_win32} ; \
	for X in $? ; \
	do echo "copy $$D/$$X ./runtime/${SHIPDIR}/$$X" ; \
	echo "if exist $$D/$$X copy $$D/$$X ./runtime/${SHIPDIR}/$$X" >> ${INIT_WIN32} ; \
	done ; 

else

installship: ;

installshipwin32: ;

endif

ifdef SHIPEXEC

installshipexec: $(SHIPEXEC)
	@D=`pwd` ; \
	createdir="${INSTLIBBASE}/${SHIPEXECDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	for X in $? ; \
	do echo " $(INSTALL_SCRIPT) $$X $$createdir/$$X"; \
	$(INSTALL_SCRIPT) $$X $$createdir/$$X ; \
	done

installshipexecwin32: $(SHIPEXEC)
	@D=`pwd` ; \
	createdir="runtime/${SHIPEXECDIR}" ; \
	${create_dir_win32} ; \
	for X in $? ; \
	do echo "copy $$D/$$X ./runtime/${SHIPEXECDIR}/$$X" ; \
	echo "if exist $$D/$$X copy $$D/$$X ./runtime/${SHIPEXECDIR}/$$X" >> ${INIT_WIN32} ; \
	done ; 

else

installshipexec: ;

installshipexecwin32: ;

endif

ifdef SHIPCREATEDIRS

installshipmkdir:
	@for D in  $(SHIPCREATEDIRS) ; \
	do echo " Creating ${INSTVARBASE}/$$D Directory" ; \
	$(mkinstalldirs) ${INSTVARBASE}/$$D ; \
	done

installshipmkdirwin32:
	@for D in  $(SHIPCREATEDIRS) ; \
	do createdir="runtime/$$D" ; \
	${create_dir_win32_} ; \
	done ;

else

installshipmkdir: ;

installshipmkdirwin32: ;

endif

ifdef EXPORTS

export: $(EXPORTS)
	@D=`pwd` ; \
	createdir="${EXPORTBASE}/${EXPDIR}" ;\
	$(mkinstalldirs) $$createdir ; \
	for X in $? ; \
	do echo " Exporting $$X to $$createdir/$$X"; \
	ln -sf $$D/$$X $$createdir/$$X ; \
	done

exportwin32: $(EXPORTS)
	@D=`pwd` ; \
	createdir="${EXPORTBASE}/${EXPDIR}" ;\
	${create_dir_win32} ; \
	for X in $? ; \
	do echo "copy $$D/$$X $$createdir/$$X" ; \
	echo "if exist $$D/$$X copy $$D/$$X $$createdir/$$X" >> ${INIT_WIN32} ; \
	done ;

else

export: ;

exportwin32: ;

endif



ifdef PROGRAM

${PROGRAM}: ${OBJECTS} $(subst -l,${EXPORTBASE}/lib/lib, ${LIBS:=.a})
	${CXX} ${OBJECTS} ${LDFLAGS} ${LIBS} ${SOLIBS} ${EXT_LIBS} -o $@

installprogram: ${PROGRAM}
	@ createdir="${INSTLIBBASE}" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${PROGRAM}" ; \
	echo " $(INSTALL_PROGRAM) $$X $$createdir/$$X"; \
	$(INSTALL_PROGRAM) $$X $$createdir/$$X

else

installprogram: ;

endif

ifdef LOCALTOOLS

${LOCALTOOLS}: ${OBJECTS} $(subst -l,${EXPORTBASE}/lib/lib, ${LIBS:=.a})
	${CXX} ${OBJECTS} ${LDFLAGS} ${LIBS} ${EXT_LIBS} ${SOLIBS} -o $@

endif

ifdef TOOLS

${TOOLS}: ${OBJECTS} $(subst -l,${EXPORTBASE}/lib/lib, ${LIBS:=.a})
	${CXX} ${OBJECTS} ${LDFLAGS} ${LIBS} ${EXT_LIBS} ${SOLIBS} -o $@

installtools: ${TOOLS}
	@createdir="${INSTBINBASE}/${TOOLSDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${TOOLS}" ; \
	echo " $(INSTALL_PROGRAM) $$X $$createdir/$$X"; \
	$(INSTALL_PROGRAM) $$X $$createdir/$$X

else

ifdef TOOLSCRIPT

installtools: ${TOOLSCRIPT}
	@createdir="${INSTBINBASE}/${TOOLSDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${TOOLSCRIPT}" ; \
	echo " $(INSTALL_SCRIPT) $$X $$createdir/$$X"; \
	$(INSTALL_SCRIPT) $$X $$createdir/$$X

else

installtools: ;

endif
endif

ifdef TOOLSDATA

toolsdata: ;

installtoolsdata: $(TOOLSDATA)
	@createdir="${INSTDATABASE}/${TOOLSDATADIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	for X in $? ; \
	do echo " $(INSTALL_DATA) $$X $$createdir/$$X"; \
	$(INSTALL_DATA) $$X $$createdir/$$X ; \
	done


else 

toolsdata: ;
installtoolsdata: ;

endif


ifdef SOLIBRARY

${SOLIBRARY}: ${OBJECTS}
	${CXX} -shared -o ${SOLIBRARY} ${OBJECTS} ${LDFLAGS} ${LIBSPATH} ${LIBS} ${DEBUG_LIBS}
	@D=`pwd` ; \
	createdir="${EXPORTBASE}/lib" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${SOLIBRARY}" ; \
	echo " Exporting $$X to $$createdir/$$X"; \
	ln -sf $$D/$$X $$createdir/$$X


installsolibrary: ${SOLIBRARY}
	@createdir="${INSTLIBBASE}/lib" ; \
	X="${SOLIBRARY}" ;\
	$(mkinstalldirs) $$createdir ; \
	echo " $(INSTALL_DATA) $$X $$createdir/$$X"; \
	$(INSTALL_DATA) $$X $$createdir/$$X

installsolibrarywin32:
	@createdir="runtime/${SOLIBDIR}" ; \
	${create_dir_win32_}

else

installsolibrary: ;

installsolibrarywin32: ;

endif

ifdef MODULE

${MODULE}: ${OBJECTS}
	${CXX} -shared -o ${MODULE} ${OBJECTS} ${LDFLAGS} ${LIBSPATH} ${LIBS} 
	@D=`pwd` ; \
	createdir="${EXPORTBASE}/${MODULEDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	X="${MODULE}" ; \
	echo " Exporting $$X to $$createdir/$$X"; \
	ln -sf $$D/$$X $$createdir/$$X


installmodule: ${MODULE}
	@createdir="${INSTLIBBASE}/${MODULEDIR}" ; \
	X="${MODULE}" ;\
	$(mkinstalldirs) $$createdir ; \
	echo " $(INSTALL_DATA) $$X $$createdir/$$X"; \
	$(INSTALL_DATA) $$X $$createdir/$$X

installmodulewin32:
	@createdir="runtime/${MODULEDIR}" ; \
	${create_dir_win32_}


else

installmodule: ;

installmodulewin32: ;

endif

ifdef PKGLIST

packagelist:
	@for Pkg in ${PKGLIST} ;\
	do ${MAKE} onepackage PKG=$$Pkg  TORCS_BASE=${TORCS_BASE} MAKE_DEFAULT=${MAKE_DEFAULT};\
	createdir="${PACKAGESBASE}" ;\
	$(mkinstalldirs) $$createdir ; \
	archive="TORCS-${VERSION}-$$Pkg".tgz ;\
	echo "Creating Package $$archive" ;\
	tar -C ${PACKAGEBASE} -co . | gzip -9 > "${PACKAGESBASE}/$$archive" ;\
	rm -rf ${PACKAGEBASE} ;\
	done

else

packagelist: ;

endif

ifdef PKG

PKGFILES = $($(PKG)_PKGFILES)
PKGDIR = $($(PKG)_PKGDIR)

ifeq ($(strip $(PKGFILES)),)
PKGFILES = ""
endif

packagefiles:
	@if [ -n "${PKGFILES}" ];\
	then createdir="${PACKAGEBASE}/${PKGDIR}" ; \
	$(mkinstalldirs) $$createdir ; \
	for Pkg in ${PKGFILES} ; \
	do cp $$Pkg $$createdir/$$Pkg ; \
	echo " Package $$Pkg to $$createdir/$$Pkg"; \
	done ;\
	fi

else

packagefiles: ;

endif

ifdef SPECFILES

installspecfiles:
	@createdir="$(SPECFILESBASE)" ; \
	$(mkinstalldirs) $$createdir ; \
	for spec in $(SPECFILES) ; \
	do cp $$spec $$createdir/$$spec ; \
	echo " Specfile $$spec copied to $$createdir/$$spec" ; \
	done ;

else

installspecfiles: ;

endif

ifdef PKGSUBDIRS

packagedirs:
	@RecurseDirs="${PKGSUBDIRS}" ; \
	RecurseFlags="onepackage PKG=${PKG}" ; \
	${recursedirs}

else

packagedirs: ;

endif

ifdef DATASUBDIRS

instdatadirs:
	@RecurseDirs="${DATASUBDIRS}" ; \
	RecurseFlags="install" ; \
	${recursedirs}

else

instdatadirs: ;

endif

ifdef SHIPSUBDIRS

instshipdirs:
	@RecurseDirs="${SHIPSUBDIRS}" ; \
	RecurseFlags="install" ; \
	${recursedirs}

else

instshipdirs: ;

endif

ifdef SHIPEXECSUBDIRS

instshipexecdirs:
	@RecurseDirs="${SHIPEXECSUBDIRS}" ; \
	RecurseFlags="install" ; \
	${recursedirs}

else

instshipexecdirs: ;

endif


ifdef EXPINCDIRS

expincdirs: 
	@RecurseDirs="${EXPINCDIRS}" ; \
	RecurseFlags="exports" ; \
	${recursedirs}

expincwin32dirs: 
	@RecurseDirs="${EXPINCDIRS}" ; \
	RecurseFlags="exportswin32" ; \
	${recursedirs}

else

expincdirs: ;

expincwin32dirs: ;

endif



ifdef SUBDIRS

subdirs: 
	@RecurseDirs="${SUBDIRS}" ; \
	RecurseFlags="compil" ; \
	${recursedirs}

cleansubdirs:
	@RecurseDirs="${SUBDIRS}" ; \
	RecurseFlags="cleancompil" ; \
	${recursedirs}

instsubdirs:
	@RecurseDirs="${SUBDIRS}" ; \
	RecurseFlags="install" ; \
	${recursedirs}

specfilesdirs:
	@RecurseDirs="${SUBDIRS}" ; \
	RecurseFlags="specfiles" ; \
	${recursedirs}


else

subdirs: ;
cleansubdirs: ;
instsubdirs: ;
specfilesdirs: ;

endif



ifdef TOOLSUBDIRS

toolsdirs: 
	@RecurseDirs="${TOOLSUBDIRS}" ; \
	RecurseFlags="tools" ; \
	${recursedirs}

cleantoolsdirs:
	@RecurseDirs="${TOOLSUBDIRS}" ; \
	RecurseFlags="cleantools" ; \
	${recursedirs}

insttoolsdirs:
	@RecurseDirs="${TOOLSUBDIRS}" ; \
	RecurseFlags="install" ; \
	${recursedirs}

else

toolsdirs: ;
cleantoolsdirs: ;
insttoolsdirs: ;

endif

ifdef INITSUBDIRS

initsubdirs:
	@RecurseDirs="${INITSUBDIRS}" ; \
	RecurseFlags="init" ; \
	${recursedirs}

endif

#to be done in a better way...
ifdef SRCDOC

info:
	mkdir -p ${DOCBASE}/manual/api
	doxygen ${SOURCEBASE}/doc/torcsdoc.conf

endif

ifdef CONFIGCREATEDIRS

installconfmkdir:
	@for D in  $(CONFIGCREATEDIRS) ; \
	do echo "mkdir -p \$$1/$$D 2>/dev/null" >> ${SETUP_LINUX} ; \
	done

else

installconfmkdir: ;

endif

ifdef CONFIG

installconf:
	@for C in ${CONFIG} ; \
	do echo "if [ ! -e  \$$1/${SHIPDIR}/$$C ] || [ ${SHIPDIR}/$$C -nt \$$1/${SHIPDIR}/$$C ]" >> ${SETUP_LINUX} ; \
	echo "then" >> ${SETUP_LINUX} ; \
	echo "    if [ -e \$$1/${SHIPDIR}/$$C ]" >> ${SETUP_LINUX} ; \
	echo "    then" >> ${SETUP_LINUX} ; \
	echo "        echo \"Saving \$$1/${SHIPDIR}/$$C to \$$1/${SHIPDIR}/$$C.old\"" >> ${SETUP_LINUX} ; \
	echo "        cp -f \$$1/${SHIPDIR}/$$C \$$1/${SHIPDIR}/$$C.old" >> ${SETUP_LINUX} ; \
	echo "    fi" >> ${SETUP_LINUX} ; \
	echo "    cp -f ${SHIPDIR}/$$C \$$1/${SHIPDIR}/$$C" >> ${SETUP_LINUX} ; \
	echo "    chmod 640 \$$1/${SHIPDIR}/$$C" >> ${SETUP_LINUX} ; \
	echo "fi" >> ${SETUP_LINUX} ; \
	done

else

installconf: ;

endif



installdirs:
	@if [ -n "${SHIPSUBDIRS}" ] || [ -n "${SUBDIRS}" ] || [ -n "${TOOLSUBDIRS}" ] ; \
	then R=`for I in ${SHIPSUBDIRS} ${SUBDIRS} ${TOOLSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="install" ; \
	${recursedirs} ; \
	fi

installwin32dirs:
	@if [ -n "${SHIPSUBDIRS}" ] || [ -n "${SUBDIRS}" ] || [ -n "${TOOLSUBDIRS}" ] ; \
	then R=`for I in ${SHIPSUBDIRS} ${SUBDIRS} ${TOOLSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="installwin32" ; \
	${recursedirs} ; \
	fi

installdatadirs:
	@if [ -n "${DATASUBDIRS}" ] ; \
	then R=`for I in ${DATASUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="datainstall" ; \
	${recursedirs} ; \
	fi

installwin32datadirs:
	@if [ -n "${DATASUBDIRS}" ] ; \
	then R=`for I in ${DATASUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="win32datainstall" ; \
	${recursedirs} ; \
	fi

installshipdirs:
	@if [ -n "${SHIPSUBDIRS}" ] ; \
	then R=`for I in ${SHIPSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="installships" ; \
	${recursedirs} ; \
	fi

installshipexecdirs:
	@if [ -n "${SHIPEXECSUBDIRS}" ] ; \
	then R=`for I in ${SHIPEXECSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="installships" ; \
	${recursedirs} ; \
	fi

installshipwin32dirs:
	@if [ -n "${SHIPSUBDIRS}" ] ; \
	then R=`for I in ${SHIPSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="installshipswin32" ; \
	${recursedirs} ; \
	fi

installshipexecwin32dirs:
	@if [ -n "${SHIPEXECSUBDIRS}" ] ; \
	then R=`for I in ${SHIPEXECSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="installshipswin32" ; \
	${recursedirs} ; \
	fi


installconfdirs:
	@if [ -n "${SHIPSUBDIRS}" ] || [ -n "${SHIPEXECSUBDIRS}" ] || [ -n "${SUBDIRS}" ] || [ -n "${TOOLSUBDIRS}" ] ; \
	then R=`for I in ${SHIPSUBDIRS} ${SHIPEXECSUBDIRS} ${SUBDIRS} ${TOOLSUBDIRS} ; \
	do echo $$I ;\
	done | sort -u` ; \
	RecurseDirs="$$R" ; \
	RecurseFlags="userconfinstall" ; \
	${recursedirs} ; \
	fi
