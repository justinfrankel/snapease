# Microsoft Developer Studio Project File - Name="snapease" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=snapease - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "snapease.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "snapease.mak" CFG="snapease - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "snapease - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "snapease - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "snapease - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "../WDL/zlib" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "USE_ICC" /D "PNG_WRITE_SUPPORTED" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /machine:I386 /opt:nowin98 /LARGEADDRESSAWARE
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "../WDL/zlib" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "PNG_WRITE_SUPPORTED" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib comctl32.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "snapease - Win32 Release"
# Name "snapease - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "WDL"

# PROP Default_Filter ""
# Begin Group "lice"

# PROP Default_Filter ""
# Begin Group "pnglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\libpng\png.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\png.h
# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngerror.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngget.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngmem.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngpread.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngread.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngrio.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngrtran.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngrutil.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngset.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngtrans.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngwio.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngwrite.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngwtran.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\WDL\libpng\pngwutil.c

!IF  "$(CFG)" == "snapease - Win32 Release"

# ADD CPP /D "PNG_USE_PNGVCRD" /D "PNG_LIBPNG_SPECIALBUILD" /D "__MMX__" /D "PNG_HAVE_MMX_COMBINE_ROW" /D "PNG_HAVE_MMX_READ_INTERLACE" /D "PNG_HAVE_MMX_READ_FILTER_ROW"

!ELSEIF  "$(CFG)" == "snapease - Win32 Debug"

!ENDIF 

# End Source File
# End Group
# Begin Group "zlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\zlib\adler32.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\compress.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\crc32.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\crc32.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\deflate.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\deflate.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\infback.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inffast.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inffast.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inffixed.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inflate.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inflate.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inftrees.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\inftrees.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\trees.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\trees.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\uncompr.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\zconf.in.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\zlib.h
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\zutil.c
# End Source File
# Begin Source File

SOURCE=..\WDL\zlib\zutil.h
# End Source File
# End Group
# Begin Group "jpeglib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\jpeglib\jcapimin.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcapistd.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jccoefct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jccolor.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcdctmgr.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jchuff.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jchuff.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcinit.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcmainct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcmarker.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcmaster.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcomapi.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jconfig.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcparam.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcphuff.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcprepct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jcsample.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jctrans.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdapimin.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdapistd.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdatadst.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdatasrc.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdcoefct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdcolor.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdct.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jddctmgr.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdhuff.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdhuff.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdinput.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdmainct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdmarker.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdmaster.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdmerge.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdphuff.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdpostct.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdsample.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jdtrans.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jerror.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jerror.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jfdctflt.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jfdctfst.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jfdctint.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jidctflt.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jidctfst.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jidctint.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jidctred.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jinclude.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jmemmgr.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jmemnobs.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jmemsys.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jmorecfg.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jpegint.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jpeglib.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jquant1.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jquant2.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jutils.c
# End Source File
# Begin Source File

SOURCE=..\WDL\jpeglib\jversion.h
# End Source File
# End Group
# Begin Group "giflib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\giflib\config.h
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\dgif_lib.c
# ADD CPP /I "../WDL/giflib" /D "HAVE_CONFIG_H"
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\egif_lib.c
# ADD CPP /I "../WDL/giflib" /D "HAVE_CONFIG_H"
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\gif_hash.c
# ADD CPP /I "../WDL/giflib" /D "HAVE_CONFIG_H"
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\gif_hash.h
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\gif_lib.h
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\gif_lib_private.h
# End Source File
# Begin Source File

SOURCE=..\WDL\giflib\gifalloc.c
# ADD CPP /I "../WDL/giflib" /D "HAVE_CONFIG_H"
# End Source File
# End Group
# Begin Source File

SOURCE=..\WDL\lice\lice.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice.h
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_arc.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_bezier.h
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_bmp.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_colorspace.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_combine.h
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_extended.h
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_gif.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_ico.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_image.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_jpg.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_jpg_write.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_line.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_pcx.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_png.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_png_write.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_text.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_text.h
# End Source File
# Begin Source File

SOURCE=..\WDL\lice\lice_textnew.cpp
# End Source File
# End Group
# Begin Group "vwnd"

# PROP Default_Filter ""
# Begin Source File

SOURCE="..\WDL\wingui\virtwnd-controls.h"
# End Source File
# Begin Source File

SOURCE="..\WDL\wingui\virtwnd-iconbutton.cpp"
# End Source File
# Begin Source File

SOURCE="..\WDL\wingui\virtwnd-listbox.cpp"
# End Source File
# Begin Source File

SOURCE="..\WDL\wingui\virtwnd-skin.h"
# End Source File
# Begin Source File

SOURCE="..\WDL\wingui\virtwnd-slider.cpp"
# End Source File
# Begin Source File

SOURCE=..\WDL\wingui\virtwnd.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\wingui\virtwnd.h
# End Source File
# Begin Source File

SOURCE=..\WDL\wingui\wndsize.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\wingui\wndsize.h
# End Source File
# End Group
# Begin Group "jnetlib"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\jnetlib\asyncdns.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\asyncdns.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\connection.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\connection.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\httpget.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\httpget.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\jnetlib.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\netinc.h
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\util.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\jnetlib\util.h
# End Source File
# End Group
# Begin Group "coolsb"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\WDL\wingui\scrollbar\coolscroll.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=..\WDL\projectcontext.cpp
# End Source File
# Begin Source File

SOURCE=..\WDL\projectcontext.h
# End Source File
# Begin Source File

SOURCE=..\WDL\win32_utf8.c
# End Source File
# Begin Source File

SOURCE=..\WDL\win32_utf8.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\config.cpp
# End Source File
# Begin Source File

SOURCE=.\decode_thread.cpp
# End Source File
# Begin Source File

SOURCE=.\export.cpp
# End Source File
# Begin Source File

SOURCE=.\imagerecord.cpp
# End Source File
# Begin Source File

SOURCE=.\label_edit.cpp
# End Source File
# Begin Source File

SOURCE=.\loadsave.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\main_wnd.cpp
# End Source File
# Begin Source File

SOURCE=.\sqlite3.c
# End Source File
# Begin Source File

SOURCE=.\sqlite3.h
# End Source File
# Begin Source File

SOURCE=.\upload_post.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\images\bw_on.png
# End Source File
# Begin Source File

SOURCE=.\images\color.png
# End Source File
# Begin Source File

SOURCE=.\images\crop_off.png
# End Source File
# Begin Source File

SOURCE=.\images\crop_on.png
# End Source File
# Begin Source File

SOURCE=.\images\full_off.png
# End Source File
# Begin Source File

SOURCE=.\images\full_on.png
# End Source File
# Begin Source File

SOURCE=.\images\remove.png
# End Source File
# Begin Source File

SOURCE=.\images\rot_left.png
# End Source File
# Begin Source File

SOURCE=.\images\rot_right.png
# End Source File
# Begin Source File

SOURCE=.\images\snapease.ico
# End Source File
# Begin Source File

SOURCE=.\snapease.rc
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\imagerecord.h
# End Source File
# Begin Source File

SOURCE=.\main.h
# End Source File
# Begin Source File

SOURCE=.\uploader.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\images\snapease.png
# End Source File
# End Target
# End Project
