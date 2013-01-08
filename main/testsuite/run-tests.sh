#!/bin/sh

TESTS=tests.txt
CWM='' #'/Users/jesseweaver/Desktop/cwm-1.2.1/cwm'

cwmfailures=0
cwmexpected=0
inferfailures=0
ntests=0
maindir=`cd ..; pwd; cd - 2>&1 > /dev/null`
maindir="$maindir/testfiles"

for line in `cat $TESTS`
do
	rulefile=`echo "$line" | cut -d ',' -f 1`
	datafile=`echo "$line" | cut -d ',' -f 2`
	mpiprocs=`echo "$line" | cut -d ',' -f 3`
	cwmnote=`echo "$line" | cut -d ',' -f 4`
	rulefile="$maindir/$rulefile"
	datafile="$maindir/$datafile"
	echo "[TEST] $rulefile $datafile"
	if [ "$CWM" != "" ] && [ "$cwmnote" != "cwmoff" ]; then
		./cwm-test.sh $CWM $rulefile $datafile
		rc=$?
		if [ "$cwmnote" != "cwmignore" ]; then
			cwmexpected=`expr $cwmexpected + 1`
			if [ $rc -ne 0 ]; then
				cwmfailures=`expr $cwmfailures + 1`
			fi
		else
			echo "(IGNORE CWM)"
		fi
	fi
	./infer-test.sh $rulefile $datafile $mpiprocs
	rc=$?
	if [ $rc -ne 0 ]; then
		inferfailures=`expr $inferfailures + 1`
	fi
	ntests=`expr $ntests + 1`
done

if [ $cwmfailures -gt 0 ]; then
	echo "FAILED $cwmfailures/$cwmexpected comparisons with CWM that were expected to succeed.  It might be okay.  CWM isn't perfect, but double-check the results anyway."
elif [ "$CWM" != "" ]; then
	echo "PASSED the expected $cwmexpected comparisons with CWM.  This is good if CWM is right, and if the expectations are correct."
fi

if [ $inferfailures -gt 0 ]; then
	echo "FAILED $inferfailures/$ntests tests showing an inconsistency in the infer scripts.  FIX IT!"
	exit -1
fi

echo "PASSED all $ntests tests between infer scripts, so everything seems consistent."
exit 0
