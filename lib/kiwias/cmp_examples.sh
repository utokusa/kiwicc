#! /bin/bash

option=$3

# diff <(./run_example.sh $1 $3) <(./run_example.sh $2 $3)
./run_example.sh $1 $3 > tmp_out1.txt
./run_example.sh $2 $3 > tmp_out2.txt
diff tmp_out1.txt tmp_out2.txt

