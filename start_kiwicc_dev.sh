#!/bin/bash
ABS_DIR_PATH=$(cd $(dirname $0); pwd)/
docker run --rm -it -v $ABS_DIR_PATH:/kiwicc -w /kiwicc/kiwicc riscv_dev