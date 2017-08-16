#!/usr/bin/env bash
BASEDIR=$(dirname "$0")
cd $BASEDIR/Dr_linker
./build.sh
cd ..
cd $BASEDIR/EntryPointIdentifier
./build.sh
