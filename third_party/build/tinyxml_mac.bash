#!/bin/bash

git clone https://github.com/leethomason/tinyxml2.git
cd tinyxml2
git checkout 5.0.1
make -j4


mkdir -p ./../../mac/lib
cp -a libtinyxml2.a ./../../mac/lib
cp -a tinyxml2.h ./../../include
