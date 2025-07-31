#!/bin/bash


# This script relies on the following directory hierarchy and library existing:
# /Volumes/devdrv (or /mnt/devdrv) (mount point)
# 	lib (dir)
#		MacOS (or Linux) (dir)
#			targets_m13.h (file) (edited to define "MACOS_m13" or "LINUX_m13")
#			libmed_m13_mac.a (or libmed_m13_lin.a) (file)
#	MED2RAW (dir)
#		MacOS (or Linux) (dir)
#			MED2RAW.c (file)
#
# Link the library with "-L$LIBOBJ -lmed_m13_$LIBSFX" in executables (variables defined below)


#########################
#### Start Edit Here ####
#########################

# program name
PRG="MED2RAW"

# compiler: "clang", "gcc", or "icc"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m13.h file: "local" or "library"
TGT_FILE="library"

# binary: "x86", "arm", or "all"
BIN="all"

# compile for deuggers: "true" or "false"
DBG="false"

########################
#### Stop Edit Here ####
########################


if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/devdrv"
	LIBSFX="lin"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/devdrv";
	LIBSFX="mac"
fi

PRGINC=${DEVDRV}/$PRG
PRGSRC=$PRGINC
PRGOBJ=${PRGSRC}/$OS
LIBINC=${DEVDRV}/lib
LIBOBJ=${LIBINC}/$OS

if [ $TGT_FILE = "library" ]; then
	TGTINC=$LIBOBJ
elif [ $TGT_FILE = "local" ]; then
	TGTINC=$PRGOBJ
fi

if [ $DBG = "true" ]; then
	CC_OPT="-g -Wall -fms-extensions -Wno-microsoft-anon-tag"
else
	CC_OPT="-O3 -Wall -fms-extensions -Wno-microsoft-anon-tag"
fi

if [ $CC = "clang" ]; then
	CC="/usr/bin/clang"
elif [ $CC = "gcc" ]; then
	CC="/usr/bin/gcc"
elif [ $CC = "icc" ]; then
	BIN="x86"
	CC="/opt/intel/bin/icc"
	CC_OPT="${CC_OPT} -Qoption,cpp,--extended_float_types -static-intel"
	if [ $OS = "Linux" ]; then
		CC_OPT="${CC_OPT} -ipo"
	fi
fi


# compile
if [ $BIN = "x86" ]; then
	if [  $OS = "MacOS" ]; then
		TMP_CC_OPT="${CC_OPT} -arch x86_64"
	else
		TMP_CC_OPT="${CC_OPT} -lm"
	fi
	CC_CMD="$CC -o ${PRGOBJ}/$PRG $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13_$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/$PRG $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13_$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_x86 $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13_$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_arm $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13_$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD

	echo " "
	CMD="lipo -create -output ${PRGOBJ}/$PRG ${PRGOBJ}/${PRG}_x86 ${PRGOBJ}/${PRG}_arm"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${PRGOBJ}/${PRG}_x86 ${PRGOBJ}/${PRG}_arm"
	echo $CMD; echo " "
	$CMD
fi


