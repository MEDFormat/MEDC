
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

set DHNDRV=Z

set LIBSRC=%DHNDRV%:\lib\m13
set LIBINC=%LIBSRC%
set LIBOBJ=%LIBSRC%\Windows
set TGTINC=%LIBOBJ%
set LIBSFX=win

cl /c /std:c17 /experimental:c11atomics /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medlib_m13.c
cl /c /std:c17 /experimental:c11atomics /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medrec_m13.c
lib /LTCG /nologo %LIBOBJ%\medlib_m13.obj %LIBOBJ%\medrec_m13.obj /OUT:%LIBOBJ%\libmed_m13%_LIBSFX%.lib

cl /c /std:c17 /experimental:c11atomics /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\dhnlib_d13.c
lib /LTCG /nologo %LIBOBJ%\dhnlib_d13.obj /OUT:%LIBOBJ%\libdhn_d13%_LIBSFX%.lib

:: dhnlib with keys
move %TGTINC%\targets_m13.h %TGTINC%\targets_nokey_m13.h
move %TGTINC%\targets_key_m13.h %TGTINC%\targets_m13.h
del  %LIBOBJ%\dhnlib_d13.obj
cl /c /std:c17 /experimental:c11atomics /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\dhnlib_d13.c
lib /LTCG /nologo %LIBOBJ%\dhnlib_d13.obj /OUT:%LIBOBJ%\libdhnkey_d13%_LIBSFX%.lib
move %TGTINC%\targets_m13.h %TGTINC%\targets_key_m13.h
move %TGTINC%\targets_nokey_m13.h %TGTINC%\targets_m13.h


del %LIBOBJ%\*.obj


