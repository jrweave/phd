#!/bin/sh

# Copyright 2012 Jesse Weaver
# 
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
# 
#        http://www.apache.org/licenses/LICENSE-2.0
# 
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
#    implied. See the License for the specific language governing
#    permissions and limitations under the License.

if [ $# -ne 3 ]; then
	echo "[USAGE] $0 <cwm-script> <rif-core-file> <ntriples-files>"
	exit -1
fi

CWM="python $1"

cp $2 _rules
cp $3 _data
perl ground-bnodes.pl < _data > _grounded
perl core2n3.pl < _rules > _n3rules
$CWM --ntriples _grounded --think=_n3rules | perl fix-cwm-ntriples.pl > _temp0
grep -v '<tag:weavej3@rpi.edu,2012:/n3/' _temp0 | sort -u > _temp1
grep '<http://www.w3.org/2007/rif#error> <tag:weavej3@rpi.edu,2012:/n3/tuple> <http://www.w3.org/1999/02/22-rdf-syntax-ns#nil>' _temp0 > _inc1
cd ..; ./infer.sh testsuite/_rules testsuite/_grounded 2>&1 | grep INCONSISTENT > testsuite/_inc2 ; cd - 2>&1 > /dev/null
sort -u _grounded-closure.nt > _temp2
status=0
delta=`diff _temp1 _temp2`
if [ "$delta" != "" ]; then
	echo "$delta"
	echo "FAILED CWM $2 $3"
	status=-1
fi
inc1=`wc -l _inc1 | awk '{ print $1 }'`
inc2=`wc -l _inc2 | awk '{ print $1 }'`
if [ $inc1 -gt 0 ]; then
	if [ $inc2 -le 0 ]; then
		echo "CWM found inconsistency, but infer.sh did not."
		echo "FAILED CWM $2 $3"
		status=-1
	fi
fi
if [ $inc1 -le 0 ]; then
	if [ $inc2 -gt 0 ]; then
		echo "infer.sh found inconsistency, but CWM did not."
		echo "FAILED CWM $2 $3"
		status=-1
	fi
fi
if [ $status -eq 0 ]; then
	echo "PASSED CWM $2 $3"
fi
rm _*
exit $status
