#!/bin/bash -x

hostname -s > /tmp/hosts.$SLURM_JOB_ID

if [ "x$SLURM_NPROCS" = "x" ] 
then
	if [ "x$SLURM_NTASKS_PER_NODE" = "x" ] 
	then
		SLURM_NTASKS_PER_NODE=1
	fi
	SLURM_NPROCS=`expr $SLURM_JOB_NUM_NODES \* $SLURM_NTASKS_PER_NODE`
fi

#MVAPICH:  "srun --mpi=mvapich <executable> <executable input arguments>"
#MVAPICH2: "srun --mpi=none <executable> <executable input arguments>"
#OpenMPI:  "mpirun -machinefile /tmp/hosts.$SLURM_JOB_ID -np $SLURM_NPROCS <executable> <executable input arguments>"

mpirun -machinefile /tmp/hosts.$SLURM_JOB_ID -np $SLURM_NPROCS $@

rm /tmp/hosts.$SLURM_JOB_ID
