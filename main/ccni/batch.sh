#!/bin/sh

# parse arguments to determine how to run

partition=''
constraint=''
nproc=''
mailtype=''
mailuser=''
time=''
slurmout=''
slurmerr=''
runargs=''
nodes=0
tasks_per_node=0
for arg in "$@"
do
	case "$arg" in
		-p=*)						partition=`echo $arg | cut -d '=' -f 2-`
										;;
		--constraint=*)	constraint=`echo $arg | cut -d '=' -f 2-`
										;;
		-n=*)						nproc=`echo $arg | cut -d '=' -f 2-`
										;;
		-N=*)						nodes=`echo $arg | cut -d '=' -f 2-`
										;;
		--tasks-per-node=*)  tasks_per_node=`echo $arg | cut -d '=' -f 2-`
										;;
		--mail-type=*)	mailtype=`echo $arg | cut -d '=' -f 2-`
										;;
		--mail-user=*)	mailuser=`echo $arg | cut -d '=' -f 2-`
										;;
		-t=*)						time=`echo $arg | cut -d '=' -f 2-`
										;;
		-o=*)						slurmout=`echo $arg | cut -d '=' -f 2-`
										;;
		-e=*)						slurmerr=`echo $arg | cut -d '=' -f 2-`
										;;
		--help)					echo "[USAGE] $0 -n=<number-of-processors>"
										echo "                   -t=<time-estimate>"
										echo "                   -p=<partition-name>"
										echo "                   [--constraint=<constraint-name>]"
										echo "                   [-o=<slurm-out-file>]"
										echo "                   [-e=<slurm-err-file>]"
										echo "                   [--mail-type=BEGIN|END|FAIL|REQUEUE|ALL|]"
										echo "                   [--mail-user=<email-address>]"
										echo "                   <mpi-executable> <program-arguments>"
										exit
										;;
		*)							runargs="$runargs $arg"
										;;
	esac
done

if [ "$partition" == "" ]; then
	echo "[ERROR] Must specify a partition using -p=<partition-name>"
	exit
fi

host=`hostname`
case "$host" in
	q.ccni.rpi.edu)		runscript="run-q.sh"
										;;
	osb00*)						runscript="run-$partition.sh"
										;;
	bgl16fen)					runscript="run-bgl16fen.sh"
										;;
	*)								echo "[ERROR] Unknown or unsupported host $host."
										exit
										;;
esac

if [ ! -e "$runscript" ]; then
	echo "[ERROR] Not such run script: $runscript"
	exit
fi
batchcmd="sbatch -p $partition"

if [ "$constraint" != "" ]; then
	batchcmd="$batchcmd --constraint=$constraint"
fi

if [ "$nproc" == "" ]; then
	echo "[ERROR] Some number of processors must be specified using -n=<number-of-processors>"
	exit
fi
batchcmd="$batchcmd -n $nproc"

if [ $nodes -ne 0 ]; then
	batchcmd="$batchcmd -N $nodes"
fi

if [ $tasks_per_node -ne 0 ]; then
	batchcmd="$batchcmd --tasks-per-node=$tasks_per_node"
fi

if [ "$mailtype" != "" ]; then
	batchcmd="$batchcmd --mail-type=$mailtype"
fi

if [ "$mailuser" != "" ]; then
	batchcmd="$batchcmd --mail-user=$mailuser"
fi

if [ "$time" == "" ]; then
	echo "[ERROR] A time estimate must be specified using -t=<number-of-minutes>"
	exit
fi
batchcmd="$batchcmd -t $time"

if [ "$slurmout" != "" ]; then
	batchcmd="$batchcmd -o $slurmout"
fi

if [ "$slurmerr" != "" ]; then
	batchcmd="$batchcmd -e $slurmerr"
fi

batchcmd="$batchcmd $runscript $runargs"

#echo "Does the following batch command seem right to you (y/n)?  $batchcmd"
#echo
#read Y
#if [ "$Y" != "y" ]; then
#	echo "JOB WAS NOT BATCHED BECAUSE YOU DID NOT ENTER 'y'."
#	exit
#else
#	$batchcmd
#fi
echo "$batchcmd"
x=`$batchcmd`
echo "$x"
