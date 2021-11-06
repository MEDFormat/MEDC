#!/bin/bash


#########################
#### Start Edit Here ####
#########################

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
	MEDDEV="/mnt/dhndev"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	MEDDEV="/Volumes/dhndev"
	LIBSFX="_mac"
fi

LIBSRC=${MEDDEV}/lib
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

CC_OPT="-c -O3 -Wall"
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

