#!/usr/bin/env bash
MITIE_DIR=$(cd ../../; pwd)
DLIB_DIR="$MITIE_DIR/dlib"
MITIELIB_DIR="$MITIE_DIR/mitielib"
BINDING_DIR=`pwd`/MITIE
echo "MITIE_DIR = $MITIE_DIR"
echo "DLIB_DIR = $DLIB_DIR"
echo "MITIELIB_DIR = $MITIELIB_DIR"
echo "BINDING_DIR = $BINDING_DIR"

# copy dlib source
rm -rf $BINDING_DIR/src/dlib
cp -r $DLIB_DIR $BINDING_DIR/src
rm $BINDING_DIR/src/dlib/.[a-z]*

# copy mitielib source
rm -rf $BINDING_DIR/src/mitielib
mkdir $BINDING_DIR/src/mitielib
cp -r $MITIELIB_DIR/src $BINDING_DIR/src/mitielib
cp -r $MITIELIB_DIR/include $BINDING_DIR/src/mitielib
cp $MITIELIB_DIR/LICENSE.txt $BINDING_DIR/src/mitielib
