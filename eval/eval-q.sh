#!/bin/sh

datadir='/gpfs/lb/provisioned/DSSW/DSSWweav'
cd ..
rulesdir=`pwd`/main/testfiles
cd -

override_makefile() {
	len=`wc -l Makefile.bkp | perl -ne 'if ($_ =~ /(\d+)/) { print "$1\n"; }'`
	offset=`grep -n '#OVERRIDES' Makefile.bkp | cut -d ':' -f 1 | head -n 1`
	CC=$1
	LD=$1
	if [ "$CC" == "mpixlcxx" ]; then
		PRJCFLAGS="-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DMPICH_IGNORE_CXX_SEEK -DSYSTEM=SYS_BLUE_GENE_Q -DUCS_TRUST_CODEPOINTS -DUCS_PLAY_DUMB -DUSE_POSIX_MEMALIGN -DCACHE_LOOKUPS=1 -O3 -qstrict"
		USE_3RD_LZO="yes"
		USE_PAR_MPI="yes"
	else
		PRJCFLAGS="-D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DMPICH_IGNORE_CXX_SEEK -DUCS_TRUST_CODEPOINTS -DUCS_PLAY_DUMB"
		USE_3RD_LZO="no"
		USE_PAR_MPI="no"
	fi

	case $2 in
		*rdfs*)		PRJCFLAGS="$PRJCFLAGS -DTUPLE_SIZE=4"
							;;
		*owl*)		PRJCFLAGS="$PRJCFLAGS -DTUPLE_SIZE=7"
							;;
	esac

	head -n $offset Makefile.bkp > Makefile.inc
	echo "USE_3RD_LZO=$USE_3RD_LZO" >> Makefile.inc
	echo "USE_PAR_MPI=$USE_PAR_MPI" >> Makefile.inc
	echo "CC=$CC" >> Makefile.inc
	echo "LD=$LD" >> Makefile.inc
	echo "PRJCFLAGS=$PRJCFLAGS" >> Makefile.inc
	echo >> Makefile.inc
	tail -n `expr $len - $offset` Makefile.bkp >> Makefile.inc
}

MYPWD=`pwd`
cd ../
make clean
module unload xl
module load gnu
override_makefile g++ ''
make
cp main/der main/der-fe
cp main/encode-rules main/encode-rules-fe
make clean
module unload gnu
module load xl
override_makefile mpixlcxx ''
make
cp main/der-fe main/der
cp main/encode-rules-fe main/encode-rules
cd main/ccni

for mem in 4096 2048 8192
do
	for packet_size in 128 64 256
		do
		for check_every in 4096 2048 8192
			do
			for num_requests in 8 16 32
			do
				for rules in parminrdfs parowl2
				do
					cd $MYPWD/..
					override_makefile mpixlcxx $rules
					cd main
					rm infer-rules-mpi
					make
					cd ccni
					for data in norm-timbl-data-4 lubm10k uniq-btc-2012
					do
						case $data in
							uniq-btc-2012)		mem_per_proc=4
																;;
							*)								mem_per_proc=1
																;;
						esac
						tasks=`expr $mem / $mem_per_proc`
						tasks_per_node=`expr 16 / $mem_per_proc`
						nodes=`expr $tasks / $tasks_per_node`
						time=`expr 40 \* 4096 / $tasks`
						cmd="./infer.sh -n=$tasks -N=$nodes --tasks-per-node=$tasks_per_node -t=$time -p=debug --rules=$rulesdir/$rules.core --data=$datadir/$data.nt.lzo --index=$datadir/$data.nt.idx --packet-size=$packet_size --check-every=$check_every --num-requests=$num_requests"
						echo "$cmd"
						$cmd
					done
				done
			done
		done
	done
done

cd $MYPWD
