#!/bin/bash


#########################
#### Start Edit Here ####
#########################

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
	DHNDEV="/mnt/dhndev"
	PGINC="/usr/include/postgresql"
	LIBSFX="_lin"
elif [  $OS = "MacOS" ]; then
	DHNDEV="/Users/matt/DHN"
	PGINC="/Applications/Postgres.app/Contents/Versions/latest/include"
	LIBSFX="_mac"
fi

if [ $CC = "icc" ]; then
	BIN="x86"
fi

LIBSRC=${DHNDEV}/lib
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

CC_OPT="-c -O3 -Wall -fms-extensions -Wno-microsoft-anon-tag"
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

# build libaries
LIBNAME="medlib_m12"
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi

LIBNAME="medrec_m12"
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi

CMD="ar rcs libmed_m12${LIBSFX}.a medlib_m12.o medrec_m12.o"
echo $CMD
$CMD
echo " "


LIBNAME="dhnlib_d12"
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${LIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi

CMD="ar rcs libdhn_d12${LIBSFX}.a dhnlib_d12.o"
echo $CMD
$CMD
echo " "


# dhnlib with keys
BASELIBNAME="dhnlib_d12"
LIBNAME="dhnkeylib_d12"
mv ${TGTINC}/targets_m12.h ${TGTINC}/targets_nokey_m12.h
mv ${TGTINC}/targets_key_m12.h ${TGTINC}/targets_m12.h
if [ $BIN = "x86" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}.o -I$LIBINC -I$TGTINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}.o -I$LIBINC -I$TGTINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then	
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi
mv ${TGTINC}/targets_m12.h ${TGTINC}/targets_key_m12.h
mv ${TGTINC}/targets_nokey_m12.h ${TGTINC}/targets_m12.h

CMD="ar rcs libdhnkey_d12${LIBSFX}.a dhnkeylib_d12.o"
echo $CMD
$CMD
echo " "


CMD="rm *.o"
echo $CMD; echo " "
$CMD

