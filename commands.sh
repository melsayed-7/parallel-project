#!/bin/bash
g++ -fopenmp -o gen filegen.cpp
# sleep 5
./gen
mpicc -o p p1.c
mpirun -n 4 ./p



