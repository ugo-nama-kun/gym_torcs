#!/bin/bash

##############################################################################
#
#    file                 : robotgen
#    created              : Mon Jul 29 21:01:37 CEST 2002
#    copyright            : (C) 2002 by Eric Espié                        
#    email                : Eric.Espie@torcs.org   
#    version              : $Id: robotgen,v 1.4 2004/08/02 08:51:04 berniw Exp $                                  
#
##############################################################################
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
##############################################################################


function usage() {
    echo "usage: robotgen -n <name> -a <author> -c <car> [-d description] [--gpl]"
    exit 1
}

while [ $# -ne 0 ]
do
    case "$1" in
    "-n")
	NAME=$2
	shift;;

    "-a")
	AUTHOR=$2
	shift;;
    "-c")
	CAR=$2
	shift;;
    "-d")
	DESCRIPTION=$2
	shift;;
    "--gpl")
	GPL=yes
	;;
    esac
    shift
done

if [ -z "$NAME" ] || [ -z "$AUTHOR" ] || [ -z "$CAR" ]
then
    usage
fi

CATEGORY=$(grep category data/cars/models/$CAR/$CAR.xml | sed -e 's/^.*val=\"//' -e 's/\".*$//')

if [ -z "$CATEGORY" ]
then
    usage
fi

echo "Generation of robot $NAME author $AUTHOR"

ROBOTDIR=src/drivers/$NAME

rm -rf $ROBOTDIR

mkdir $ROBOTDIR || exit 1

echo -n "Copying $CAR.rgb ... "
cp data/cars/models/$CAR/$CAR.rgb $ROBOTDIR || exit 1
cp data/cars/models/$CAR/$CAR.xcf $ROBOTDIR 2>/dev/null
if [ $? == 0 ]
then
    echo -n "and $CAR.xcf ... "
fi
echo "done"

echo -n "Copying logo.rgb ... "
cp data/data/textures/logo.rgb $ROBOTDIR || exit 1
echo "done"

MAKEFILE=$ROBOTDIR/Makefile

echo -n "Generating $MAKEFILE ... "

echo '##############################################################################' > $MAKEFILE || exit 1
echo '#' >> $MAKEFILE || exit 1
echo '#    file                 : Makefile' >> $MAKEFILE || exit 1
echo '#    created              : '`date` >> $MAKEFILE || exit 1
echo '#    copyright            : (C) 2002 '$AUTHOR >> $MAKEFILE || exit 1
echo '#' >> $MAKEFILE || exit 1
echo '##############################################################################' >> $MAKEFILE || exit 1

if [ -n "$GPL" ]
then
    echo '#' >> $MAKEFILE || exit 1
    echo '#   This program is free software; you can redistribute it and/or modify' >> $MAKEFILE || exit 1
    echo '#   it under the terms of the GNU General Public License as published by' >> $MAKEFILE || exit 1
    echo '#   the Free Software Foundation; either version 2 of the License, or' >> $MAKEFILE || exit 1
    echo '#   (at your option) any later version.' >> $MAKEFILE || exit 1
    echo '#' >> $MAKEFILE || exit 1
    echo '##############################################################################' >> $MAKEFILE || exit 1
fi

echo >> $MAKEFILE || exit 1
echo "ROBOT       = $NAME" >> $MAKEFILE || exit 1
echo 'MODULE      = ${ROBOT}.so' >> $MAKEFILE || exit 1
echo 'MODULEDIR   = drivers/${ROBOT}' >> $MAKEFILE || exit 1
echo 'SOURCES     = ${ROBOT}.cpp' >> $MAKEFILE || exit 1
echo '' >> $MAKEFILE || exit 1
echo 'SHIPDIR     = drivers/${ROBOT}' >> $MAKEFILE || exit 1
echo 'SHIP        = ${ROBOT}.xml '$CAR'.rgb logo.rgb' >> $MAKEFILE || exit 1
echo 'SHIPSUBDIRS = ' >> $MAKEFILE || exit 1
echo '' >> $MAKEFILE || exit 1
echo 'PKGSUBDIRS  = ${SHIPSUBDIRS}' >> $MAKEFILE || exit 1
echo 'src-robots-'$NAME'_PKGFILES = $(shell find * -maxdepth 0 -type f -print)' >> $MAKEFILE || exit 1
echo 'src-robots-'$NAME'_PKGDIR   = ${PACKAGE}-${VERSION}/$(subst ${TORCS_BASE},,$(shell pwd))' >> $MAKEFILE || exit 1
echo '' >> $MAKEFILE || exit 1
echo 'include ${MAKE_DEFAULT}' >> $MAKEFILE || exit 1

