# Microsoft Developer Studio Project File - Name="RtspService" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=RtspService - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "RtspService.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "RtspService.mak" CFG="RtspService - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "RtspService - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "RtspService - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "RtspService - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "..\include" /I "..\network" /I "..\rtsp" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"..\Bin\RtspService.exe"

!ELSEIF  "$(CFG)" == "RtspService - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /Zi /Od /I "..\include" /I "..\network" /I "..\rtsp" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"..\Bin\RtspServiceD.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "RtspService - Win32 Release"
# Name "RtspService - Win32 Debug"
# Begin Group "Source"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\H264.h
# End Source File
# Begin Source File

SOURCE=.\h264_slice.h
# End Source File
# Begin Source File

SOURCE=.\h264_sps.h
# End Source File
# Begin Source File

SOURCE=.\RtspService.cpp
# End Source File
# Begin Source File

SOURCE=.\RtspService.h
# End Source File
# Begin Source File

SOURCE=.\RtspService_t.cpp
# End Source File
# Begin Source File

SOURCE=.\RtspService_t.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\vlc_bits.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Group "Include"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Include\cmd.h
# End Source File
# Begin Source File

SOURCE=..\Include\string_t.h
# End Source File
# Begin Source File

SOURCE=..\Include\TLock.h
# End Source File
# Begin Source File

SOURCE=..\Include\VBuffer.h
# End Source File
# Begin Source File

SOURCE=..\Include\VBufferT.h
# End Source File
# End Group
# Begin Group "Rtsp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Rtsp\BaseEncoder.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\Bitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\Bitstream.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaSession.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaSession.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStream.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStream.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamAMR.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamAMR.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamH263.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamH263.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamH264.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamH264.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4A.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4A.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4ALatm.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4ALatm.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4V.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamMP4V.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\MediaStreamTransport.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtpTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtpTransport.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\Rtsp.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\Rtsp.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspResponse.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspResponse.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspSession.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspSession.h
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspTransport.cpp
# End Source File
# Begin Source File

SOURCE=..\Rtsp\RtspTransport.h
# End Source File
# End Group
# Begin Group "Network"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\Network\Rtp.cpp
# End Source File
# Begin Source File

SOURCE=..\Network\Rtp.h
# End Source File
# Begin Source File

SOURCE=..\Network\Socket.cpp
# End Source File
# Begin Source File

SOURCE=..\Network\Socket.h
# End Source File
# Begin Source File

SOURCE=..\Network\Tcp.cpp
# End Source File
# Begin Source File

SOURCE=..\Network\Tcp.h
# End Source File
# Begin Source File

SOURCE=..\Network\Udp.cpp
# End Source File
# Begin Source File

SOURCE=..\Network\Udp.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
