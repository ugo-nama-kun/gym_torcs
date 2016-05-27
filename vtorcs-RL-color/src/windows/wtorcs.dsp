# Microsoft Developer Studio Project File - Name="wtorcs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=wtorcs - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "wtorcs.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "wtorcs.mak" CFG="wtorcs - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "wtorcs - Win32 Release" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE "wtorcs - Win32 Debug" (basierend auf  "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "wtorcs - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "..\..\export\include" /I "..\windows\include" /I "..\.." /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /FD /c
# SUBTRACT CPP /WX /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x409 /fo"Release/tocs.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 client.lib txml.lib libpng.lib tgf.lib /nologo /subsystem:console /pdb:none /map /machine:I386 /nodefaultlib:"LIBCD" /libpath:"../../export/lib" /libpath:"../windows/lib"
# SUBTRACT LINK32 /debug
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Desc=exports
PostBuild_Cmds=copy $(TargetDir)\wtorcs.exe $(WkspDir)\runtime	copy dll\glut32.dll $(WkspDir)\runtime	copy dll\OpenAL32.dll $(WkspDir)\runtime
# End Special Build Tool

!ELSEIF  "$(CFG)" == "wtorcs - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W2 /Gi /GX /ZI /Od /I "..\..\export\include" /I "..\windows\include" /I "..\.." /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "DEBUG" /D "DEBUG_OUT" /FR /FD /GZ /c
# SUBTRACT CPP /WX /YX
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 client.lib txml.lib libpng.lib tgf.lib /nologo /subsystem:console /map /debug /machine:I386 /nodefaultlib:"LIBC" /libpath:"../../export/libd" /libpath:"../windows/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Desc=exports
PostBuild_Cmds=copy $(TargetDir)\wtorcs.exe $(WkspDir)\runtimed	copy dll\glut32.dll $(WkspDir)\runtimed	copy dll\OpenAL32.dll $(WkspDir)\runtimed
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "wtorcs - Win32 Release"
# Name "wtorcs - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=..\..\torcs.rc
# End Source File
# Begin Source File

SOURCE=.\windowsspec.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\libs\torcs\driverconfig.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\exitmenu.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\mainmenu.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\optionmenu.h
# End Source File
# Begin Source File

SOURCE=.\osspec.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\raceman.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\splash.h
# End Source File
# Begin Source File

SOURCE=..\libs\torcs\torcs.h
# End Source File
# Begin Source File

SOURCE=.\windowsspec.h
# End Source File
# End Group
# Begin Source File

SOURCE=..\..\Ticon.ico
# End Source File
# End Target
# End Project
