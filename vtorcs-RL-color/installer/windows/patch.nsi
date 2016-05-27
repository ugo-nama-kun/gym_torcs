;
;        file       : update.nsi
;        created    : Wed Feb 2 15:35:10 CET 2005
;        copyright  : (C) 2005 Bernhard Wymann
;        email      : berniw@bluewin.ch
;        version    : $Id: patch.nsi,v 1.2 2005/08/23 18:22:11 berniw Exp $
;
;        This program is free software; you can redistribute it and/or modify
;        it under the terms of the GNU General Public License as published by
;        the Free Software Foundation; either version 2 of the License, or
;        (at your option) any later version.

!define VER_MAJOR 1
!define VER_MINOR 2
!define VER_REVISION 4
!define VER_EXTRA
!define VER_RELDATE 20050823
!define PRODUCT_DIR_REGKEY "Software\Microsoft\Windows\CurrentVersion\App Paths\wtorcs.exe"

; The name of the installer
Name "TORCS 1.2.4-test3 to 1.2.4 Release Patch"

; The file to write
OutFile "patch_1_2_4_test2_to_release.exe"

; The default installation directory
; Get from registry
InstallDir "$PROGRAMFILES\torcs\"
InstallDirRegKey HKLM "${PRODUCT_DIR_REGKEY}" ""

; The text to prompt the user to enter a directory
DirText "Choose your TORCS 1.2.4-test3 folder"

; Show details
ShowInstDetails show

;--------------------------------

SetCompressor lzma

Section ""
  IfFileExists "$INSTDIR\wtorcs.exe" +3
    DetailPrint "Error: $INSTDIR\wtorcs.exe not found"
    Abort

  ; Set output path to the installation directory
  SetOutPath $INSTDIR

  ; Extract the patch to the plug-ins folder (temporary)
  InitPluginsDir
  File /oname=$TEMP\accc_exe.pat accc_exe.pat
  File /oname=$TEMP\client_dll.pat client_dll.pat
  File /oname=$TEMP\berniw_dll.pat berniw_dll.pat
  File /oname=$TEMP\berniw2_dll.pat berniw2_dll.pat
  File /oname=$TEMP\berniw3_dll.pat berniw3_dll.pat
  File /oname=$TEMP\bt_dll.pat bt_dll.pat
  File /oname=$TEMP\cylos1_dll.pat cylos1_dll.pat
  File /oname=$TEMP\damned_dll.pat damned_dll.pat
  File /oname=$TEMP\human_dll.pat human_dll.pat
  File /oname=$TEMP\inferno_dll.pat inferno_dll.pat
  File /oname=$TEMP\inferno2_dll.pat inferno2_dll.pat
  File /oname=$TEMP\lliaw_dll.pat lliaw_dll.pat
  File /oname=$TEMP\olethros_dll.pat olethros_dll.pat
  File /oname=$TEMP\tanhoj_dll.pat tanhoj_dll.pat
  File /oname=$TEMP\tita_dll.pat tita_dll.pat
  File /oname=$TEMP\learning_dll.pat learning_dll.pat
  File /oname=$TEMP\simuv2_dll.pat simuv2_dll.pat
  File /oname=$TEMP\track_dll.pat track_dll.pat
  File /oname=$TEMP\nfs2ac_exe.pat nfs2ac_exe.pat
  File /oname=$TEMP\nfsperf_exe.pat nfsperf_exe.pat
  File /oname=$TEMP\robottools_dll.pat robottools_dll.pat
  File /oname=$TEMP\tgf_dll.pat tgf_dll.pat
  File /oname=$TEMP\trackgen_exe.pat trackgen_exe.pat
  File /oname=$TEMP\txml_dll.pat txml_dll.pat
