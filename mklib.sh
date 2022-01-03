#!/bin/bash


#
# This script relies on the following directory hierarchy existing:
# lib (dir)
#	MacOS (or Linux) (dir)
#		targets_m10.h (file) (edited to define "MACOS_m10" or "LINUX_m10")
#	medlib_m10.c (file)
#	medlib_m10.h (file)
#	medrec_m10.c (file)
#	medrec_m10.h (file)
#
# The library object code will be "/Volumes/devdrv/lib/MacOS/libmed_m10_mac.a" (or "/mnt/devdrv/lib/Linux/libmed_m10_lin.a").
# Link the library with "-L$LIBOBJ -lmed_m10$LIBSFX" in executables (see "compile.sh" for an example).
#


#########################
#### Start Edit Here ####
#########################

# compiler: "icc" or "clang"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m10.h file location: "local" or "library"
TGT_FILE="library"

########################
#### Stop Edit Here ####
########################


if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/devdrv"
	LIBSFX="_lin"
	PGINC="/usr/include/postgresql"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/devdrv"
	PGINC="/Applications/Postgres.app/Contents/Versions/latest/include"
	LIBSFX="_mac"
fi

LIBSRC=${DEVDRV}/lib
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

CC_OPT="-c -O2 -Wall"
if [ $CC = "icc" ]; then
	CC="/opt/intel/bin/icc"
	CC_OPT="${CC_OPT} -Qoption,cpp,--extended_float_types -static-intel"
	if [ $OS = "Linux" ]; then
		CC_OPT="${CC_OPT} -ipo"
	fi
elif [ $CC = "clang" ]; then
	CC="/usr/bin/clang"
fi

if [ $TGT_FILE = "library" ]; then
	TGTINC=$LIBOBJ
elif [ $TGT_FILE = "local" ]; then
	TGTINC=$PWD
fi


# delete old libraries
echo " "
CMD="rm *.a"
echo $CMD; echo " "
$CMD

# build libary
CMD="$CC $CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/medlib_m10.c"
echo $CMD; echo " "
$CMD

CMD="$CC $CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/medrec_m10.c"
echo $CMD; echo " "
$CMD

CMD="ar rcs libmed_m10${LIBSFX}.a medlib_m10.o medrec_m10.o"
echo $CMD; echo " "
$CMD

CMD="rm *.o"
echo $CMD; echo " "
$CMD

