# Microsoft Developer Studio Project File - Name="solid" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=solid - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "solid.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "solid.mak" CFG="solid - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "solid - Win32 Release" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE "solid - Win32 Debug" (basierend auf  "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "solid - Win32 Release"

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
# ADD CPP /nologo /G6 /W2 /GX /O2 /Ob2 /I "../../../../../export/include" /I "../../../../libs/torcs" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
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
WkspDir=.
TargetDir=.\Release
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.lib $(WkspDir)\export\lib
# End Special Build Tool

!ELSEIF  "$(CFG)" == "solid - Win32 Debug"

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
# ADD CPP /nologo /G6 /W2 /GX /ZI /Od /I "../../../../../export/include" /I "../../../../libs/torcs" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "DEBUG" /D "DEBUG_OUT" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo
# Begin Special Build Tool
WkspDir=.
TargetDir=.\Debug
SOURCE="$(InputPath)"
PostBuild_Cmds=copy $(TargetDir)\*.lib $(WkspDir)\export\libd
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "solid - Win32 Release"
# Name "solid - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\BBoxTree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Box.cpp
# End Source File
# Begin Source File

SOURCE=".\src\C-api.cpp"
# End Source File
# Begin Source File

SOURCE=.\src\Complex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Cone.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Convex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Cylinder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Endpoint.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Object.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Polygon.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Polyhedron.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Response.cpp
# End Source File
# Begin Source File

SOURCE=.\src\RespTable.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Simplex.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\src\Transform.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\AlgoTable.h
# End Source File
# Begin Source File

SOURCE=.\src\BBox.h
# End Source File
# Begin Source File

SOURCE=.\src\BBoxTree.h
# End Source File
# Begin Source File

SOURCE=.\src\Box.h
# End Source File
# Begin Source File

SOURCE=.\src\Complex.h
# End Source File
# Begin Source File

SOURCE=.\src\Cone.h
# End Source File
# Begin Source File

SOURCE=.\src\Convex.h
# End Source File
# Begin Source File

SOURCE=.\src\Cylinder.h
# End Source File
# Begin Source File

SOURCE=.\src\Encounter.h
# End Source File
# Begin Source File

SOURCE=.\src\Endpoint.h
# End Source File
# Begin Source File

SOURCE=.\src\IndexArray.h
# End Source File
# Begin Source File

SOURCE=.\src\Object.h
# End Source File
# Begin Source File

SOURCE=.\src\Polygon.h
# End Source File
# Begin Source File

SOURCE=.\src\Polyhedron.h
# End Source File
# Begin Source File

SOURCE=.\src\Polytope.h
# End Source File
# Begin Source File

SOURCE=.\src\Response.h
# End Source File
# Begin Source File

SOURCE=.\src\RespTable.h
# End Source File
# Begin Source File

SOURCE=.\src\Shape.h
# End Source File
# Begin Source File

SOURCE=.\src\Simplex.h
# End Source File
# Begin Source File

SOURCE=.\src\Sphere.h
# End Source File
# Begin Source File

SOURCE=.\src\Transform.h
# End Source File
# Begin Source File

SOURCE=.\src\VertexBase.h
# End Source File
# End Group
# End Target
# End Project
