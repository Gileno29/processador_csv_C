#!/bin/sh
apt-get install gcc
gcc -c libcsv.c -o libcsv.o
gcc libcsv.o  -o libcsv.so