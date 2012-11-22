#!/bin/sh

if [ $# != 1 ]; then
	echo "Correct usage: parts.sh <rule-file>"
	exit
fi

base=`echo $1 | cut -d '.' -f 1`

perl scripts/core2prd.pl < $1 > $base-refined.prd
egrep "^#PRAGMA" $base-refined.prd > $base-pragmas.prd
rm -f $base.err $base-relsat-notes.txt
echo '' > $base.prd
echo '# These rules represent semantics that have been sacrificed for performance.' > $base-pruned.prd

n=0
cmp $base.prd $base-refined.prd > /dev/null 2> /dev/null
cmpeq=$?

while [ $cmpeq -ne 0 ]
do
	n=`expr $n + 1`
	echo "===== Iteration $n ====="
	echo "===== Iteration $n =====" >> $base.err
	mv $base-refined.prd $base.prd
	./prd2cnf < $base.prd > $base.cnf 2>> $base.err

	#relsat -#a $base.cnf | perl decode.pl $base.cnf > $base.asgn
	#perl scripts/pickasgn.pl < $base.asgn > $base-picked.asgn
	echo "ccccc Iteration $n ccccc" >> $base-relsat-notes.txt
	#relsat -p 3 -#a $base.cnf | perl decode.pl $base.cnf 2>> $base-relsat-notes.txt | perl scripts/pickasgn.pl > $base-picked.asgn
	relsat -p 3 -t n -#a $base.cnf | perl scripts/picknasgn.pl $base.cnf > $base-picked.asgn 2>> $base-relsat-notes.txt

	perl scripts/curies.pl < $base-picked.asgn
  ./refine $base-picked.asgn < $base.prd > $base-refined-$n.prd 2>> $base.err
	cp $base-pragmas.prd $base-refined.prd
	egrep "^#PRAGMA" $base-refined-$n.prd >> $base-refined.prd
	egrep -v "^#" $base-refined-$n.prd >> $base-refined.prd
	echo "##### Iteration $n #####" >> $base-pruned.prd
	egrep "^#PRUNED" $base-refined-$n.prd | cut -d ' '  -f 2- >> $base-pruned.prd
	cmp $base.prd $base-refined.prd > /dev/null 2> /dev/null
	cmpeq=$?
done
