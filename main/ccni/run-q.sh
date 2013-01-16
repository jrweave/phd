#!/bin/bash

srun --runjob-opts='--mapping=ABCDET --envs MALLOC_MMAP_MAX_=0 BG_MAPCOMMONHEAP=1' $@
