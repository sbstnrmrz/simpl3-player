#!/bin/bash

CC="cc"
CCFLAGS="-Wall -Wextra"
LIBDIR="/usr/local/lib"
INCLUDEDIR="/usr/local/include"
LFLAGS="-L$LIBDIR"
IFLAGS="-I$INCLUDEDIR"
LIBS="-lm -lSDL3 -lSDL3_image -lSDL3_ttf"
SRC="src/*.c"
RPATH="-rpath $LIBDIR"
EXE="simpl3player"
BUILDDIR="build/"

if [ `uname` = "Linux" ]; then
    LIBS+=" -ldl -pthread"
    BUILDDIR+="linux/"
elif [ `uname` = "Darwin" ]; then
    BUILDDIR+="mac/"
fi

rm -r $BUILDDIR
mkdir -p $BUILDDIR
cp -r assets $BUILDDIR

$CC $CCFLAGS $SRC -o $BUILDDIR$EXE $IFLAGS $LFLAGS $LIBS $RPATH