echo "done"

XMLFILE=$ROBOTDIR/$NAME.xml

echo -n "Generating $XMLFILE ... "

echo '<?xml version="1.0" encoding="UTF-8"?>' > $XMLFILE || exit 1
echo '<!--' >> $XMLFILE || exit 1
echo '    file                 : '$NAME.xml >> $XMLFILE || exit 1
echo '    created              : '`date` >> $XMLFILE || exit 1
echo '    copyright            : (C) 2002 '$AUTHOR >> $XMLFILE || exit 1
echo '-->' >> $XMLFILE || exit 1
echo '' >> $XMLFILE || exit 1


if [ -n "$GPL" ]
then
    echo '<!--    This program is free software; you can redistribute it and/or modify  -->' >> $XMLFILE || exit 1
    echo '<!--    it under the terms of the GNU General Public License as published by  -->' >> $XMLFILE || exit 1
    echo '<!--    the Free Software Foundation; either version 2 of the License, or     -->' >> $XMLFILE || exit 1
    echo '<!--    (at your option) any later version.                                   -->' >> $XMLFILE || exit 1
    echo '' >> $XMLFILE || exit 1
fi

echo '<!DOCTYPE params SYSTEM "../../libs/tgf/params.dtd">' >> $XMLFILE || exit 1
echo '<params name="'$NAME'" type="robotdef">' >> $XMLFILE || exit 1
echo '  <section name="Robots">' >> $XMLFILE || exit 1
echo '    <section name="index">' >> $XMLFILE || exit 1
echo '      <section name="1">' >> $XMLFILE || exit 1
echo '	<attstr name="name" val="'$NAME'"></attstr>' >> $XMLFILE || exit 1
echo '	<attstr name="desc" val="'$DESCRIPTION'"></attstr>' >> $XMLFILE || exit 1
echo '	<attstr name="team" val=""></attstr>' >> $XMLFILE || exit 1
echo '	<attstr name="author" val="'$AUTHOR'"></attstr>' >> $XMLFILE || exit 1
echo '	<attstr name="car name" val="'$CAR'"></attstr>' >> $XMLFILE || exit 1
echo '	<attstr name="category" val="'$CATEGORY'"></attstr>' >> $XMLFILE || exit 1
echo '	<attnum name="race number" val="0"></attnum>' >> $XMLFILE || exit 1
echo '	<attnum name="red" val="1.0"></attnum>' >> $XMLFILE || exit 1
echo '	<attnum name="green" val="1.0"></attnum>' >> $XMLFILE || exit 1
echo '	<attnum name="blue" val="1.0"></attnum>' >> $XMLFILE || exit 1
echo '      </section>' >> $XMLFILE || exit 1
echo '    </section>' >> $XMLFILE || exit 1
echo '  </section>' >> $XMLFILE || exit 1
echo '</params>' >> $XMLFILE || exit 1

echo "done"

CPPFILE=$ROBOTDIR/$NAME.cpp

echo -n "Generating $CPPFILE ... "

echo '/***************************************************************************' > $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '    file                 : '$NAME.cpp >> $CPPFILE || exit 1
echo '    created              : '`date` >> $CPPFILE || exit 1
echo '    copyright            : (C) 2002 '$AUTHOR >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo ' ***************************************************************************/' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1

if [ -n "$GPL" ]
then
    echo '/***************************************************************************' >> $CPPFILE || exit 1
    echo ' *                                                                         *' >> $CPPFILE || exit 1
    echo ' *   This program is free software; you can redistribute it and/or modify  *' >> $CPPFILE || exit 1
    echo ' *   it under the terms of the GNU General Public License as published by  *' >> $CPPFILE || exit 1
    echo ' *   the Free Software Foundation; either version 2 of the License, or     *' >> $CPPFILE || exit 1
    echo ' *   (at your option) any later version.                                   *' >> $CPPFILE || exit 1
    echo ' *                                                                         *' >> $CPPFILE || exit 1
    echo ' ***************************************************************************/' >> $CPPFILE || exit 1
    echo '' >> $CPPFILE || exit 1
