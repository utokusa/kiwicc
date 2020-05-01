#!/bin/bash
ABS_DIR_PATH=$(cd $(dirname $0); pwd)/
docker run --rm -it -v $ABS_DIR_PATH/9cc:/9cc compilerbook