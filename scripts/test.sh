#!/bin/bash

# File:   test.sh
# Brief:  This script compiles and runs the test file(s). User can choose
#         whether to test in serial or parallel mode.
# Author: Marco Plaitano
# Date:   27 Nov 2021
#
# COUNTING SORT MPI
# Parallelize and Evaluate Performances of "Counting Sort" Algorithm, by using
# MPI.
#
# Copyright (C) 2022 Plaitano Marco
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.


function show_guide {
    echo $"NAME
    Test

SYNOPSIS
    test.sh [OPTION]...

DESCRIPTION
    This script compiles and runs the test file(s) in the test/ directory. The
    user can choose whether to test the C program in serial or parallel mode.

OPTIONS
    -h, --help
        Show this guide and exit.

    --silent
        Suppress all test output except failure messages.

    -n N, --numproc N
        Use N processes in parallel execution. (default is 4)" | more -d
}


# Delete all temporary files before exiting.
# Argument $1 is the exit code.
function safe_exit {
    echo "Deleting temporary output..." > $out_stream
    make -C "$project_dir" clean > /dev/null 2>&1
    echo "Exiting script." > $out_stream
    exit $1
}


# Redirect to the safe_exit function if user presses 'CTRL+C'.
trap "safe_exit 0" SIGINT


# Print error message (if any) and interrupt the execution of the script.
function raise_error {
    [[ -n "$1" ]] && echo "$1" || echo "Error."
    out_stream="/dev/stdout"
    safe_exit 1
}



# Parse command line arguments.
while [[ -n $1 ]]; do
    case $1 in
        -h | --help)
            show_guide
            exit 0 ;;
        --silent)
            out_stream="/dev/null"
            shift ;;
        -n | --numproc)
            num_proc=$2
            [[ -z $num_proc ]] && raise_error "No number of processes given."
            shift ; shift ;;
        *)
            raise_error "Argument '$1' not recognized." ;;
    esac
done

# Check that the argument is actually a positive integer
if [[ -n $num_proc ]] && [[ ! $num_proc =~ ^[0-9]+$ ]]; then
    raise_error "Not a valid number of processes."
fi


# Default values.
num_proc=${num_proc:="4"}
out_stream=${out_stream:="/dev/stdout"}

# Determine root project directory based on whether this script has been
# launched from there or from the scripts/ subdirectory.
project_dir=$(pwd)
[[ $(pwd) == *scripts ]] && project_dir=${project_dir%"scripts"}

# Executable test file compiled with make.
executable_file="$project_dir/bin/test.out"

# Clean previous compilation output.
make -C "$project_dir" clean > /dev/null 2>&1

# Compile.
make -C "$project_dir" test > /dev/null
[[ $? != 0 ]] && raise_error

# Generate a file containing enough random integers to test the program with.
"$project_dir"/scripts/generate_numbers.sh
data_file="$project_dir"/data/numbers.dat

# Run.
mpiexec -np $num_proc "$executable_file" "$data_file" > $out_stream
[[ $? == 0 ]] && echo "All tests passed."

safe_exit 0
