#!/bin/bash
LLVM_DIR=$LLVM_ROOT/../cmake
echo "[*] Trying to Run Cmake"
cmake .
echo "[*] Trying to make"
make -j8
