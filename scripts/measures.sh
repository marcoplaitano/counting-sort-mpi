#!/bin/bash

# File:   measures.sh
# Brief:  This script compiles and runs the C program multiple times to measure
#         its execution time taking into account different parameters.
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
    Measures

SYNOPSIS
    measures.sh [OPTION]...

DESCRIPTION
    This script compiles and runs the C program in the src/ directory to measure
    its execution time taking into account different values of the following
    parameters:
        - Problem's size
        - Compiling optimization level
        - Number of processes used.
    Each combination is tried out multiple times; when all measures are done,
    the 'evaluate.py' script is launched to estimate means and draw tables and
    plots of the newly generated results.

OPTIONS
    -h, --help
        Show this guide and exit.

    -n N, --num-measures N
        Run every measure N times. (default is 50)

    -d DIR, --dir DIR
        Write all the output in the DIR directory of your choice instead of the
        default location './output/'.

    --no-plot
        Do not run the Python script to generate the plots." | more -d
}


# Delete all temporary files before exiting.
# Argument $1 is the exit code.
function safe_exit {
    echo $'\n'"Deleting temporary output..."
    [[ -f "$tmp_file" ]]    && rm "$tmp_file"
    [[ -d "$project_dir"/scripts/__pycache__ ]] && \
    rm -r "$project_dir"/scripts/__pycache__
    echo "Exiting script."
    exit $1
}

# Redirect to the safe_exit function if user presses 'CTRL+C'.
trap "safe_exit 0" SIGINT


# Print error message (if any) and interrupt the execution of the script.
function raise_error {
    [[ -n "$1" ]] && echo "$1" || echo "Error."
    safe_exit 1
}


# Compile source file(s) via makefile.
# Argument $1 is the target: either 'serial' or 'parallel'.
# Argument $2 is the level of optimization to use when compiling.
function compile {
    make -C "$project_dir" clean > /dev/null 2>&1

    make -C "$project_dir" $1 OPT_LEVEL=$2 > /dev/null

    [[ $? != 0 ]] && raise_error
}


# Measure the execution time and save the results on a file.
function measure_time {
    # First line in CSV file declares the columns format.
    echo "size;processes;time_init;time_sort;time_elapsed;sys;real" \
    > "$output_file"

    # Show initial 0% progress.
    printf "\r[  0/%d   0%%]" $num_measures

    for (( i=1; i<=$num_measures; i++ )); do
        # Time the execution of the program and save the result in a temp file.
        if [[ $num_proc == 0 ]]; then
            (
                time "$exec_file" "${exec_args[@]}"
            ) > "$tmp_file" 2>&1
        else
            (
                time mpiexec -np $num_proc "$exec_file" "${exec_args[@]}"
            ) > "$tmp_file" 2>&1
        fi

        # Exit the script if the execution produced an error.
        if [[ $? != 0 ]]; then
            raise_error "An error occurred during the execution of the program."
        fi

        # Normalize temporary output and save it.
        cat "$tmp_file" | sed -e 's/,/./g' >> "$output_file"

        # Show current progress.
        printf "\r[%3d/%d %3d%%] " $i $num_measures $(($i*100 / $num_measures))
        printf "=%.0s" $(seq -s " " 1 $(( $i * 50 / $num_measures)) )
    done
    echo
}


# Return a string representing a number with leading zeros.
# Argument $1 is the number to modify.
# Argument $2 is the maximum value to use as scale when deciding the number of
# zeros to add.
# Example:
#   $1 = 5, $2 = 10  produces the output:  05
#   $1 = 5, $2 = 100 produces the output: 005
function add_leading_zeros {
    printf "%0*d" ${#2} $1
}



# Parse command line arguments.
while [[ -n $1 ]]; do
    case $1 in
        -h | --help)
            show_guide
            exit 0 ;;
        -n | --num-measures)
            num_measures=$2
            [[ -z $num_measures ]] && raise_error "No number of measures given."
            shift ; shift ;;
        -d | --dir)
            output_dir="$2"
            [[ -z $output_dir ]] && raise_error "No output directory given."
            shift ; shift ;;
        --no-plot)
            no_plot=1
            shift ;;
        *)
            raise_error "Argument '$1' not recognized." ;;
    esac
done

# Check that the argument is actually a positive integer
if [[ -n $num_measures ]] && [[ ! $num_measures =~ ^[0-9]+$ ]]; then
    raise_error "Not a valid number of measures."
fi


# Determine root project directory based on whether this script has been
# launched from there or from the scripts/ subdirectory.
project_dir=$(pwd)
[[ $(pwd) == *scripts ]] && project_dir=${project_dir%"scripts"}

# Executable file compiled from C source that has to be launched.
exec_file="$project_dir/bin/main.out"

# Set number of measures to perform, if not given.
num_measures=${num_measures:="50"}

# Declare format in which to save execution time result:
# S: CPU-seconds used by the system on behalf of the process (in kernel mode)
# E: Elapsed real (wall clock) time used by the process.
TIMEFORMAT="%3S;%3E"

# All levels of optimization to apply when compiling.
optimization_levels=(0 1 2 3)

# Different sizes for the array in the program.
sizes=(10000 500000 2000000)

# All number of processes to execute the program with.
# When this value is 0 the serial version is launched.
num_processes=(0 1 2 4 8 16)

# If not given set directory in which to store measurements results.
output_dir=${output_dir:="$project_dir/output"}
# Create the directory if it does not exist yet.
[[ ! -d "$output_dir" ]] && mkdir -p "$output_dir"
# Delete any previously generated content (if user agrees).
if [[ "$(ls "$output_dir")" ]]; then
    echo "Warning: the output directory '""$output_dir""' is not empty."
    read -p "Delete previous output (all contents) from it? [y;N] " answer
    case $answer in
        [yY] | [yY][eE][sS])
            rm -Ir "$output_dir"/* ;;
        *)
            echo "Not deleting." ;;
    esac
fi
# File in which to store temporary output
tmp_file="$output_dir"/.temp.txt


for num_proc in ${num_processes[@]}; do
    for opt_lvl in ${optimization_levels[@]}; do
        # Do not run measures with compiling optimization O0 if the program
        # has to run with multiple processes.
        if (( $opt_lvl == 0 )) && (( $num_proc > 0 ))
            then continue
        fi

        # Based on the number of processes, update the make compilation target
        # to be either serial or parallel.
        [[ $num_proc == 0 ]] && target="serial" || target="parallel"

        compile $target $opt_lvl

        for size in ${sizes[@]}; do
            # Show current values
            printf "PROCESSES: $num_proc, OPTIMIZATION: $opt_lvl, "
            printf "SIZE: %'d\n" $size

            # Command line arguments to pass to the C program.
            exec_args=($size)

            # Add leading zeros to the size and num_proc variables in order to
            # create files which can be correctly sorted.
            lznum_proc=$(add_leading_zeros $num_proc ${num_processes[-1]})
            lzsize=$(add_leading_zeros $size ${sizes[-1]})

            # Subdirectory in which to store measures carried out with current
            # parameters.
            curr_output_dir="$output_dir"/size_$size\_opt_$opt_lvl
            mkdir -p "$curr_output_dir"

            # File in which to store current output.
            output_file="$curr_output_dir"/S$lzsize\_P$lznum_proc\_O$opt_lvl.csv

            measure_time
        done
    done
done

echo "All measures completed."

[[ -z $no_plot ]] && python3 "$project_dir"/scripts/evaluate.py "$output_dir"

echo "Done."

safe_exit 0
