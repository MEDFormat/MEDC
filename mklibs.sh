#!/bin/bash


#########################
#### Start Edit Here ####
#########################

# compiler: "clang", "gcc", or "icc"
CC="clang"

# OS: "MacOS" or "Linux"
OS="MacOS"

# targets_m11.h file: "local" or "library"
TGT_FILE="library"

# binary: "x86", "arm", or "all"
BIN="all"

# compile for deuggers: "true" or "false"
DBG="false"

########################
#### Stop Edit Here ####
########################

if [ $OS = "Linux" ]; then
	DHNDEV="/mnt/dhndev"
	LIBSFX="lin"
	PGINC="/usr/include/postgresql"
	PGLIB="/usr/lib/x86_64-linux-gnu"  # (no linking in this script)
elif [  $OS = "MacOS" ]; then
	DHNDEV="/Volumes/dhndev";
	LIBSFX="mac"
	PGINC="/Applications/Postgres.app/Contents/Versions/latest/include"
	PGLIB="/Applications/Postgres.app/Contents/Versions/latest/lib"  # (no linking in this script)
fi

LIBSRC=${DHNDEV}/lib/m13
LIBINC=$LIBSRC
LIBOBJ=${LIBINC}/$OS

if [ $TGT_FILE = "library" ]; then
	TGTINC=$LIBOBJ
elif [ $TGT_FILE = "local" ]; then
	TGTINC=$PWD
fi

if [ $DBG = "true" ]; then
	CC_OPT="-c -g -Wall -fms-extensions -Wno-microsoft-anon-tag"
else
	CC_OPT="-c -O3 -Wall -fms-extensions -Wno-microsoft-anon-tag"
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

# delete old libraries
echo " "
CMD="rm *_m13_${LIBSFX}.a"
echo $CMD; echo " "
$CMD

# build libaries
LIBNAME="medlib_m13"
if [ $BIN = "x86" ]; then
	if [  $OS = "MacOS" ]; then
		TMP_CC_OPT="${CC_OPT} -arch x86_64"
	else
		TMP_CC_OPT=$CC_OPT
	fi
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


LIBNAME="medrec_m13"
if [ $BIN = "x86" ]; then
	if [  $OS = "MacOS" ]; then
		TMP_CC_OPT="${CC_OPT} -arch x86_64"
	else
		TMP_CC_OPT=$CC_OPT
	fi
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

CMD="ar rcs libmed_m13_${LIBSFX}.a medlib_m13.o medrec_m13.o"
echo $CMD
$CMD
echo " "


# medlib with database functions
BASELIBNAME="medlib_m13"
LIBNAME="meddblib_m13"
mv ${TGTINC}/targets_m13.h ${TGTINC}/targets_nodb_m13.h
mv ${TGTINC}/targets_db_m13.h ${TGTINC}/targets_m13.h
if [ $BIN = "x86" ]; then
	if [  $OS = "MacOS" ]; then
		TMP_CC_OPT="${CC_OPT} -arch x86_64"
	else
		TMP_CC_OPT=$CC_OPT
	fi
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}.o -I$LIBINC -I$TGTINC -I$PGINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "arm" ]; then
	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12 -mmacosx-version-min=12.0"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}.o -I$LIBINC -I$TGTINC -I$PGINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD
elif [ $BIN = "all" ]; then
	TMP_CC_OPT="${CC_OPT} -arch x86_64"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_x86.o -I$LIBINC -I$TGTINC -I$PGINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	TMP_CC_OPT="${CC_OPT} -target arm64-apple-macos12"
	CMD="$CC $TMP_CC_OPT -o ${LIBNAME}_arm.o -I$LIBINC -I$TGTINC -I$PGINC ${LIBSRC}/${BASELIBNAME}.c"
	echo $CMD; echo " "
	$CMD

	CMD="lipo -create -output ${LIBNAME}.o ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD

	CMD="rm ${LIBNAME}_x86.o ${LIBNAME}_arm.o"
	echo $CMD; echo " "
	$CMD
fi
mv ${TGTINC}/targets_m13.h ${TGTINC}/targets_db_m13.h
mv ${TGTINC}/targets_nodb_m13.h ${TGTINC}/targets_m13.h

CMD="ar rcs libmeddb_m13_${LIBSFX}.a meddblib_m13.o medrec_m13.o"
echo $CMD
$CMD
echo " "


CMD="rm *.o"
echo $CMD; echo " "
$CMD

