#/bin/sh

main=`pwd`/..
local=`pwd`/jobs
jobs='/gpfs/lb/data/DSSW/DSSWweav/jobs'
blocksize=`expr 4 \* 1024 \* 1024`
pagesize=`expr 4 \* 1024 \* 1024`

rulefile=''
datafile=''
dataformat=''
dictfile=''
indexfile=''
nproc=''
decode=''
batchargs=''
runargs=''
packetsize=''
checkevery=''
numrequests=''

for arg in "$@"
do
	case "$arg" in
		--packet-size=*)	packetsize=`echo "$arg" | cut -d '=' -f 2-`
											runargs="$runargs --packet-size $packetsize"
											;;
		--check-every=*)	checkevery=`echo "$arg" | cut -d '=' -f 2-`
											runargs="$runargs --check-every $checkevery"
											;;
		--num-requests=*)	numrequests=`echo "$arg" | cut -d '=' -f 2-`
											runargs="$runargs --num-requests $numrequests"
											;;
		--decode)					decode='1'
											;;
		--rules=*)				rulefile=`echo "$arg" | cut -d '=' -f 2-`
											;;
		--data=*)					datafile=`echo "$arg" | cut -d '=' -f 2-`
											;;
		--dict=*)					datafile=`echo "$arg" | cut -d '=' -f 2-`
											;;
		--index=*)				indexfile=`echo "$arg" | cut -d '=' -f 2-`
											;;
		--format=*)				dataformat=`echo "$arg" | cut -d '=' -f 2-`
											;;
		-n=*)							nproc=`echo "$arg" | cut -d '=' -f 2-`
											batchargs="$batchargs $arg"
											;;
		*)								batchargs="$batchargs $arg"
											;;
	esac
done

if [ "$rulefile" == "" ]; then
	echo "[ERROR] Must specify a rule file using --rules=<file-name>"
	exit
fi

if [ "$datafile" == "" ]; then
	echo "[ERROR] Must specify a data file using --data=<file-name>"
	exit
fi

if [ "$nproc" == "" ]; then
	echo "[ERROR] Must specify number of processors using -n=<number-of-processors>"
	exit
fi

if [ "$dataformat" == "" ]; then
	maybe=`echo "$datafile" | cut -d '.' -f 2-`
	case "$maybe" in
		nt.lzo)		dataformat="nt.lzo"
							;;
		lzo)			dataformat="nt.lzo"
							;;
		der)			dataformat="der"
							;;
		nt)				dataformat="nt"
							;;
		*)				echo "[ERROR] No --format specified, and cannot infer format from file extension $maybe."
							exit
							;;
	esac
fi

case "$dataformat" in
	nt.lzo)		if [ "$indexfile" == "" ]; then
							echo "[ERROR] --index must be specified when using nt.lzo format."
							exit
						fi
						;;
	nt)				;;
	*)				echo "[ERROR] Unrecognized/unsupported format $dataformat."
						exit
						;;
esac

id=`date "+%Y-%m-%dT%H-%M-%S%z"`
rbase=`echo $rulefile | perl -ne 'if ($_ =~ /^(?:.*\/)?([^\/\.]+)(?:.[^\/]*)?$/) { print "$1\n"; }'`
dbase=`echo $datafile | perl -ne 'if ($_ =~ /^(?:.*\/)?([^\/\.]+)(?:.[^\/]*)?$/) { print "$1\n"; }'`
if [ "$rbase" = "$dbase" ]; then
	rbase="rules-$rbase"
	dbase="data-$dbase"
fi
id="$id--$rbase--$dbase"

mkdir $local/$id
mkdir $jobs/$id
i=0
while [ $i -lt $nproc ]; do
	mkdir $jobs/$id/$i
	i=`expr $i + 1`
done

batchargs="$batchargs -o=$local/$id/slurm-%j.out -e=$local/$id/slurm-%j.err"

