#!/bin/bash

if [ ! -d zopfli ]; then
    git clone git://github.com/google/zopfli.git
fi

if [ ! -d zlib-turbo ]; then
    git clone git://github.com/icodywu/zlib-turbo.git
fi

make -C zlib-turbo -f win32/Makefile.gcc
make -C zopfli

make OPT_CFLAGS=-I./zlib-turbo OPT_LDFLAGS="-L./zlib-turbo -static" E=.exe
