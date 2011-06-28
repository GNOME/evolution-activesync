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
#   cmake
#   libwbxml (must use 0.11.beta5 - obtain from http://sourceforge.net/projects/libwbxml/files/libwbxml/0.11.beta5/)
#   libical


# the file accounts.cfg needs to be in your system configuration folder "/usr/local/etc"
# type the following command in your terminal:
#
# sudo cp ./eas-daemon/data/accounts.cfg /usr/local/etc 

export GLIB_GENMARSHAL=glib-genmarshal

clear
make clean

./autogen.sh
touch NEWS README AUTHORS ChangeLog
automake --add-missing
./configure --enable-gconf-schemas-install

make all
./gen-todo-list.sh