;  File /oname=$TEMP\uninst_exe.pat uninst_exe.pat
  File /oname=$TEMP\wtorcs_exe.pat wtorcs_exe.pat

  DetailPrint "Updating $INSTDIR\accc.exe..."
  vpatch::vpatchfile "$TEMP\accc_exe.pat" "$INSTDIR\accc.exe" "$INSTDIR\accc.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\accc.exe"
  Rename "$INSTDIR\accc.exe_" "$INSTDIR\accc.exe"
  Delete "$TEMP\accc_exe.pat"

  DetailPrint "Updating $INSTDIR\client.dll..."
  vpatch::vpatchfile "$TEMP\client_dll.pat" "$INSTDIR\client.dll" "$INSTDIR\client.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\client.dll"
  Rename "$INSTDIR\client.dll_" "$INSTDIR\client.dll"
  Delete "$TEMP\client_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\berniw\berniw.dll..."
  vpatch::vpatchfile "$TEMP\berniw_dll.pat" "$INSTDIR\drivers\berniw\berniw.dll" "$INSTDIR\drivers\berniw\berniw.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\berniw\berniw.dll"
  Rename "$INSTDIR\drivers\berniw\berniw.dll_" "$INSTDIR\drivers\berniw\berniw.dll"
  Delete "$TEMP\berniw_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\berniw2\berniw2.dll..."
  vpatch::vpatchfile "$TEMP\berniw2_dll.pat" "$INSTDIR\drivers\berniw2\berniw2.dll" "$INSTDIR\drivers\berniw2\berniw2.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\berniw2\berniw2.dll"
  Rename "$INSTDIR\drivers\berniw2\berniw2.dll_" "$INSTDIR\drivers\berniw2\berniw2.dll"
  Delete "$TEMP\berniw2_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\berniw3\berniw3.dll..."
  vpatch::vpatchfile "$TEMP\berniw3_dll.pat" "$INSTDIR\drivers\berniw3\berniw3.dll" "$INSTDIR\drivers\berniw3\berniw3.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\berniw3\berniw3.dll"
  Rename "$INSTDIR\drivers\berniw3\berniw3.dll_" "$INSTDIR\drivers\berniw3\berniw3.dll"
  Delete "$TEMP\berniw3_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\bt\bt.dll..."
  vpatch::vpatchfile "$TEMP\bt_dll.pat" "$INSTDIR\drivers\bt\bt.dll" "$INSTDIR\drivers\bt\bt.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\bt\bt.dll"
  Rename "$INSTDIR\drivers\bt\bt.dll_" "$INSTDIR\drivers\bt\bt.dll"
  Delete "$TEMP\bt_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\cylos1\cylos1.dll..."
  vpatch::vpatchfile "$TEMP\cylos1_dll.pat" "$INSTDIR\drivers\cylos1\cylos1.dll" "$INSTDIR\drivers\cylos1\cylos1.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\cylos1\cylos1.dll"
  Rename "$INSTDIR\drivers\cylos1\cylos1.dll_" "$INSTDIR\drivers\cylos1\cylos1.dll"
  Delete "$TEMP\cylos1_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\damned\damned.dll..."
  vpatch::vpatchfile "$TEMP\damned_dll.pat" "$INSTDIR\drivers\damned\damned.dll" "$INSTDIR\drivers\damned\damned.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\damned\damned.dll"
  Rename "$INSTDIR\drivers\damned\damned.dll_" "$INSTDIR\drivers\damned\damned.dll"
  Delete "$TEMP\damned_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\human\human.dll..."
  vpatch::vpatchfile "$TEMP\human_dll.pat" "$INSTDIR\drivers\human\human.dll" "$INSTDIR\drivers\human\human.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\human\human.dll"
  Rename "$INSTDIR\drivers\human\human.dll_" "$INSTDIR\drivers\human\human.dll"
  Delete "$TEMP\human_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\inferno\inferno.dll..."
  vpatch::vpatchfile "$TEMP\inferno_dll.pat" "$INSTDIR\drivers\inferno\inferno.dll" "$INSTDIR\drivers\inferno\inferno.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\inferno\inferno.dll"
  Rename "$INSTDIR\drivers\inferno\inferno.dll_" "$INSTDIR\drivers\inferno\inferno.dll"
  Delete "$TEMP\inferno_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\inferno2\inferno2.dll..."
  vpatch::vpatchfile "$TEMP\inferno2_dll.pat" "$INSTDIR\drivers\inferno2\inferno2.dll" "$INSTDIR\drivers\inferno2\inferno2.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\inferno2\inferno2.dll"
  Rename "$INSTDIR\drivers\inferno2\inferno2.dll_" "$INSTDIR\drivers\inferno2\inferno2.dll"
  Delete "$TEMP\inferno2_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\lliaw\lliaw.dll..."
  vpatch::vpatchfile "$TEMP\lliaw_dll.pat" "$INSTDIR\drivers\lliaw\lliaw.dll" "$INSTDIR\drivers\lliaw\lliaw.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\lliaw\lliaw.dll"
  Rename "$INSTDIR\drivers\lliaw\lliaw.dll_" "$INSTDIR\drivers\lliaw\lliaw.dll"
  Delete "$TEMP\lliaw_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\olethros\olethros.dll..."
  vpatch::vpatchfile "$TEMP\olethros_dll.pat" "$INSTDIR\drivers\olethros\olethros.dll" "$INSTDIR\drivers\olethros\olethros.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\olethros\olethros.dll"
  Rename "$INSTDIR\drivers\olethros\olethros.dll_" "$INSTDIR\drivers\olethros\olethros.dll"
  Delete "$TEMP\olethros_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\tanhoj\tanhoj.dll..."
  vpatch::vpatchfile "$TEMP\tanhoj_dll.pat" "$INSTDIR\drivers\tanhoj\tanhoj.dll" "$INSTDIR\drivers\tanhoj\tanhoj.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\tanhoj\tanhoj.dll"
  Rename "$INSTDIR\drivers\tanhoj\tanhoj.dll_" "$INSTDIR\drivers\tanhoj\tanhoj.dll"
  Delete "$TEMP\tanhoj_dll.pat"

  DetailPrint "Updating $INSTDIR\drivers\tita\tita.dll..."
  vpatch::vpatchfile "$TEMP\tita_dll.pat" "$INSTDIR\drivers\tita\tita.dll" "$INSTDIR\drivers\tita\tita.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\drivers\tita\tita.dll"
  Rename "$INSTDIR\drivers\tita\tita.dll_" "$INSTDIR\drivers\tita\tita.dll"
  Delete "$TEMP\tita_dll.pat"

  DetailPrint "Updating $INSTDIR\learning.dll..."
  vpatch::vpatchfile "$TEMP\learning_dll.pat" "$INSTDIR\learning.dll" "$INSTDIR\learning.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\learning.dll"
  Rename "$INSTDIR\learning.dll_" "$INSTDIR\learning.dll"
  Delete "$TEMP\learning_dll.pat"

  DetailPrint "Updating $INSTDIR\modules\simu\simuv2.dll..."
  vpatch::vpatchfile "$TEMP\simuv2_dll.pat" "$INSTDIR\modules\simu\simuv2.dll" "$INSTDIR\modules\simu\simuv2.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\modules\simu\simuv2.dll"
  Rename "$INSTDIR\modules\simu\simuv2.dll_" "$INSTDIR\modules\simu\simuv2.dll"
  Delete "$TEMP\simuv2_dll.pat"

  DetailPrint "Updating $INSTDIR\modules\track\track.dll..."
  vpatch::vpatchfile "$TEMP\track_dll.pat" "$INSTDIR\modules\track\track.dll" "$INSTDIR\modules\track\track.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\modules\track\track.dll"
  Rename "$INSTDIR\modules\track\track.dll_" "$INSTDIR\modules\track\track.dll"
  Delete "$TEMP\track_dll.pat"

  DetailPrint "Updating $INSTDIR\nfs2ac.exe..."
  vpatch::vpatchfile "$TEMP\nfs2ac_exe.pat" "$INSTDIR\nfs2ac.exe" "$INSTDIR\nfs2ac.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\nfs2ac.exe"
  Rename "$INSTDIR\nfs2ac.exe_" "$INSTDIR\nfs2ac.exe"
  Delete "$TEMP\nfs2ac_exe.pat"

  DetailPrint "Updating $INSTDIR\nfsperf.exe..."
  vpatch::vpatchfile "$TEMP\nfsperf_exe.pat" "$INSTDIR\nfsperf.exe" "$INSTDIR\nfsperf.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\nfsperf.exe"
  Rename "$INSTDIR\nfsperf.exe_" "$INSTDIR\nfsperf.exe"
  Delete "$TEMP\nfsperf_exe.pat"

  DetailPrint "Updating $INSTDIR\robottools.dll..."
  vpatch::vpatchfile "$TEMP\robottools_dll.pat" "$INSTDIR\robottools.dll" "$INSTDIR\robottools.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\robottools.dll"
  Rename "$INSTDIR\robottools.dll_" "$INSTDIR\robottools.dll"
  Delete "$TEMP\robottools_dll.pat"

  DetailPrint "Updating $INSTDIR\tgf.dll..."
  vpatch::vpatchfile "$TEMP\tgf_dll.pat" "$INSTDIR\tgf.dll" "$INSTDIR\tgf.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\tgf.dll"
  Rename "$INSTDIR\tgf.dll_" "$INSTDIR\tgf.dll"
  Delete "$TEMP\tgf_dll.pat"

  DetailPrint "Updating $INSTDIR\trackgen.exe..."
  vpatch::vpatchfile "$TEMP\trackgen_exe.pat" "$INSTDIR\trackgen.exe" "$INSTDIR\trackgen.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\trackgen.exe"
  Rename "$INSTDIR\trackgen.exe_" "$INSTDIR\trackgen.exe"
  Delete "$TEMP\trackgen_exe.pat"

  DetailPrint "Updating $INSTDIR\txml.dll..."
  vpatch::vpatchfile "$TEMP\txml_dll.pat" "$INSTDIR\txml.dll" "$INSTDIR\txml.dll_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\txml.dll"
  Rename "$INSTDIR\txml.dll_" "$INSTDIR\txml.dll"
  Delete "$TEMP\txml_dll.pat"
