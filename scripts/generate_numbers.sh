#!/bin/bash

# File:   measures.sh
# Brief:  This script writes random integers onto a binary file.
# Author: Marco Plaitano
# Date:   13 Jan 2022
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


# Delete all temporary files before exiting.
# Argument $1 is the exit code.
function safe_exit {
    [[ -f gen_num.c ]]   && rm gen_num.c
    [[ -f gen_num.out ]] && rm gen_num.out
    exit $1
}

# Redirect to the safe_exit function if user presses 'CTRL+C'.
trap "safe_exit 0" SIGINT


# Print error message and interrupt the execution of the script.
function raise_error {
    [[ -n "$1" ]] && echo "$1" || echo "Error."
    safe_exit 1
}



# Determine root project directory based on whether this script has been
# launched from there or from the scripts/ subdirectory.
project_dir=$(pwd)
[[ $(pwd) == *scripts ]] && project_dir=${project_dir%"scripts"}

# Create data/ directory.
[[ ! -d "$project_dir"/data ]] && mkdir "$project_dir"/data
file_path="$project_dir"/data/numbers.dat

# If the file containing the numbers already exists then nothing has to be done.
[[ -f "$file_path" ]] && safe_exit 0



# Create C program to generate the numbers with OpenMP parallelization.
echo $"
#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define MIN 0
#define MAX 100000
int main(int argc, char **argv) {
    if (argc < 4)
        return 201;
    const long long size = atoll(argv[1]);
    int nthreads = atoi(argv[2]);
    int *array = (int *)malloc(size * sizeof(int));
    if (array == NULL)
        return 202;
    long long i = 0;
    #pragma omp parallel num_threads(nthreads) shared(array, size) private(i)
    {
        unsigned seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp for
        for (i = 0; i < size; i++)
            array[i] = rand_r(&seed) % (MAX + 1 - MIN) + MIN;
    }
    FILE *file = fopen(argv[3], \"wb\");
    if (file == NULL) {
        free(array);
        return 203;
    }
    fwrite(array, sizeof(int), size, file);
    fclose(file);
    free(array);
    return 0;
}
" > gen_num.c

# Compile C program.
gcc -O3 -fopenmp gen_num.c -o gen_num.out
[[ $? != 0 ]] && raise_error "Compilation error."

# Run the program.
size=20000000
num_threads=4
./gen_num.out $size $num_threads "$file_path"

# Catch any error.
error_code=$?
if [[ $error_code == "201" ]];
    then raise_error "Not enough command line arguments."
elif [[ $error_code == "202" ]];
    then raise_error "Could not allocate needed memory."
elif [[ $error_code == "203" ]];
    then raise_error "Could not open file to write on it."
elif [[ $error_code != "0" ]];
    then raise_error "Execution error."
else
    printf "Written %'d integers in file '""$file_path""'\n" $size
fi

safe_exit 0
