# Microsoft Developer Studio Project File - Name="client" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=client - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "client.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "client.mak" CFG="client - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "client - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "client - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "client - Win32 Release"

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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIENT_EXPORTS" /YX /FD /c
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "../../../export/include" /I "../../interfaces" /I "../../windows/include" /I "../../.." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIENT_EXPORTS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 txml.lib libpng.lib sg.lib ul.lib tgf.lib robottools.lib ssggraphlib.lib ssg.lib ssgAux.lib sl.lib OpenAL32.lib Alut.lib /nologo /dll /machine:I386 /nodefaultlib:"LIBCD" /def:".\client.def" /libpath:"../../../export/lib" /libpath:"../../windows/lib"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtime	copy $(TargetDir)\*.lib $(WkspDir)\export\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

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
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIENT_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /G6 /W2 /GX /ZI /Od /I "../../../export/include" /I "../../windows/include" /I "../../interfaces" /I "../../.." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "CLIENT_EXPORTS" /D "DEBUG" /D "DEBUG_OUT" /FR /FD /GZ /c
# SUBTRACT CPP /WX /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 txml.lib libpng.lib sg.lib ul.lib tgf.lib robottools.lib ssggraphlib.lib ssg.lib ssgAux.lib sl.lib OpenAL32.lib Alut.lib /nologo /dll /map /debug /machine:I386 /nodefaultlib:"LIBC" /pdbtype:sept /libpath:"../../../export/libd" /libpath:"../../windows/lib"
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.dll $(WkspDir)\runtimed	copy $(TargetDir)\*.lib $(WkspDir)\export\libd
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "client - Win32 Release"
# Name "client - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\client.def

!IF  "$(CFG)" == "client - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "client - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\tgfclient\control.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\controlconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\driverconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\driverselect.cpp
# End Source File
# Begin Source File

SOURCE=.\entry.cpp
# End Source File
# Begin Source File

SOURCE=.\exitmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\fileselect.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\glfeatures.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\graphconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\gui.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guibutton.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guiedit.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guifont.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guihelp.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guiimage.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guilabel.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guimenu.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guiobject.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guiscrollbar.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\guiscrollist.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\img.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\joystickconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\loadingscreen.cpp
# End Source File
# Begin Source File

SOURCE=.\mainmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\miscscreens.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\mouseconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\openglconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\optionmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\pitmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\raceengine.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\racegl.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\raceinit.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\racemain.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\racemanmenu.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\racemenu.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\raceresults.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\racestate.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\results.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\screen.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\simuconfig.cpp
# End Source File
# Begin Source File

SOURCE=..\raceengineclient\singleplayer.cpp
# End Source File
# Begin Source File

SOURCE=..\confscreens\soundconfig.cpp
# End Source File
# Begin Source File

SOURCE=.\splash.cpp
# End Source File
# Begin Source File

SOURCE=..\tgfclient\tgfclient.cpp
# End Source File
# Begin Source File

SOURCE=..\racescreens\trackselect.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\driverconfig.h
# End Source File
# Begin Source File

SOURCE=.\exitmenu.h
# End Source File
# Begin Source File

SOURCE=.\mainmenu.h
# End Source File
# Begin Source File

SOURCE=.\optionmenu.h
# End Source File
# Begin Source File

SOURCE=.\splash.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
