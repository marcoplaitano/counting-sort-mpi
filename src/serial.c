/**
 * @file serial.c
 * @brief Main program performing a serial version of the Counting Sort
 *        algorithm.
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

#ifdef _SERIAL
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

/** @brief Minimum integer value accepted in the array. */
#define RANGE_MIN 0

/** @brief Maximum integer value accepted in the array. */
#define RANGE_MAX 255

/**
 * @brief Start measuring the passing of time.
 * @param var: Name of the `double` variable in which to save measurements.
 *
 * The measurement is taken on Wall-Clock time by calling the `gettimeofday()`
 * function.
 */
#define START_TIME(var) \
    struct timeval begin_##var, end_##var; \
    gettimeofday(&begin_##var, 0);

/**
 * @brief Stop measuring the passing of time and save the result in `var`.
 * @param var: Name of the `double` variable in which to save measurements.
 *
 * The measurement is taken on Wall-Clock time by calling the `gettimeofday()`
 * function. The function is supposed to have already been called before, via
 * the START_TIME macro, on the same variable.
 * `var` will contain the difference (in seconds and microseconds) of the values
 * gotten by the two calls to `gettimeofday()`.
 */
#define END_TIME(var) \
    gettimeofday(&end_##var, 0); \
    long seconds_##var = end_##var.tv_sec - begin_##var.tv_sec; \
    long microseconds_##var = end_##var.tv_usec - begin_##var.tv_usec; \
    var = seconds_##var + microseconds_##var * 1e-6;



/**
 * @brief Sort the given array using Counting Sort.
 * @param array: The array.
 * @param size:  Number of elements in the array.
 */
void counting_sort(int *array, long long size);

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
 */
void array_init_random(int *array, long long size, int min, int max);

/**
 * @brief Find min and max values in the array.
 * @param array: The array.
 * @param size:  Number of elements in the array.
 * @param min:   Minimum value (output).
 * @param max:   Maximum value (output).
 */
void array_min_max(const int *array, long long size, int *min, int *max);





int main(int argc, char **argv) {
    /* Check for the correct amount of command line arguments. */
    if (argc != 2) {
        fprintf(stderr, "ERROR! usage: bin/serial.out array_size\n");
        exit(EXIT_FAILURE);
    }

    /* Check for the correctness of the parameters. */
    if (RANGE_MAX <= RANGE_MIN) {
        fprintf(stderr, "ERROR! can't have RANGE_MAX <= RANGE_MIN.\n");
        exit(EXIT_FAILURE);
    }

    /* Create the array with size given as command line argument. */
    const long long size = atoll(argv[1]);
    int *array = (int *)safe_alloc(size * sizeof(int));

    /* To store execution time measurements. */
    double time_init = 0, time_sort = 0, time_elapsed = 0;

    START_TIME(time_init);
    array_init_random(array, size, RANGE_MIN, RANGE_MAX);
    END_TIME(time_init);

    START_TIME(time_sort);
    counting_sort(array, size);
    END_TIME(time_sort);

    free(array);

    time_elapsed = time_init + time_sort;
    fprintf(stdout, "%lld;0;%.5f;%.5f;%.5f;", size, time_init, time_sort,
            time_elapsed);
}




void counting_sort(int *array, long long size) {
    long long i = 0, j = 0, k = 0;
    int max = 0, min = 0;
    array_min_max(array, size, &min, &max);
    const int count_size = max - min + 1;

    int *count = (int *)safe_alloc(sizeof(int) * count_size);
    for (i = 0; i < count_size; i++)
        count[i] = 0;

    for (i = 0; i < size; i++)
        count[array[i] - min] += 1;

    for (i = min; i < max + 1; i++)
        for (j = 0; j < count[i - min]; j++)
            array[k++] = i;

    free(count);
}


void *safe_alloc(long long size) {
    if (size < 1) {
        fprintf(stderr, "Can not allocate memory of %lld bytes.\n", size);
        exit(EXIT_FAILURE);
    }

    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Could not allocate memory of %lld bytes.\n", size);
        exit(EXIT_FAILURE);
    }
    return ptr;
}


void array_init_random(int *array, long long size, int min, int max) {
    unsigned seed = time(NULL);
    for (long long i = 0; i < size; i++)
        array[i] = rand_r(&seed) % (max + 1 - min) + min;
}


void array_min_max(const int *array, long long size, int *min, int *max) {
    *min = array[0];
    *max = array[0];

    for (long long i = 0; i < size; i++)
        if (array[i] < *min)
            *min = array[i];
        else if (array[i] > *max)
            *max = array[i];
}


#endif /* _SERIAL */
