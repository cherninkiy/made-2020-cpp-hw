#!/bin/bash

set -e

g++ -std=c++17 -I./src test/test.cpp -o chunk_allocator_test
./chunk_allocator_test

echo Test complete!