#!/bin/sh

test -z "$1" &&
{
    echo "usage: $0 <timeout>";
    exit -1;
}

test "$1" -lt 60 &&
{
    echo "timeouts smaller than 60 secs are NOT recommended !!";
    exit 1;
}

timeout=$1;

trap "killall netxplode.pl ; exit" 2 3 15

while true; do
	./netxplode.pl >/dev/null 2>&1 &
	sleep $timeout;
done
