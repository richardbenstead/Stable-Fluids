#!/bin/bash
set -euxo pipefail

BUILD_PATH=build
mkdir -p ${BUILD_PATH}
pushd ${BUILD_PATH}
cmake -DCMAKE_BUILD_TYPE=Release ..
# cmake -DCMAKE_BUILD_TYPE=Debug ..
make
