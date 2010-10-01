#!/bin/bash

FILENAME=source/loader/alt_ios_gen.h
GENERATE=0
VERSION=249

if [ ! -z "$1" ];
then
	VERSION=$1
fi

if [ ! -f $FILENAME ];
then
	GENERATE=1
else
	CURRENT_VERSION=`grep mainIOS\ = $FILENAME | awk '{printf "%d", $4}'`
	if [ $CURRENT_VERSION -ne $VERSION ];
	then
		GENERATE=1
	fi
fi

if [ $GENERATE -eq 1 ];
then
	IOSVERSION="ODD"
	if [ $VERSION -eq 249 -o $VERSION -eq 250 -o $VERSION -eq 222 -o $VERSION -eq 223 -o $VERSION -eq 224 ];
	then
		IOSVERSION=$VERSION
	fi

	cat <<EOF > $FILENAME
#include "alt_ios.h"

int mainIOS = $VERSION;
int mainIOSminRev = IOS_${IOSVERSION}_MIN_REV;
EOF
fi
