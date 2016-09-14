#!/bin/sh

set -e

# wipe out temporary autotools files, necessary
# when switching between distros
rm -rf aclocal.m4 m4 autom4te.cache config.guess config.sub config.h.in configure depcomp install-sh ltmain.sh missing

# intltoolize fails to copy its macros unless m4 exits
mkdir m4

libtoolize -c
glib-gettextize --force --copy
intltoolize --force --copy --automake
aclocal -I m4 -I m4-repo
autoheader
automake -a -c -Wno-portability
autoconf

test -n "$NOCONFIGURE" || ./configure --enable-maintainer-mode "$@"
