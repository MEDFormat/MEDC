
set MEDDRV=C
set PRG=MED2RAW
set OS=Windows

set PRGINC=%MEDDRV%:\%PRG%
set PRGSRC=%PRGINC%
set PRGOBJ=%PRGSRC%\%OS%
set LIBINC=%MEDDRV%:\lib
set LIBOBJ=%LIBINC%\%OS%
set LIBSFX=_win

set TGTINC=%PRGOBJ%

cl /I%LIBINC% /I%PRGINC% /I%TGTINC% /Gd /GL /EHsc /nologo /O2 /D NDEBUG /D _CONSOLE /D UNICODE /D _UNICODE %PRGSRC%\%PRG%.c /link %LIBOBJ%\libmed_m10%LIBSFX%.lib /OUT:%PRGOBJ%\%PRG%.exe

del *.obj

