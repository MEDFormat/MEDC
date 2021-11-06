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

# targets_m10.h file: "local" or "library"
TGT_FILE="library"

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

PRGINC=${DHNDEV}/$PRG
PRGSRC=$PRGINC
PRGOBJ=${PRGSRC}/$OS
LIBINC=${DHNDEV}/lib
LIBOBJ=${LIBINC}/$OS

CC_OPT="-O3 -Wall"
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
CC_CMD="$CC -o ${PRGOBJ}/$PRG -Wall $CC_OPT -I$TGTINC -I$PRGINC -I$LIBINC ${PRGSRC}/${PRG}.c -L$LIBOBJ -lmed_m10$LIBSFX"
echo " "
echo $CC_CMD
$CC_CMD
echo " "

