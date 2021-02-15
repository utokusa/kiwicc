#!/bin/bash
set -e

TMP=$1
CC=$2
OUTPUT=$3

rm -rf $TMP
mkdir -p $TMP

kiwicc() {
  riscv64-unknown-linux-gnu-gcc -c -o $TMP/${1%.c}.o $1
}

kiwicc main.c
kiwicc preprocess.c
kiwicc parse.c
kiwicc codegen.c
kiwicc tokenize.c
kiwicc type.c

(cd $TMP; riscv64-unknown-linux-gnu-gcc -o ../$OUTPUT *.o)