fi

echo '#ifdef _WIN32' >> $CPPFILE || exit 1
echo '#include <windows.h>' >> $CPPFILE || exit 1
echo '#endif' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '#include <stdio.h>' >> $CPPFILE || exit 1
echo '#include <stdlib.h> ' >> $CPPFILE || exit 1
echo '#include <string.h> ' >> $CPPFILE || exit 1
echo '#include <math.h>' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '#include <tgf.h> ' >> $CPPFILE || exit 1
echo '#include <track.h> ' >> $CPPFILE || exit 1
echo '#include <car.h> ' >> $CPPFILE || exit 1
echo '#include <raceman.h> ' >> $CPPFILE || exit 1
echo '#include <robottools.h>' >> $CPPFILE || exit 1
echo '#include <robot.h>' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo 'static tTrack	*curTrack;' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo 'static void initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s); ' >> $CPPFILE || exit 1
echo 'static void newrace(int index, tCarElt* car, tSituation *s); ' >> $CPPFILE || exit 1
echo 'static void drive(int index, tCarElt* car, tSituation *s); ' >> $CPPFILE || exit 1
echo 'static void endrace(int index, tCarElt *car, tSituation *s);' >> $CPPFILE || exit 1
echo 'static void shutdown(int index);' >> $CPPFILE || exit 1
echo 'static int  InitFuncPt(int index, void *pt); ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* ' >> $CPPFILE || exit 1
echo ' * Module entry point  ' >> $CPPFILE || exit 1
echo ' */ ' >> $CPPFILE || exit 1
echo 'extern "C" int ' >> $CPPFILE || exit 1
echo $NAME'(tModInfo *modInfo) ' >> $CPPFILE || exit 1
echo '{' >> $CPPFILE || exit 1
echo '    memset(modInfo, 0, 10*sizeof(tModInfo));' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '    modInfo->name    = "'$NAME'";		/* name of the module (short) */' >> $CPPFILE || exit 1
echo '    modInfo->desc    = "'$DESCRIPTION'";	/* description of the module (can be long) */' >> $CPPFILE || exit 1
echo '    modInfo->fctInit = InitFuncPt;		/* init function */' >> $CPPFILE || exit 1
echo '    modInfo->gfId    = ROB_IDENT;		/* supported framework version */' >> $CPPFILE || exit 1
echo '    modInfo->index   = 1;' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '    return 0; ' >> $CPPFILE || exit 1
echo '} ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* Module interface initialization. */' >> $CPPFILE || exit 1
echo 'static int ' >> $CPPFILE || exit 1
echo 'InitFuncPt(int index, void *pt) ' >> $CPPFILE || exit 1
echo '{ ' >> $CPPFILE || exit 1
echo '    tRobotItf *itf  = (tRobotItf *)pt; ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '    itf->rbNewTrack = initTrack; /* Give the robot the track view called */ ' >> $CPPFILE || exit 1
echo '				 /* for every track change or new race */ ' >> $CPPFILE || exit 1
echo '    itf->rbNewRace  = newrace; 	 /* Start a new race */' >> $CPPFILE || exit 1
echo '    itf->rbDrive    = drive;	 /* Drive during race */' >> $CPPFILE || exit 1
echo '    itf->rbPitCmd   = NULL;' >> $CPPFILE || exit 1
echo '    itf->rbEndRace  = endrace;	 /* End of the current race */' >> $CPPFILE || exit 1
echo '    itf->rbShutdown = shutdown;	 /* Called before the module is unloaded */' >> $CPPFILE || exit 1
echo '    itf->index      = index; 	 /* Index used if multiple interfaces */' >> $CPPFILE || exit 1
echo '    return 0; ' >> $CPPFILE || exit 1
echo '} ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* Called for every track change or new race. */' >> $CPPFILE || exit 1
echo 'static void  ' >> $CPPFILE || exit 1
echo 'initTrack(int index, tTrack* track, void *carHandle, void **carParmHandle, tSituation *s) ' >> $CPPFILE || exit 1
echo '{ ' >> $CPPFILE || exit 1
echo '    curTrack = track;' >> $CPPFILE || exit 1
echo '    *carParmHandle = NULL; ' >> $CPPFILE || exit 1
echo '} ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* Start a new race. */' >> $CPPFILE || exit 1
echo 'static void  ' >> $CPPFILE || exit 1
echo 'newrace(int index, tCarElt* car, tSituation *s) ' >> $CPPFILE || exit 1
echo '{ ' >> $CPPFILE || exit 1
echo '} ' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* Drive during race. */' >> $CPPFILE || exit 1
echo 'static void  ' >> $CPPFILE || exit 1
echo 'drive(int index, tCarElt* car, tSituation *s) ' >> $CPPFILE || exit 1
echo '{ ' >> $CPPFILE || exit 1
echo '    memset((void *)&car->ctrl, 0, sizeof(tCarCtrl)); ' >> $CPPFILE || exit 1
echo '    car->ctrl.brakeCmd = 1.0; /* all brakes on ... */ ' >> $CPPFILE || exit 1
echo '    /*  ' >> $CPPFILE || exit 1
echo '     * add the driving code here to modify the ' >> $CPPFILE || exit 1
echo '     * car->_steerCmd ' >> $CPPFILE || exit 1
echo '     * car->_accelCmd ' >> $CPPFILE || exit 1
echo '     * car->_brakeCmd ' >> $CPPFILE || exit 1
echo '     * car->_gearCmd ' >> $CPPFILE || exit 1
echo '     * car->_clutchCmd ' >> $CPPFILE || exit 1
echo '     */ ' >> $CPPFILE || exit 1
echo '}' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* End of the current race */' >> $CPPFILE || exit 1
echo 'static void' >> $CPPFILE || exit 1
echo 'endrace(int index, tCarElt *car, tSituation *s)' >> $CPPFILE || exit 1
echo '{' >> $CPPFILE || exit 1
echo '}' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1
echo '/* Called before the module is unloaded */' >> $CPPFILE || exit 1
echo 'static void' >> $CPPFILE || exit 1
echo 'shutdown(int index)' >> $CPPFILE || exit 1
echo '{' >> $CPPFILE || exit 1
echo '}' >> $CPPFILE || exit 1
echo '' >> $CPPFILE || exit 1

