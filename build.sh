#!/bin/bash

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

if [[ $? -ne 0 ]]; then
    rm -r build/*
fi

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

cmake --build build
