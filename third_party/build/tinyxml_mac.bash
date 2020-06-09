#!/bin/bash

git clone https://github.com/leethomason/tinyxml2.git
cd tinyxml2
git checkout 8.0.0
make -j4


mkdir -p ./../../mac/lib
cp -a libtinyxml2.a ./../../mac/lib
cp -a tinyxml2.h ./../../include
