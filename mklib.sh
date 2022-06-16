#!/bin/bash


#
# This script relies on the following directory hierarchy existing:
# /Volumes/devdrv (or /mnt/devdrv) (mount point)
# 	lib (dir)
#		MacOS (or Linux) (dir)
#			targets_m11.h (file) (edited to define "MACOS_m11" or "LINUX_m11")
#		medlib_m11.c (file)
#		medlib_m11.h (file)
#		medrec_m11.c (file)
#		medrec_m11.h (file)
#
# The library object code will be "/Volumes/devdrv/lib/MacOS/libmed_m11_mac.a" (or "/mnt/devdrv/lib/Linux/libmed_m11_lin.a").
# Link the library with "-L$LIBOBJ -lmed_m11$LIBSFX" in executables (see "compile.sh" for an example).
#


#########################
#### Start Edit Here ####
#########################

# compiler: "icc" or "clang"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m11.h file location: "local" or "library"
TGT_FILE="library"

#### Set DEVDRV here ####

if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/devdrv"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/devdrv"
	LIBSFX="_mac"
fi

########################
#### Stop Edit Here ####
########################


LIBSRC=${DEVDRV}/lib
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

CC_OPT="-c -O2 -Wall -fms-extensions -Wno-microsoft-anon-tag"
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
CMD="$CC $CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/medlib_m11.c"
echo $CMD; echo " "
$CMD

CMD="$CC $CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/medrec_m11.c"
echo $CMD; echo " "
$CMD

CMD="ar rcs libmed_m11${LIBSFX}.a medlib_m11.o medrec_m11.o"
echo $CMD; echo " "
$CMD

CMD="rm *.o"
echo $CMD; echo " "
$CMD

