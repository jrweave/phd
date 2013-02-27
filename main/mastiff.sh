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

if [ $# -lt 3 ] || [ $# -gt 4 ]; then
	echo "[USAGE] $0 <num-procs> <rule-file-base> <data-file-base> [reuse-data-from-job-number]"
	exit -1
fi

nproc=$1
rules=$2
data=$3
reuse=0
if [ $# -gt 3 ]; then
	reuse=$4
fi

maindir=`pwd`

rulesdir=$maindir/testfiles

PACKETSIZE=128
NUMREQUESTS=8
CHECKEVERY=4096
PAGESIZE=4096

echo "SETUP:"
echo "  Processors: $nproc"
echo "  Rules file: $rulesdir/$rules.core"
echo "  Data file: ~/data/$data.nt.lzo"
echo "  Index file: ~/data/$data.nt.idx"
echo "  Packet size: $PACKETSIZE"
echo "  Number of requests: $NUMREQUESTS"
echo "  Check every: $CHECKEVERY"
echo "  Page size: $PAGESIZE"
echo

perl scripts/core2prd.pl < $rulesdir/$rules.core > $rulesdir/$rules.prd

./encode-rules --print-constants $rulesdir/$rules-repls.enc < $rulesdir/$rules.prd > $rulesdir/$rules.enc 2> $rulesdir/$rules.dct

./der --print-index $rulesdir/$rules.dct | awk "{print \"--force \"\$2;}" > $rulesdir/$rules.frc

if [ $reuse -eq 0 ]; then
	LASTJOB=`date "+%Y%m%d%H%M%S"`
	LASTNP=$nproc
	~/makejobdirs.sh $LASTJOB $LASTNP
	echo "$LASTJOB"
	mpirun  -np  $LASTNP ./red-mpi -b 4194304 -p $PAGESIZE -si -if nt.lzo -i ~/data/$data.nt.lzo -ix ~/data/$data.nt.idx -of der -o ~/jobs/$LASTJOB/#/$data.der -od ~/jobs/$LASTJOB/#/$data.dct --global-dict --packet-size $PACKETSIZE --num-requests $NUMREQUESTS --check-every $CHECKEVERY `cat $rulesdir/$rules.frc`
	DATAJOB=$LASTJOB
else
	DATAJOB=$reuse
fi

LASTJOB=`date "+%Y%m%d%H%M%S"`
LASTNP=$nproc
echo "$LASTJOB"
~/makejobdirs.sh $LASTJOB $LASTNP
mpirun  -np  $LASTNP ./infer-rules-mpi $rulesdir/$rules.enc ~/jobs/$DATAJOB/#/$data.der ~/jobs/$LASTJOB/#/$data.der $rulesdir/$rules-repls.enc --page-size $PAGESIZE --packet-size $PACKETSIZE --num-requests $NUMREQUESTS --check-every $CHECKEVERY

date "+%Y%m%d%H%M%S"
