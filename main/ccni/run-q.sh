#!/bin/bash

export MALLOC_MMAP_MAX_=0
srun $@