echo "===== PARAMETERS ====="
echo "Rules: $rulefile"
echo "Data: $datafile"
echo "Index: $indexfile"
echo "Dictionary: $dictfile (ignore this)"
echo "Format: $dataformat"
echo "Processors: $nproc"
echo "Packet size: $packetsize"
echo "Check every: $checkevery"
echo "Number of requests: $numrequests"
echo
echo "===== DIRECTORIES ====="
echo "Main: $main"
echo "Jobs: $jobs"
echo "Local: $local"
echo
echo "===== EXECUTION ====="
echo "ID: $id"

d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] perl ../scripts/core2prd.pl < $rulefile > $local/$id/$rbase.prd"
perl ../scripts/core2prd.pl < $rulefile > $local/$id/$rbase.prd

d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] ../encode-rules --print-constants $local/$id/$rbase-repls.enc < $local/$id/$rbase.prd > $local/$id/$rbase.enc 2> $local/$id/$rbase.dct"
../encode-rules --print-constants $local/$id/$rbase-repls.enc < $local/$id/$rbase.prd > $local/$id/$rbase.enc 2> $local/$id/$rbase.dct

d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] ../der --print-index $local/$id/$rbase.dct | awk '{print \"--force \"\$2;}' > $local/$id/$rbase.frc"
../der --print-index $local/$id/$rbase.dct | awk "{print \"--force \"\$2;}" > $local/$id/$rbase.frc

cmd="$main/red-mpi --time -b $blocksize -p $pagesize -si -if $dataformat -i $datafile -of der -o $jobs/$id/#/$dbase.der -od $jobs/$id/#/$dbase.dct --global-dict $runargs"
if [ "$dataformat" == "nt.lzo" ]; then
	cmd="$cmd -ix $indexfile"
fi
forces=`cat $local/$id/$rbase.frc`
cmd="$cmd $forces"
d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] ./batch.sh $batchargs $cmd"
slurmid=`./batch.sh $batchargs $cmd 2>&1 | tail -n 1 | perl -ne 'if ($_ =~ /(\d+)/) { print "$1\n"; }'`
if [ "$slurmid" == "" ]; then
	echo "[ERROR] No slurmid was determined after submitting encoding job."
	exit
fi
echo "Waiting on slurm job $slurmid to finish."
check=`squeue -h -j$slurmid`
while [ "$check" != "" ]; do
	sleep 15
	check=`squeue -h -j$slurmid`
done

cmd="$main/infer-rules-mpi $local/$id/$rbase.enc $jobs/$id/#/$dbase.der $jobs/$id/#/$dbase-closure.der $local/$id/$rbase-repls.enc --page-size $pagesize --randomize --uniq $runargs"
d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] ./batch.sh $batchargs $cmd"
slurmid=`./batch.sh $batchargs $cmd 2>&1 | tail -n 1 | perl -ne 'if ($_ =~ /(\d+)/) { print "$1\n"; }'`
if [ "$slurmid" == "" ]; then
	echo "[ERROR] No slurmid was determined after submitting inference job."
	exit
fi
echo "Waiting on slurm job $slurmid to finish."
check=`squeue -h -j$slurmid`
while [ "$check" != "" ]; do
	sleep 15
	check=`squeue -h -j$slurmid`
done

if [ "$decode" != "" ]; then
	cmd="$main/red-mpi --time -b $blocksize -p $pagesize -if der -i $jobs/$id/#/$dbase-closure.der -id $jobs/$id/#/$dbase.dct --global-dict -of nt -o $jobs/$id/#/$dbase-closure.nt $runargs"
	d=`date "+Y-%m-%dT%H:%M:%S%z %s"`
	echo "[$d] ./batch.sh $batchargs $cmd"
	slurmid=`./batch.sh $batchargs $cmd 2>&1 | tail -n 1 | perl -ne 'if ($_ =~ /(\d+)/) { print "$1\n"; }'`
	if [ "$slurmid" == "" ]; then
		echo "[ERROR] No slurmid was determined after submitting decoding job."
		exit
	fi
	echo "Waiting on slurm job $slurmid to finish."
	check=`squeue -h -j$slurmid`
	while [ "$check" != "" ]; do
		sleep 15
		check=`squeue -h -j$slurmid`
	done
fi

d=`date "+%Y-%m-%dT%H:%M:%S%z %s"`
echo "[$d] DONE"
