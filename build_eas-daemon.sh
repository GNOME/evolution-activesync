#!/bin/bash

# Before you try to  build the eas-daemon you need to Install the following packages : 
#   autoconf
#   libtool
#   libedataserver
#   libedataserver1.2-dev
#   libdbus-glib-1-dev
#   check
#   libexpat1-dev
#   libsoup2.4-dev
#	cmake
# 	libwbxml (must use 0.11.beta5 - obtain from http://sourceforge.net/projects/libwbxml/files/libwbxml/0.11.beta5/)

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
./eas-daemon/src/activesyncd &
make check
killall lt-activesyncd

