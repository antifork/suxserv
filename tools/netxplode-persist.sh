#!/bin/sh

mintimeout=30
timeout=$mintimeout

test -n "$1" &&
{
    timeout=$1
}

test "$timeout" -lt $mintimeout &&
{
    echo "timeouts smaller than $mintimeout secs are NOT recommended !!";
    timeout=$mintimeout
}

( while true; do
	./netxplode.pl >/dev/null 2>&1 &
	sleep $timeout;
done ) &
