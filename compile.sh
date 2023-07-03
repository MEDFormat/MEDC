#!/bin/bash



#
# This script relies on the following directory hierarchy and library existing:
# /Volumes/devdrv (or /mnt/devdrv) (mount point)
# 	lib (dir)
#		MacOS (or Linux) (dir)
#			targets_m12.h (file) (edited to define "MACOS_m12" or "LINUX_m122")
#			libmed_m12_mac.a (or libmed_m11_lin.a) (file)
#	MED2RAW (dir)
#		MacOS (or Linux) (dir)
#			MED2RAW.c (file)
#
# Link the library with "-L$LIBOBJ -lmed_m12$LIBSFX" in executables, as below.
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

# targets_m11.h file: "local" or "library"
TGT_FILE="library"

# binary: "x86", "arm", or "all"
BIN="all"

########################
#### Stop Edit Here ####
########################


if [ $OS = "Linux" ]; then
	DEVDRV="/mnt/devdrv"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DEVDRV="/Volumes/devdrv";
	LIBSFX="_mac"
fi

if [ $CC = "icc" ]; then
	BIN="x86"
fi

PRGINC=${DEVDRV}/$PRG
PRGSRC=$PRGINC
PRGOBJ=${PRGSRC}/$OS
LIBINC=${DEVDRV}/lib
LIBOBJ=${LIBINC}/$OS

CC_OPT="-O3 -Wall -fms-extensions -Wno-microsoft-anon-tag"
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
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CC_CMD="$CC -o ${PRGOBJ}/$PRG -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m12$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/$PRG -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m12$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_x86 -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m12$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_arm -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m12$LIBSFX"
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


