#!/bin/bash

export GLIB_GENMARSHAL=glib-genmarshal
clear
make clean
./autogen.sh
aclocal
autoconf
autoheader

# these files are empty files but required by the autoconf build tool
# these filea are in Mobica Projec SVN so no need to generate them
#touch NEWS README AUTHORS ChangeLog

automake --add-missing
./configure
make all