echo "done"

DEFFILE=$ROBOTDIR/$NAME.def

echo -n "Generating $DEFFILE ... "

echo ';' > $DEFFILE || exit 1
echo ';   file                 : '$NAME.def >> $DEFFILE || exit 1
echo ';   created              : '`date` >> $DEFFILE || exit 1
echo ';   copyright            : (C) 2002 '$AUTHOR >> $DEFFILE || exit 1
echo ';' >> $DEFFILE || exit 1

if [ -n "$GPL" ]
then
    echo ';' >> $DEFFILE || exit 1
    echo ';    This program is free software; you can redistribute it and/or modify' >> $DEFFILE || exit 1
    echo ';    it under the terms of the GNU General Public License as published by' >> $DEFFILE || exit 1
    echo ';    the Free Software Foundation; either version 2 of the License, or   ' >> $DEFFILE || exit 1
    echo ';    (at your option) any later version.                                 ' >> $DEFFILE || exit 1
    echo ';' >> $DEFFILE || exit 1
fi

echo '' >> $DEFFILE || exit 1
echo 'LIBRARY	'$NAME >> $DEFFILE || exit 1
echo '' >> $DEFFILE || exit 1
echo 'SECTIONS .data READ WRITE' >> $DEFFILE || exit 1
echo '' >> $DEFFILE || exit 1
echo 'EXPORTS' >> $DEFFILE || exit 1
echo '	'$NAME >> $DEFFILE || exit 1
echo '' >> $DEFFILE || exit 1

echo "done"

DSPFILE=$ROBOTDIR/$NAME.dsp
NAME_=$(echo $NAME | tr "abcdefghijklmnopqrstuvwxyz" "ABCDEFGHIJKLMNOPQRSTUVWXYZ")
echo -n "Generating $DSPFILE ... "

