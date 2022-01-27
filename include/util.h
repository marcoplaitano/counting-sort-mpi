/**
 * @file util.h
 * @brief This file contains some useful, general-purpose functions.
 * @author Marco Plaitano
 * @date 27 Nov 2021
 *
 * COUNTING SORT MPI
 * Parallelize and Evaluate Performances of "Counting Sort" Algorithm, by using
 * MPI.
 *
 * Copyright (C) 2022 Plaitano Marco
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef UTIL_H
#define UTIL_H

#include <sys/time.h>

/** @brief Minimum integer value accepted in the array. */
#define RANGE_MIN 0

/** @brief Maximum integer value accepted in the array. */
#define RANGE_MAX 100000

/**
 * @brief Path to the file containing random numbers to fill the array with.
 *
 * The file contains 20'000'000 positive integers in the range #RANGE_MIN to
 * #RANGE_MAX.
 */
#define INPUT_FILE_PATH "data/numbers.dat"


/**
 * @brief Start measuring the passing of time.
 * @param var: Name of the `double` variable in which to save measurements.
 *
 * The measurement is taken on Wall-Clock time by calling the `gettimeofday()`
 * function.
 */
#define START_TIME(var) \
    struct timeval begin_##var, end_##var; \
    MPI_Barrier(MPI_COMM_WORLD); \
    gettimeofday(&begin_##var, 0);

/**
 * @brief Stop measuring the passing of time and save the result in `var`.
 * @param var: Name of the `double` variable in which to save measurements.
 *
 * The measurement is taken on Wall-Clock time by calling the `gettimeofday()`
 * function. The function is supposed to have already been called before, via
 * the #START_TIME macro, on the same variable.
 * `var` will contain the difference (in seconds) of the values got by the two
 * calls to `gettimeofday()`.
 */
#define END_TIME(var) \
    gettimeofday(&end_##var, 0); \
    long seconds_##var = end_##var.tv_sec - begin_##var.tv_sec; \
    long microseconds_##var = end_##var.tv_usec - begin_##var.tv_usec; \
    var = seconds_##var + microseconds_##var * 1e-6;


/**
 * @brief Allocate `size` bytes of memory and check that the operation is
 *        successful.
 * @param size: Number of bytes to allocate.
 * @return Pointer to the memory allocated.
 */
void *safe_alloc(long long size);

/**
 * @brief Fill the given array with random integers.
 * @param array:    The array.
 * @param size:     Number of elements to generate.
 * @param min:      Minimum value accepted in the array.
 * @param max:      Maximum value accepted in the array.
 * @param num_proc: Number of MPI processes.
 * @param rank:     Rank of the process calling the function.
 */
void array_init_random(int *array, long long size, int min, int max,
                       int num_proc, int rank);

/**
 * @brief Fill the given array with integers read from a file.
 * @param array:     The array.
 * @param size:      Number of elements to read from the file.
 * @param file_path: Path to the file containing the numbers.
 * @param num_proc:  Number of MPI processes.
 * @param rank:      Rank of the process calling the function.
 */
void array_init_from_file(int *array, long long size, const char *file_path,
                          int num_proc, int rank);

/**
 * @brief Find min and max values in the array.
 * @param array: The array.
 * @param size:  Number of elements in the array.
 * @param min:   Minimum value (output).
 * @param max:   Maximum value (output).
 */
void array_min_max(const int *array, long long size, int *min, int *max);


#endif /* UTIL_H */
