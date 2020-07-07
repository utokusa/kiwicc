#!/bin/bash
ABS_DIR_PATH=$(cd $(dirname $0); pwd)/
docker run --rm -it -v $ABS_DIR_PATH/kiwicc:/kiwicc kiwicc_dev