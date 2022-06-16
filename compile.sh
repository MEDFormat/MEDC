#!/bin/bash


#
# This script relies on the following directory hierarchy and library existing:
# /Volumes/devdrv (or /mnt/devdrv) (mount point)
# 	lib (dir)
#		MacOS (or Linux) (dir)
#			targets_m11.h (file) (edited to define "MACOS_m11" or "LINUX_m11")
#			libmed_m11_mac.a (or libmed_m11_lin.a) (file)
#	MED2RAW (dir)
#		MacOS (or Linux) (dir)
#			MED2RAW.c (file)
#
# Link the library with "-L$LIBOBJ -lmed_m11$LIBSFX" in executables, as below.
#


#########################
#### Start Edit Here ####
#########################

# program name
PRG="MED2RAW"

# compiler: "icc" or "clang"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m11.h file location: "local" or "library"
TGT_FILE="library"

#### Set DEVDRV here ####

if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/dhndev"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/dhndev"
	LIBSFX="_mac"
fi

########################
#### Stop Edit Here ####
########################


PRGINC=${DEVDRV}/$PRG
PRGSRC=$PRGINC
PRGOBJ=${PRGSRC}/$OS
LIBINC=${DEVDRV}/lib
LIBOBJ=${LIBINC}/$OS

CC_OPT="-O2 -Wall -fms-extensions -Wno-microsoft-anon-tag"
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


# compile
CC_CMD="$CC -o ${PRGOBJ}/$PRG $CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m11$LIBSFX"
echo " "
echo $CC_CMD
$CC_CMD
echo " "

