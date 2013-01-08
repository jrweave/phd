#!/bin/sh

if [ $# -ne 3 ]; then
	echo "[USAGE] $0 <rif-core-file> <ntriples-files> <num-mpi-procs>"
	exit -1
fi

mpiprocs=$3

cp $1 _rules
cp $2 _data
cd ..; ./infer.sh testsuite/_rules testsuite/_data 2>&1 | tee testsuite/_out | grep INCONSISTENT > testsuite/_inc0; cd - 2>&1 > /dev/null
grep '\[ERROR\]' _out > _err
sort -u _data-closure.nt > _closure

## what a hassle
#mv _rules rules
#mv _data data
#rm _*
#mv closure _closure
#mv inc0 _inc0
#mv rules _rules
#mv data _data
## end hassle

cd ..; ./infer-mpi.sh $mpiprocs testsuite/_rules testsuite/_data 2>&1 | tee testsuite/_out_mpi | grep INCONSISTENT > testsuite/_inc1; cd - 2>&1 > /dev/null
grep '\[ERROR\]' _out_mpi > _err_mpi
sort -u _data-closure-rank-*.nt > _closure_mpi
rm _data-closure-rank-*.nt
cd ..; ./infer-xmt.sh testsuite/_rules testsuite/_data 2>&1 | tee testsuite/_out_xmt | grep INCONSISTENT > testsuite/_inc2; cd - 2>&1 > /dev/null
grep '\[ERROR\]' _out_xmt > _err_xmt
sort -u _data-closure.nt > _closure_xmt
rm _data-closure.nt
status=0
statusmpi=0
statusxmt=0
inc0=`wc -l _inc0 | awk '{ print $1 }'`
err0=`wc -l _err | awk '{ print $1 }'`
if [ $err0 -ne 0 ]; then
	cat _err
	echo "[ERROR] TOTAL OF $err0 ERRORS OCCURRED IN infer.sh."
	status=`expr $status + $err0`
fi
delta=`diff _closure _closure_mpi`
if [ "$delta" != "" ]; then
	echo "$delta"
	statusmpi=`expr $statusmpi + 1`
fi
inc1=`wc -l _inc1 | awk '{ print $1 }'`
err1=`wc -l _err_mpi | awk '{ print $1 }'`
if [ $inc0 -gt 0 ] && [ $inc1 -le 0 ]; then
	echo "infer.sh found consistency, but infer-mpi.sh did not."
	statusmpi=`expr $statusmpi + 1`
fi
if [ $inc0 -le 0 ] && [ $inc1 -gt 0 ]; then
	echo "infer-mpi.sh found inconsistency, but infer.sh did not."
	statusmpi=`expr $statusmpi + 1`
fi
if [ $err1 -ne 0 ]; then
	cat _err_mpi
	echo "[ERROR] TOTAL OF $err1 ERRORS OCCURRED IN infer-mpi.sh."
	statusmpi=`expr $statusmpi + 1`
fi
delta=`diff _closure _closure_xmt`
if [ "$delta" != "" ]; then
	echo "$delta"
	statusxmt=`expr $statusxmt + 1`
fi
inc2=`wc -l _inc2 | awk '{ print $1 }'`
err2=`wc -l _err_xmt | awk '{ print $1 }'`
if [ $inc0 -gt 0 ] && [ $inc2 -le 0 ]; then
	echo "infer.sh found consistency, but infer-xmt.sh did not."
	statusxmt=`expr $statusxmt + 1`
fi
if [ $inc0 -le 0 ] && [ $inc2 -gt 0 ]; then
	echo "infer-xmt.sh found inconsistency, but infer.sh did not."
	statusxmt=`expr $statusxmt + 1`
fi
if [ $err2 -ne 0 ]; then
	cat _err_xmt
	echo "[ERROR] TOTAL OF $err2 ERRORS OCCURRED IN infer-xmt.sh."
	statusxmt=`expr $statusxmt + 1`
fi
if [ $statusmpi -gt 0 ]; then
	echo "FAILED MPI $1 $2"
else
	echo "PASSED MPI $1 $2"
fi
if [ $statusxmt -gt 0 ]; then
	echo "FAILED XMT $1 $2"
else
	echo "PASSED XMT $1 $2"
fi
#rm _*
exit `expr $status + $statusmpi + $statusxmt`
