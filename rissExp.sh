#!/bin/bash

# How to run this script: 
#
# DIR/rissExp.sh DIR BENCHNAME RANDOMSEED TIMELIMIT MEMLIMIT NBCORE
# 
#    Path and filenames DIR and BENCHNAME are assumed to contain no spaces.
#

DIR=${1}              # Path to solver binary
BENCHNAME=${2}
RANDOMSEED=${3}
TIMELIMIT=${4}
MEMLIMIT=${5}         # In MiB
CORES=${6}            

SOLVER=${DIR}/rissExp

# run solver with extra settings
$SOLVER $BENCHNAME -Ppropagation csp -Pdup_long_conflicts 1 -Pdup_lastBinCon 1 -Prestart_event dynamic
