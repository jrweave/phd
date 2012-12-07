#!/bin/sh
make
perl scripts/core2prd.pl < scripts/owl2rl.core > scripts/owl2rl.prd
./refine_rules < scripts/owl2rl.prd > owl2rl.cnf
relsat -p 3 -t n -#a owl2rl.cnf > owl2rl.out
perl scripts/decode.pl owl2rl.cnf < owl2rl.out | perl scripts/curies.pl > owl2rl.asgn
perl scripts/catasgn.pl `perl scripts/pickasgn.pl < owl2rl.asgn | cut -d ' ' -f 1` < owl2rl.asgn > owl2rl.pick
more owl2rl.pick
