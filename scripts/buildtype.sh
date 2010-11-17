#!/bin/bash

FILENAME=source/loader/alt_ios_gen.c
GENERATE=0
VERSION=249
PORT=0

if [ ! -z "$1" ];
then
	VERSION=$1
	
	if [ ! -z "$2" ];
	then
		PORT=$2
	fi
fi

if [ $PORT -ne 0 -a $PORT -ne 1 ];
then
	PORT=0
fi

if [ $PORT -eq 1 ];
then
	PORT="true"
else
	PORT="false"
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
if [ $GENERATE -eq 0 ];
then
	CURRENT_PORT=`grep port1\ = $FILENAME | awk '{printf "%s", substr($4, 1, length($4)-1)}'`
	if [ ${#CURRENT_PORT} -eq 0 -o "$CURRENT_PORT" != "$PORT" ];
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

bool use_port1 = $PORT;
EOF
fi