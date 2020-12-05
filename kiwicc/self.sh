#!/bin/bash
set -e

TMP=$1
CC=$2
OUTPUT=$3

rm -rf $TMP
mkdir -p $TMP

kiwicc() {
  (cd $TMP; ../$CC ../$1 -o ${1%.c}.s -I..)
  gcc -c -o $TMP/${1%.c}.o $TMP/${1%.c}.s
}

kiwicc main.c
kiwicc preprocess.c
kiwicc parse.c
kiwicc codegen.c
kiwicc tokenize.c
kiwicc type.c

(cd $TMP; gcc -o ../$OUTPUT *.o)
