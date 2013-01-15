#!/bin/bash

srun --runjob-opts='--envs MALLOC_MMAP_MAX_=0 BG_MAPCOMMONHEAP=1' $@
