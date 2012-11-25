#!/bin/sh

if [ $# != 1 ]; then
	echo "Correct usage: parts.sh <rule-file>"
	exit
fi

base=`echo $1 | cut -d '.' -f 1`

perl scripts/core2prd.pl < $1 > $base.prd
echo '# These rules represent semantics that have been sacrificed for performance.' > $base-pruned.prd

./refine-all < $base.prd > $base-refined.prd 2>> $base.err
./prd2cnf < $base-refined.prd > $base.cnf 2>> $base.err

relsat -p 3 -t n -#a $base.cnf | perl scripts/picknasgn.pl $base.cnf > $base-picked.asgn 2>> $base-relsat-notes.txt
perl scripts/curies.pl < $base-picked.asgn
