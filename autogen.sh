#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -f configure.in || {
    echo "**Error**: This directory does not look like the top-level directory"
    exit 1
}

#exit on failure 
set -e

echo running aclocal ...
aclocal

echo running autoheader ...
autoheader 

echo running libtoolize ...
libtoolize --automake -c

echo running automake ...
automake -a --copy 

echo running autoconf ...
autoconf

echo all done !
