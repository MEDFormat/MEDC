#!/bin/bash


#########################
#### Start Edit Here ####
#########################

# program name
PRG="MED2RAW"

# compiler: "icc" or "clang"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m13.h file: "local" or "library"
TGT_FILE="library"

# binary: "x86", "arm", or "all"
BIN="all"

########################
#### Stop Edit Here ####
########################


if [ $OS = "Linux" ]; then
	DHNDEV="/mnt/dhndev"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DHNDEV="/Volumes/dhndev";
	LIBSFX="_mac"
fi

if [ $CC = "icc" ]; then
	BIN="x86"
fi

PRGINC=${DHNDEV}/$PRG
PRGSRC=$PRGINC
PRGOBJ=${PRGSRC}/$OS
LIBINC=${DHNDEV}/lib/m13
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
	if [  $OS = "MacOS" ]; then
		TMP_CC_OPT="${CC_OPT} -arch x86_64"
	else
		TMP_CC_OPT="${CC_OPT} -lm"
	fi
	CC_CMD="$CC -o ${PRGOBJ}/$PRG -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/$PRG -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_x86 -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13$LIBSFX"
	echo " "
	echo $CC_CMD
	$CC_CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CC_CMD="$CC -o ${PRGOBJ}/${PRG}_arm -Wall $TMP_CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m13$LIBSFX"
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


