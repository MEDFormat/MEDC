
set DHNDRV=Z

set LIBSRC=%DHNDRV%:\lib
set LIBINC=%LIBSRC%
set LIBOBJ=%LIBSRC%\Windows
set TGTINC=%LIBOBJ%
set LIBSFX=_win

cl /c /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medlib_m10.c
cl /c /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\medrec_m10.c
lib /LTCG /nologo %LIBOBJ%\medlib_m10.obj %LIBOBJ%\medrec_m10.obj /OUT:%LIBOBJ%\libmed_m10%LIBSFX%.lib

cl /c /I%LIBINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D _UNICODE /D UNICODE %LIBSRC%\dhnlib_d10.c
lib /LTCG /nologo %LIBOBJ%\dhnlib_d10.obj /OUT:%LIBOBJ%\libdhn_d10%LIBSFX%.lib

del %LIBOBJ%\*.obj


