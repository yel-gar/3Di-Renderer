#!/bin/bash

set -e

CXX=clang++ meson setup buildDir --buildtype=release --prefix=$HOME/.local -Dtests=false
meson compile -C buildDir
meson install -C buildDir
