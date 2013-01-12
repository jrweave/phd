#!/bin/sh

if [ $# -gt 1 ]; then
	echo "USAGE: $0 [optional-host-name]"
	exit 1
fi

if [ $# -gt 0 ]; then
	host=$1
else
	host=`hostname`
fi

case "$host" in
	q.ccni.rpi.edu)				script="eval-q.sh"
												;;
	mastiff.cs.rpi.edu)		script="eval-mastiff.sh"
												;;
	*)										echo "Unrecognized hostname: $host" 1>&2
												exit 2
												;;
esac

echo "$script"
echo "Do it? (y/n)"
read Y
if [ "$Y" != "y" ]; then
	echo "Aborted (you didn't enter 'y')." 1>&2
	exit 3
fi
./$script
