#!/bin/bash

# How to run this script: 
#
# DIR/rissAuto.sh DIR BENCHNAME RANDOMSEED TIMELIMIT MEMLIMIT NBCORE
# 
#    Path and filenames DIR and BENCHNAME are assumed to contain no spaces.
#

DIR=${1}              # Path to solver binary
BENCHNAME=${2}
RANDOMSEED=${3}
TIMELIMIT=${4}
MEMLIMIT=${5}         # In MiB
CORES=${6}

SOLVER=${DIR}/riss

# run solver
$SOLVER $BENCHNAME -Pconftime $TIMELIMIT
