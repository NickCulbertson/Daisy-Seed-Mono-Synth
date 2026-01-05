#!/bin/bash

./ci/build_libs.sh

echo "building main project . . ."
make clean && make
if [ $? -ne 0 ]
then
    echo "Failed to compile main project"
    exit 1
fi

echo "finished"