/*
  DetailPrint "Updating $INSTDIR\uninst.exe..."
  vpatch::vpatchfile "$TEMP\uninst_exe.pat" "$INSTDIR\uninst.exe" "$INSTDIR\uninst.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\uninst.exe"
  Rename "$INSTDIR\uninst.exe_" "$INSTDIR\uninst.exe"
  Delete "$TEMP\uninst_exe.pat"
*/
  DetailPrint "Updating $INSTDIR\wtorcs.exe..."
  vpatch::vpatchfile "$TEMP\wtorcs_exe.pat" "$INSTDIR\wtorcs.exe" "$INSTDIR\wtorcs.exe_"
  Pop $R0
  DetailPrint "Result: $R0"
  StrCmp $R0 "OK" 0 +3
  Delete "$INSTDIR\wtorcs.exe"
  Rename "$INSTDIR\wtorcs.exe_" "$INSTDIR\wtorcs.exe"
  Delete "$TEMP\wtorcs_exe.pat"

  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "VersionMajor" "${VER_MAJOR}"
  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "VersionMinor" "${VER_MINOR}"
  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "VersionRevision" "${VER_REVISION}"
  WriteRegDword HKLM "${PRODUCT_DIR_REGKEY}" "VersionDate" "${VER_RELDATE}"

  SetOutPath "$INSTDIR"
  File "uninst.exe"

SectionEnd