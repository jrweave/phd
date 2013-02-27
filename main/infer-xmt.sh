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

if [ $# != 2 ]; then
	echo "Correct usage: infer.sh <rif-core-rules-file> <ntriples-file>"
	exit
fi

base1=`echo $1 | cut -d '.' -f 1`
base2=`echo $2 | cut -d '.' -f 1`

echo "perl scripts/core2prd.pl < $1 > $base1.prd"
perl scripts/core2prd.pl < $1 > $base1.prd

echo "./encode-rules --print-constants < $base1.prd > $base1.enc 2> $base1.dct"
./encode-rules --print-constants $base1-repls.enc < $base1.prd > $base1.enc 2> $base1.dct

echo "./der --print-index $base1.dct | awk '{print \"--force \"$2;}' > $base1.frc"
./der --print-index $base1.dct | awk '{print "--force "$2;}' > $base1.frc

echo "./der $2 -o $base2.der -i $base2.dct `cat $base1.frc`"
./der $2 -o $base2.der -i $base2.dct `cat $base1.frc`

echo "./infer-rules-xmt $base1.enc $base2.der $base2-closure.der"
./infer-rules-xmt $base1.enc $base2.der $base2-closure.der

echo "./der -d $base2-closure.der -i $base2.dct -o $base2-closure.nt"
./der -d $base2-closure.der -i $base2.dct -o $base2-closure.nt
