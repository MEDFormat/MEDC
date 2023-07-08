 
::
:: This script relies on the following directory hierarchy existing:
:: Z: (drive)
::	lib (dir)
::		Windows (dir)
::			targets_m12.h (file) (edited to define "WINDOWS_m12")
::		medlib_m12.c (file)
::		medlib_m12.h (file)
::		medrec_m12.c (file)
::		medrec_m12.h (file)
::
:: The library object code will be "Z:\lib\Windows\libmed_m11_win.lib"
:: Link the library with "/link %LIBOBJ%\libmed_m12%LIBSFX%.lib" in executables (see "compile.bat" for an example).
::

::
:: For 64-bit native code When compiling with the Visual Studio compiler ("cl") from the command line:
:: 	1) Use "x64 Native Tools Command Prompt for VS 2019" as the terminal program. (This just sets a number of necessary environment variables.)
::	2) Set VSCMD_ARG_HOST_ARCH & VSCMD_ARG_TGT_ARCH to x64, as below.
::
:: For 32-bit executables remove the additions above. 
:: The library is designed for 64-bit processing, so this is less efficient, but it works.
::

set VSCMD_ARG_HOST_ARCH=x64
set VSCMD_ARG_TGT_ARCH=x64

set DEVDRV=Z

set LIBSRC=%DEVDRV%:\lib
set LIBINC=%LIBSRC%
set LIBOBJ=%LIBSRC%\Windows
set TGTINC=%LIBOBJ%
set LIBSFX=_win

cl /c /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medlib_m12.c
cl /c /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medrec_m12.c
lib /LTCG /nologo %LIBOBJ%\medlib_m12.obj %LIBOBJ%\medrec_m12.obj /OUT:%LIBOBJ%\libmed_m12%LIBSFX%.lib

del %LIBOBJ%\*.obj


