#!/bin/bash

set -e

g++ -std=c++17 -I./ test/test.cpp -o functional_test
./functional_test