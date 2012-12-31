#/bin/sh

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
