#!/bin/bash

# 	DIRECTORIES
BINARY_DIR="bin"
#	BINARY
FILTER="main"

#	OPTIONS
r=1200
k=2
l=20
tests=30
pot_t=4
even=2
for t in $(seq 0 $pot_t);
do
    t=$(($even**$t))
    for i in $(seq 1 $tests);
    do
        ./$BINARY_DIR/$FILTER $r $l $k $t
    done
done
