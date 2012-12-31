#!/bin/sh

TESTS=tests.txt
RUN=mtarun

MYPWD=`pwd`

if [ $# -gt 0 ]; then
	RUN=$1
fi

ntests=0
nfailures=0

for line in `cat $TESTS`
do
	rulefile=`echo "$line" | cut -d ',' -f 1`
	datafile=`echo "$line" | cut -d ',' -f 2`
	checkfile=`echo "$line" | cut -d ',' -f 3`
	echo "[TEST] $rulefile $datafile"
	cd ..
	$RUN ./infer-rules-xmt __tests__/$rulefile __tests__/$datafile __tests__/closure-$datafile
	cd $MYPWD
	# sleep a few seconds just to make sure the fsworker has time to write the file
	sleep 3
	hexdump -v -e '8/1 "%02X" " " 8/1 "%02X" " " 8/1 "%02X" "\n"' closure-$datafile | sort -u > dump-closure-$datafile.txt
	delta=`diff $checkfile dump-closure-$datafile.txt`
	if [ "$delta" != "" ]; then
		echo "$delta"
		echo "FAILED $rulefile $datafile"
		nfailures=`expr $nfailures + 1`
	fi
	ntests=`expr $ntests + 1`
done

if [ $nfailures -gt 0 ]; then
	echo "FAILED $nfailures/$ntests tests.  FIX IT!"
else
	echo "PASSED ALL $ntests TESTS."
fi
exit $nfailures;
