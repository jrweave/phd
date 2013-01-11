#!/bin/sh

TESTS=tests.txt

DERDUMP="hexdump -v -e '8/1 \"%02X\" \" \" 8/1 \"%02X\" \" \" 8/1 \"%02X\" \"\\n\"'"

maindir=`cd ..; pwd; cd - 2>&1 > /dev/null`
xmtdir="$maindir/deploy-to-xmt"
maindir="$maindir/testfiles"

ntests=0

for line in `cat $TESTS`
do
	rfile=`echo "$line" | cut -d ',' -f 1`
	dfile=`echo "$line" | cut -d ',' -f 2`
	rbase=`echo $rfile | cut -d '.' -f 1`
	dbase=`echo $dfile | cut -d '.' -f 1`
	rulefile="$maindir/$rfile"
	datafile="$maindir/$dfile"
	rulebase=`echo $rulefile | cut -d '.' -f 1`
	database=`echo $datafile | cut -d '.' -f 1`
	cd ..; ./infer.sh $rulefile $datafile > /dev/null 2> /dev/null; hexdump -v -e '8/1 "%02X" " " 8/1 "%02X" " " 8/1 "%02X" "\n"' $database-closure.der | sort -u > $database-$rbase-verify-dump.txt; cp $rulebase.enc $xmtdir/__tests__/$rbase-$ntests.enc; cp $database.der $xmtdir/__tests__/$dbase-$ntests.der; cp $database-$rbase-verify-dump.txt $xmtdir/__tests__/$dbase-$rbase-$ntests-verify-dump.txt; cd - 2>&1 > /dev/null
	echo "$rbase-$ntests.enc,$dbase-$ntests.der,$dbase-$rbase-$ntests-verify-dump.txt" | tee -a $xmtdir/__tests__/tests.txt
	ntests=`expr $ntests + 1`
done
