#/bin/sh

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

if [ $# -lt 3 ]; then
	echo "Correct usage: $0 <num-procs> <rif-core-rules-file> <ntriples-file> [infer-rules-mpi flags]"
	exit
fi

NP=$1
shift
rules=$1
base1=`echo $rules | cut -d '.' -f 1`
shift
data=$1
base2=`echo $data | cut -d '.' -f 1`
shift

echo "perl scripts/core2prd.pl < $rules > $base1.prd"
perl scripts/core2prd.pl < $rules > $base1.prd

echo "./encode-rules --print-constants $base1-repls.enc < $base1.prd > $base1.enc 2> $base1.dct"
./encode-rules --print-constants $base1-repls.enc < $base1.prd > $base1.enc 2> $base1.dct

echo "./der --print-index $base1.dct | awk '{print \"--force \"$2;}' > $base1.frc"
./der --print-index $base1.dct | awk '{print "--force "$2;}' > $base1.frc

echo "rm -v $base2-rank-*.der $base2-rank-*.dct $base2-closure-rank-*.der $base2-closure-rank-*.nt"
rm -v $base2-rank-*.der $base2-rank-*.dct $base2-closure-rank-*.der $base2-closure-rank-*.nt

echo "mpirun -np $NP ./red-mpi -si -i $data -of der -o $base2-#.der -od $base2-#.dct `cat $base1.frc`"
mpirun -np $NP ./red-mpi -si -i $data -of der -o $base2-rank-#.der -od $base2-rank-#.dct --global-dict `cat $base1.frc`

echo "mpirun -np $NP ./infer-rules-mpi $base1.enc $base2-#.der $base2-closure-#.der $base1-repls.enc $@"
mpirun -np $NP ./infer-rules-mpi $base1.enc $base2-rank-#.der $base2-closure-rank-#.der $base1-repls.enc $@

echo "mpirun -np $NP ./red-mpi -if der -i $base2-closure-rank-#.der -id $base2-rank-#.der -o $base2-closure-rank-#.nt --global-dict"
mpirun -np $NP ./red-mpi -if der -i $base2-closure-rank-#.der -id $base2-rank-#.dct -o $base2-closure-rank-#.nt --global-dict
