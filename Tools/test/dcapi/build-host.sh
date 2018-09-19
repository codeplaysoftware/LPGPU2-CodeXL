#!/bin/bash
#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#

##########
#
# Check build environment
#
##########

# fail on command error
set -e

# fail if using unset variable
set -u

command_exists () {
    type "$1" &> /dev/null ;
}

check_command () {
    if command_exists "$1" ; then
        :
    else
        echo "Missing a required tool: $1";
        exit 1;
    fi
}

# environment variables

var="${BUILD_DIRECTORY:?Missing environment variable for out of tree build: BUILD_DIRECTORY}"

# commands
check_command g++;
check_command python3;

###########
#
# Shim
#
###########

# 0 or 1
SHIM_DEBUG_BUILD=1

##########
#
# libdcapi test
#
##########


if [ "$SHIM_DEBUG_BUILD" -eq "1" ]
then
    set -x
fi

if [ -z "$BUILD_DIRECTORY" ]
then
   BUILD_DIRECTORY=../build/libdcapitest
fi

mkdir -p $BUILD_DIRECTORY

cp ./jni/*.* $BUILD_DIRECTORY/
cp ../../dcapi/*.h $BUILD_DIRECTORY/
cp ../../api-shim/src/get_time.h $BUILD_DIRECTORY/

# build
cd $BUILD_DIRECTORY
g++ -std=c++11 -Wall -Wextra -fPIC -shared -g -I. -Wl,-soname=libDCAPITest.so DCAPI.cpp -o libDCAPITest.so
