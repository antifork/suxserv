#!/bin/sh

mintimeout=10
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

trap "killall netxplode.pl ; exit" 2 3 15

while true; do
	./netxplode.pl >/dev/null 2>&1 &
	sleep $timeout;
done
