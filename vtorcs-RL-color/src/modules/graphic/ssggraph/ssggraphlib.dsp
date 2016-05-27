# Microsoft Developer Studio Project File - Name="ssggraphlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ssggraphlib - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "ssggraphlib.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "ssggraphlib.mak" CFG="ssggraphlib - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "ssggraphlib - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "ssggraphlib - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ssggraphlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "../../../../export/include" /I "../../../windows/include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.lib ..\..\..\..\export\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "ssggraphlib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W2 /Gm- /GX /ZI /Od /I "./" /I "../../../../export/include" /I "../../../windows/include" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "DEBUG_OUT" /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.lib ..\..\..\..\export\libd
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "ssggraphlib - Win32 Release"
# Name "ssggraphlib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CarSoundData.cpp
# End Source File
# Begin Source File

SOURCE=.\grboard.cpp
# End Source File
# Begin Source File

SOURCE=.\grcam.cpp
# End Source File
# Begin Source File

SOURCE=.\grcar.cpp
# End Source File
# Begin Source File

SOURCE=.\grcarlight.cpp
# End Source File
# Begin Source File

SOURCE=.\grloadac.cpp
# End Source File
# Begin Source File

SOURCE=.\grmain.cpp
# End Source File
# Begin Source File

SOURCE=.\grmultitexstate.cpp
# End Source File
# Begin Source File

SOURCE=.\grscene.cpp
# End Source File
# Begin Source File

SOURCE=.\grscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\grshadow.cpp
# End Source File
# Begin Source File

SOURCE=.\grskidmarks.cpp
# End Source File
# Begin Source File

SOURCE=.\grsmoke.cpp
# End Source File
# Begin Source File

SOURCE=.\grsound.cpp
# End Source File
# Begin Source File

SOURCE=.\grtexture.cpp
# End Source File
# Begin Source File

SOURCE=.\grtrackmap.cpp
# End Source File
# Begin Source File

SOURCE=.\grutil.cpp
# End Source File
# Begin Source File

SOURCE=.\grvtxtable.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenalSoundInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\PlibSoundInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\SoundInterface.cpp
# End Source File
# Begin Source File

SOURCE=.\ssggraph.cpp
# End Source File
# Begin Source File

SOURCE=.\TorcsSound.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\grboard.h
# End Source File
# Begin Source File

SOURCE=.\grcam.h
# End Source File
# Begin Source File

SOURCE=.\grcar.h
# End Source File
# Begin Source File

SOURCE=.\grmain.h
# End Source File
# Begin Source File

SOURCE=.\grmultitexstate.h
# End Source File
# Begin Source File

SOURCE=.\grscene.h
# End Source File
# Begin Source File

SOURCE=.\grshadow.h
# End Source File
# Begin Source File

SOURCE=.\grskidmarks.h
# End Source File
# Begin Source File

SOURCE=.\grsmoke.h
# End Source File
# Begin Source File

SOURCE=.\grsound.h
# End Source File
# Begin Source File

SOURCE=.\grssgext.h
# End Source File
# Begin Source File

SOURCE=.\grtrackmap.h
# End Source File
# Begin Source File

SOURCE=.\grutil.h
# End Source File
# Begin Source File

SOURCE=.\grvtxtable.h
# End Source File
# End Group
# End Target
# End Project
