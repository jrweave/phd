#!/bin/sh

if [ $# != 1 ]; then
	echo "Correct usage: $0 <rule-file>"
	exit
fi

base=`echo $1 | cut -d '.' -f 1`

make
perl scripts/core2prd.pl < $1 > $base.prd
./refine_rules < $base.prd > $base.cnf
egrep '^c' $base.cnf > $base-comments.cnf
UNSAT=`tail -n 1 $base-comments.cnf`
if [ "$UNSAT" == "c UNSATISFIABLE" ]; then
	echo 'UNSATISFIABLE'
	exit
fi
egrep '^[^c]' $base.cnf > $base-concise.cnf
relsat -p 3 -t n -#a $base-concise.cnf > $base.out
UNSAT=`tail -n 1 $base.out`
if [ "$UNSAT" == "UNSAT" ]; then
	echo 'UNSATISFIABLE'
	exit
fi
perl scripts/decode.pl $base-comments.cnf < $base.out | perl scripts/curies.pl > $base.asgn
perl scripts/catasgn.pl `perl scripts/pickasgn.pl < $base.asgn | cut -d ' ' -f 1` < $base.asgn > $base.pick
more $base.pick
