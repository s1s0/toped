#!/bin/sh
aclocal -I macros
libtoolize --automake --force --copy
automake -a -c
autoconf

