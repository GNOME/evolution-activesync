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
# 	libwbxml (Not from Package Manager - use \\boudica\projects\INT07\Reference\Linux\libwbxml-0.11.beta4.tar.gz)


# the file accounts.cfg needs to be in your system configuration folder "/usr/local/etc"
# type the following command in your terminal:
#
# sudo cp ./eas-daemon/data/accounts.cfg /usr/local/etc 

export GLIB_GENMARSHAL=glib-genmarshal

clear
make clean


./autogen.sh
aclocal
autoconf
autoheader
touch NEWS README AUTHORS ChangeLog
automake --add-missing
#mkdir $HOME/eas-daemon-install
#./configure $HOME/eas-daemon-install
./configure

make all
#make install
./eas-daemon/src/activesyncd &
#./eas-daemon/src/.libs/activesyncd &
make check
killall lt-activesyncd

