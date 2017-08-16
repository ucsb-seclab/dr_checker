#!/usr/bin/env bash
BASEDIR=$(dirname "$0")
cd $BASEDIR/AnalysisHelpers
./build.sh
cd ..
cd $BASEDIR/MainAnalysisPasses
./build.sh
