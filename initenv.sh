#!/bin/bash
#source /home/relech/Linux/initenv.sh QZS3 /home/relech/mediasync/
#source /home/relech/Linux/initenv.sh LINUX /home/relech/mediasync/
#source /home/relech/Linux/initenv.sh DOCKER /home/relech/mediasync/
#echo $0  //bash
#echo $1 //qzs3
#echo $2 //ROOTDIR
ARC=$1
ROOTDIR_=/home/relech/mediasync/
if [ -n "$2" ]; then
    ROOTDIR_=$2
fi
export ROOTDIR=$ROOTDIR_
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$ROOTDIR/libs
export C_INCLUDE_PATH=$LD_LIBRARY_PATH:$ROOTDIR/include
export KERN_DIR=/home/relech/SDK_NAND/toolchain/linux-3.4
export VERSION_SAMBA=3.6.25
export PATH=$ROOTDIR/bin/openssl:$ROOTDIR/bin/ffmpeg:$PATH
#PATH去重复
export PATH=$(echo "$PATH" | awk -v RS=: '!seen[$0]++ {if (NR>1) printf ":"; printf $0}' )

ENV_QZS3_INIT()
{
    cd ~/SDK_NAND/toolchain/;source build/envsetup.sh;cd -
    CONFIGARGS=" --host=arm-linux-gnueabihf "
    APPFLAGS="-DARM"
    export CONFIGARGS
    export APPFLAGS
    export PROJECTARC="QZS3"
    export CC=arm-linux-gnueabihf-gcc
    export CXX=arm-linux-gnueabihf-g++
    export AR=arm-linux-gnueabihf-ar
    export LIBDLPATH=$TARGET_OUT/host/usr/arm-buildroot-linux-gnueabihf/sysroot/usr/lib
    export CMAKEARGS=" -DCMAKE_C_COMPILER=arm-linux-gnueabihf-gcc -DCMAKE_CXX_COMPILER=arm-linux-gnueabihf-g++ -DCMAKE_LIBRARY_PATH=$ROOTDIR/lib -DCMAKE_INCLUDE_PATH=$ROOTDIR/include/libexpat "
#   "json-c/ sqlite-amalgamation-3310100/ pcre/ libiconv/ jsoncpp/ exiv2/ ""
    export THIRDPARTLIB=""
    export APPS=""
    mkdir -p $ROOTDIR/
    echo "#ifndef _ARM_H__" > $ROOTDIR/arch.h
    echo "#define _ARM_H__" >> $ROOTDIR/arch.h
    echo "#define ARM" >> $ROOTDIR/arch.h
    echo "#define $PROJECTARC" >> $ROOTDIR/arch.h
    echo "#define ROOTDIR \"$ROOTDIR\"" >> $ROOTDIR/arch.h
    echo "#endif" >> $ROOTDIR/arch.h
}
ENV_DEFAULT_INIT()
{
    echo "ENV_DEFAULT_INIT"
    CONFIGARGS=" "
    export CONFIGARGS
    export PROJECTARC="DEFAULT"
    export CC=gcc
    export CXX=g++
    export AR=ar
    export LIBDLPATH="/usr/lib/x86_64-linux-gnu"
    export CMAKEARGS=" -DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++ -DCMAKE_LIBRARY_PATH=$ROOTDIR/lib -DCMAKE_INCLUDE_PATH=$ROOTDIR/include/libexpat "
#   "json-c/ sqlite-amalgamation-3310100/ pcre/ libiconv/ jsoncpp/ exiv2/ ""
    export THIRDPARTLIB=""
    export APPS=""
    mkdir -p $ROOTDIR/
    echo "#ifndef _ARM_H__" > $ROOTDIR/arch.h
    echo "#define _ARM_H__" >> $ROOTDIR/arch.h
    echo "#define ROOTDIR \"$ROOTDIR\"" >> $ROOTDIR/arch.h
    echo "#endif" >> $ROOTDIR/arch.h
}
ENV_DOCKER_INIT()
{
    ENV_DEFAULT_INIT
    export PROJECTARC="DOCKER"
}
case $ARC in
    QZS3)
    ENV_QZS3_INIT
    ;;
    DOCKER)
    ENV_DOCKER_INIT
    ;;
    *)
    ENV_DEFAULT_INIT
    ;;
esac
echo "------------------------"
echo "kernel="$KERN_DIR
echo "PROJECTARC="$PROJECTARC
echo "ROOTDIR="$ROOTDIR
echo "THIRDPARTLIB="$THIRDPARTLIB
echo "APPS="$APPS
echo "VERSION_SAMBA="$VERSION_SAMBA
echo $PATH
echo $LD_LIBRARY_PATH
