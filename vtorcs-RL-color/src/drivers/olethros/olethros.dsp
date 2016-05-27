# Microsoft Developer Studio Project File - Name="olethros" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=olethros - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "olethros.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "olethros.mak" CFG="olethros - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "olethros - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "olethros - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "olethros - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OLETHROS_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "../../../export/include" /I "../../windows/include" /I "../../.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OLETHROS_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 learning.lib tgf.lib robottools.lib sg.lib ul.lib /nologo /dll /machine:I386 /libpath:"../../../export/lib" /libpath:"../../windows/lib"
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtime\drivers\olethros
# End Special Build Tool

!ELSEIF  "$(CFG)" == "olethros - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OLETHROS_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W2 /GX /ZI /Od /I "../../../export/include" /I "../../windows/include" /I "../../.." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "OLETHROS_EXPORTS" /D "DEBUG" /D "DEBUG_OUT" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 learning.lib tgf.lib robottools.lib sg.lib ul.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /libpath:"../../../export/libd" /libpath:"../../windows/lib"
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtimed\drivers\olethros
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "olethros - Win32 Release"
# Name "olethros - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cardata.cpp
# End Source File
# Begin Source File

SOURCE=.\driver.cpp
# End Source File
# Begin Source File

SOURCE=.\geometry.cpp
# End Source File
# Begin Source File

SOURCE=.\learn.cpp
# End Source File
# Begin Source File

SOURCE=.\olethros.cpp
# End Source File
# Begin Source File

SOURCE=.\olethros.def
# End Source File
# Begin Source File

SOURCE=.\opponent.cpp
# End Source File
# Begin Source File

SOURCE=.\pit.cpp
# End Source File
# Begin Source File

SOURCE=.\spline.cpp
# End Source File
# Begin Source File

SOURCE=.\strategy.cpp
# End Source File
# Begin Source File

SOURCE=.\TrackData.cpp
# End Source File
# Begin Source File

SOURCE=.\Trajectory.cpp
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cardata.h
# End Source File
# Begin Source File

SOURCE=.\driver.h
# End Source File
# Begin Source File

SOURCE=.\geometry.h
# End Source File
# Begin Source File

SOURCE=.\learn.h
# End Source File
# Begin Source File

SOURCE=.\linalg.h
# End Source File
# Begin Source File

SOURCE=.\opponent.h
# End Source File
# Begin Source File

SOURCE=.\pit.h
# End Source File
# Begin Source File

SOURCE=.\spline.h
# End Source File
# Begin Source File

SOURCE=.\strategy.h
# End Source File
# Begin Source File

SOURCE=.\TrackData.h
# End Source File
# Begin Source File

SOURCE=.\Trajectory.h
# End Source File
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