echo '# Microsoft Developer Studio Project File - Name="'$NAME'" - Package Owner=<4>
' > $DSPFILE || exit 1
echo '# Microsoft Developer Studio Generated Build File, Format Version 6.00
' >> $DSPFILE || exit 1
echo '# ** DO NOT EDIT **
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo 'CFG='$NAME' - Win32 Debug
' >> $DSPFILE || exit 1
echo '!MESSAGE This is not a valid makefile. To build this project using NMAKE,
' >> $DSPFILE || exit 1
echo '!MESSAGE use the Export Makefile command and run
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '!MESSAGE NMAKE /f "'$NAME'.mak".
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '!MESSAGE You can specify a configuration when running NMAKE
' >> $DSPFILE || exit 1
echo '!MESSAGE by defining the macro CFG on the command line. For example:
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '!MESSAGE NMAKE /f "'$NAME'.mak" CFG="'$NAME' - Win32 Debug"
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '!MESSAGE Possible choices for configuration are:
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '!MESSAGE "'$NAME' - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
' >> $DSPFILE || exit 1
echo '!MESSAGE "'$NAME' - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
' >> $DSPFILE || exit 1
echo '!MESSAGE 
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# Begin Project
' >> $DSPFILE || exit 1
echo '# PROP AllowPerConfigDependencies 0
' >> $DSPFILE || exit 1
echo '# PROP Scc_ProjName ""
' >> $DSPFILE || exit 1
echo '# PROP Scc_LocalPath ""
' >> $DSPFILE || exit 1
echo 'CPP=cl.exe
' >> $DSPFILE || exit 1
echo 'MTL=midl.exe
' >> $DSPFILE || exit 1
echo 'RSC=rc.exe
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '!IF  "$(CFG)" == "'$NAME' - Win32 Release"
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# PROP BASE Use_MFC 0
' >> $DSPFILE || exit 1
echo '# PROP BASE Use_Debug_Libraries 0
' >> $DSPFILE || exit 1
echo '# PROP BASE Output_Dir "Release"
' >> $DSPFILE || exit 1
echo '# PROP BASE Intermediate_Dir "Release"
' >> $DSPFILE || exit 1
echo '# PROP BASE Target_Dir ""
' >> $DSPFILE || exit 1
echo '# PROP Use_MFC 0
' >> $DSPFILE || exit 1
echo '# PROP Use_Debug_Libraries 0
' >> $DSPFILE || exit 1
echo '# PROP Output_Dir "Release"
' >> $DSPFILE || exit 1
echo '# PROP Intermediate_Dir "Release"
' >> $DSPFILE || exit 1
echo '# PROP Ignore_Export_Lib 1
' >> $DSPFILE || exit 1
echo '# PROP Target_Dir ""
' >> $DSPFILE || exit 1
echo '# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "'$NAME_'_EXPORTS" /YX /FD /c
' >> $DSPFILE || exit 1
echo '# ADD CPP /nologo /G5 /W3 /GX /O2 /I "../../../export/include" /I "../../windows/include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "'$NAME_'_EXPORTS" /YX /FD /c
' >> $DSPFILE || exit 1
echo '# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
' >> $DSPFILE || exit 1
echo '# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
' >> $DSPFILE || exit 1
echo '# ADD BASE RSC /l 0x40c /d "NDEBUG"
' >> $DSPFILE || exit 1
echo '# ADD RSC /l 0x40c /d "NDEBUG"
' >> $DSPFILE || exit 1
echo 'BSC32=bscmake.exe
' >> $DSPFILE || exit 1
echo '# ADD BASE BSC32 /nologo
' >> $DSPFILE || exit 1
echo '# ADD BSC32 /nologo
' >> $DSPFILE || exit 1
echo 'LINK32=link.exe
' >> $DSPFILE || exit 1
echo '# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
' >> $DSPFILE || exit 1
echo '# ADD LINK32 tgf.lib robottools.lib sg.lib ul.lib /nologo /dll /map /machine:I386 /nodefaultlib:"LIBCD" /libpath:"../../../export/lib" /libpath:"../../windows/lib"
' >> $DSPFILE || exit 1
echo '# Begin Special Build Tool
' >> $DSPFILE || exit 1
echo 'WkspDir=.
' >> $DSPFILE || exit 1
echo 'TargetDir=.\Release
' >> $DSPFILE || exit 1
echo 'SOURCE="$(InputPath)"
' >> $DSPFILE || exit 1
echo 'PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtime\drivers\'$NAME'
' >> $DSPFILE || exit 1
echo '# End Special Build Tool
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '!ELSEIF  "$(CFG)" == "'$NAME' - Win32 Debug"
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# PROP BASE Use_MFC 0
' >> $DSPFILE || exit 1
echo '# PROP BASE Use_Debug_Libraries 1
' >> $DSPFILE || exit 1
echo '# PROP BASE Output_Dir "Debug"
' >> $DSPFILE || exit 1
echo '# PROP BASE Intermediate_Dir "Debug"
' >> $DSPFILE || exit 1
echo '# PROP BASE Target_Dir ""
' >> $DSPFILE || exit 1
echo '# PROP Use_MFC 0
' >> $DSPFILE || exit 1
echo '# PROP Use_Debug_Libraries 1
' >> $DSPFILE || exit 1
echo '# PROP Output_Dir "Debug"
' >> $DSPFILE || exit 1
echo '# PROP Intermediate_Dir "Debug"
' >> $DSPFILE || exit 1
echo '# PROP Ignore_Export_Lib 1
' >> $DSPFILE || exit 1
echo '# PROP Target_Dir ""
' >> $DSPFILE || exit 1
echo '# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "'$NAME_'_EXPORTS" /YX /FD /GZ /c
' >> $DSPFILE || exit 1
echo '# ADD CPP /nologo /G5 /W3 /Gm /GX /ZI /Od /I "../../../export/include" /I "../../windows/include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "'$NAME_'_EXPORTS" /YX /FD /GZ /c
' >> $DSPFILE || exit 1
echo '# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
' >> $DSPFILE || exit 1
echo '# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
' >> $DSPFILE || exit 1
echo '# ADD BASE RSC /l 0x40c /d "_DEBUG"
' >> $DSPFILE || exit 1
echo '# ADD RSC /l 0x40c /d "_DEBUG"
' >> $DSPFILE || exit 1
echo 'BSC32=bscmake.exe
' >> $DSPFILE || exit 1
echo '# ADD BASE BSC32 /nologo
' >> $DSPFILE || exit 1
echo '# ADD BSC32 /nologo
' >> $DSPFILE || exit 1
echo 'LINK32=link.exe
' >> $DSPFILE || exit 1
echo '# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
' >> $DSPFILE || exit 1
echo '# ADD LINK32 robottools.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib sg.lib ul.lib /nologo /dll /map /debug /machine:I386 /pdbtype:sept /libpath:"../../../export/libd" /libpath:"../../windows/lib"
' >> $DSPFILE || exit 1
echo '# Begin Special Build Tool
' >> $DSPFILE || exit 1
echo 'WkspDir=.
' >> $DSPFILE || exit 1
echo 'TargetDir=.\Debug
' >> $DSPFILE || exit 1
echo 'SOURCE="$(InputPath)"
' >> $DSPFILE || exit 1
echo 'PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtimed\drivers\'$NAME'
' >> $DSPFILE || exit 1
echo '# End Special Build Tool
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '!ENDIF 
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# Begin Target
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# Name "'$NAME' - Win32 Release"
' >> $DSPFILE || exit 1
echo '# Name "'$NAME' - Win32 Debug"
' >> $DSPFILE || exit 1
echo '# Begin Group "Source Files"
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
' >> $DSPFILE || exit 1
echo '# Begin Source File
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo 'SOURCE=.\'$NAME'.cpp
' >> $DSPFILE || exit 1
echo '# End Source File
' >> $DSPFILE || exit 1
echo '# Begin Source File
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo 'SOURCE=.\'$NAME'.def
' >> $DSPFILE || exit 1
echo '# End Source File
' >> $DSPFILE || exit 1
echo '# End Group
' >> $DSPFILE || exit 1
echo '# Begin Group "Header Files"
' >> $DSPFILE || exit 1
echo '
' >> $DSPFILE || exit 1
echo '# PROP Default_Filter "h;hpp;hxx;hm;inl"
' >> $DSPFILE || exit 1
echo '# End Group
' >> $DSPFILE || exit 1
echo '# End Target
' >> $DSPFILE || exit 1
echo '# End Project
' >> $DSPFILE || exit 1

echo "done"
