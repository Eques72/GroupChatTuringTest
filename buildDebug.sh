#!/bin/bash

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

if [[ $? -ne 0 ]]; then
    rm -r build/*
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug

cmake --build build
