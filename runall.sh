#!/bin/bash

for i in 1 2 4 8 16 32
do 
    echo "[runall -- running with]: OMP_NUM_THREADS=$i $1 $2 $3 $4 $5 $6 $7"
    echo "==============================================================================================="
    OMP_NUM_THREADS=$i srun --nodes=1 $1 $2 $3 $4 $5 $6 $7
    echo "==============================================================================================="
done
