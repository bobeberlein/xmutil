#!/bin/bash
set -e

mkdir -p mac/lib
mkdir -p include

cd build

for p in icu_mac boost_mac tinyxml_mac; do
    ./${p}.bash || (echo '${p} failed' && exit 1)
done