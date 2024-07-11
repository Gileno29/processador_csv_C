#!/bin/sh


#!/bin/sh

# Exit immediately if a command exits with a non-zero status
set -e

# Ensure the script is running in the /app directory
cd "$(dirname "$0")"

# Variables
LIB_NAME="libcsvprocessor.so"
MAIN_EXEC="test_libcsv"

# Compilation flags
CFLAGS="-fPIC -Wall -Wextra -O2"
LDFLAGS="-shared"
INCLUDE_DIRS="-I."

# Source files
SOURCES="csv_processor.c"

# Compile the shared library
gcc $CFLAGS $INCLUDE_DIRS -o $LIB_NAME $SOURCES $LDFLAGS

# Compile the main program and link it with the shared library
gcc $CFLAGS $INCLUDE_DIRS -o $MAIN_EXEC main.c -L. -lcsvprocessor

# Make the main executable runnable
chmod +x $MAIN_EXEC

echo "Build complete: $LIB_NAME and $MAIN_EXEC"

apt-get install gcc
gcc -c libcsv.c -o libcsv.o
gcc libcsv.o  -o libcsv.so