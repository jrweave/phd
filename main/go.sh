#!/bin/sh

if [ $# -lt 1 ]; then
	echo "Correct usage: $0 <rule-file> <rules-to-sat-args>"
	exit
fi

input=$1
base=`echo $input | cut -d '.' -f 1`
ext=`echo $input | cut -d '.' -f 2`

shift

echo "Make sure everything is compiled."
echo "  make"
make

echo "If not explicitly a PRD file, assume it is CORE and try to translate it."
if [ "$ext" != "prd" ]; then
	echo "  perl scripts/core2prd.pl < $input > $base.prd"
	perl scripts/core2prd.pl < $input > $base.prd
fi

#echo "RULE REFINEMENT"
#echo "  ./refine_rules < $base.prd > $base.cnf"
#./refine_rules < $base.prd > $base.cnf
echo "Rules to SAT"
echo "./rules-to-sat $@ < $base.prd > $base.cnf"
./rules-to-sat $@ < $base.prd > $base.cnf

echo "Put the comments in a separate file"
echo "  egrep '^c' $base.cnf > $base-comments.cnf"
egrep '^c' $base.cnf > $base-comments.cnf

echo "Check for early detection of unsatifiability"
UNSAT=`tail -n 1 $base-comments.cnf`
if [ "$UNSAT" == "c UNSATISFIABLE" ]; then
	echo 'UNSATISFIABLE'
	exit
fi

echo "Remove long comments from cnf file so that relsat doesn't have a stroke"
echo "  egrep '^[^c]' $base.cnf > $base-concise.cnf"
egrep '^[^c]' $base.cnf > $base-concise.cnf

echo "Use relsat to print ALL possible solutions"
echo "  relsat -p 3 -t n -#a $base-concise.cnf > $base.out"
relsat -p 3 -t n -#a $base-concise.cnf > $base.out

echo "Check for unsatisifability again"
UNSAT=`tail -n 1 $base.out`
if [ "$UNSAT" == "UNSAT" ]; then
	echo 'UNSATISFIABLE'
	exit
fi

echo "Decode the SAT solution"
echo "  perl scripts/decode.pl $base-comments.cnf < $base.out | perl scripts/curies.pl > $base.asgn"
perl scripts/decode.pl $base-comments.cnf < $base.out | perl scripts/curies.pl > $base.asgn

echo "Heuristically pick a solution"
echo "  perl scripts/catasgn.pl `perl scripts/pickasgn.pl < $base.asgn | cut -d ' ' -f 1` < $base.asgn > $base.pick"
perl scripts/catasgn.pl `perl scripts/pickasgn.pl < $base.asgn | cut -d ' ' -f 1` < $base.asgn > $base.pick

echo "Show it to the user"
echo "  more $base.pick"
more $base.pick
