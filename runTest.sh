#!/bin/bash

for grain_size in 64 128 256
do
    for i in 1 2 4 8 16
    do 
        echo "[runall -- running with]: OMP_NUM_THREADS=$i"
        export OMP_NUM_THREADS=$i
        ./bin/gpsa --x data/X2.txt --y data/Y2.txt --grain-size $grain_size --exec-mode 3 >> timings
    done
done

