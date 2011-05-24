#!/bin/bash
export GLIB_GENMARSHAL=glib-genmarshal
clear
make clean
./autogen.sh
aclocal
autoconf
autoheader
touch NEWS README AUTHORS ChangeLog
automake --add-missing
./configure
make all
