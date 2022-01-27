/**
 * @file test.c
 * @brief This file contains the functions needed to test the correct execution
 *        of the parallelized version of the program.
 * @author Marco Plaitano
 * @date 30 Dic 2021
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

#include <mpi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "counting_sort.h"
#include "util.h"

/** Number of array sizes the program is tested with. */
#define NUM_SIZES 5


/**
 * @brief Check that all the elements in the array are in the range [min; max].
 * @param array: The array.
 * @param size:  Number of elements in the array.
 * @param min:   Mininum value accepted.
 * @param max:   Maximum value accepted.
 * @return `true` if all elements are in given range; `false` otherwise.
 */
bool elements_in_range(int *array, long long size, int min, int max);

/**
 * @brief Test the correct inizialization of the array with random numbers.
 * @param array:    The array.
 * @param size:     Size of the array to inizialize.
 * @param num_proc: Number of MPI processes.
 * @param rank:     Rank of the process calling the function.
 */
void test_init_random(int *array, long long size, int num_proc, int rank);

/**
 * @brief Test the correct inizialization of the array by reading from file.
 * @param array:     The array.
 * @param size:      Size of the array to inizialize.
 * @param file_path: Path to the file containing the numbers.
 * @param num_proc:  Number of MPI processes.
 * @param rank:      Rank of the process calling the function.
 */
void test_init_from_file(int *array, long long size, const char *file_path,
                         int num_proc, int rank);

/**
 * @brief Test the correctness of the sorting algorithm.
 * @param array:    The array to sort.
 * @param size:     Size of the array.
 * @param num_proc: Number of MPI processes.
 * @param rank:     Rank of the process calling the function.
 */
void test_sort(int *array, long long size, int num_proc, int rank);



int main(int argc, char **argv) {
    int rank, num_proc;
    /*
     * Some of these sizes are prime numbers and therefore surely not evenly
     * divisible by any number of processes.
     */
    long long sizes[NUM_SIZES] = {10, 6053, 30000, 500009, 20000000};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    for (int i = 0; i < NUM_SIZES; i++) {
        if (rank == 0) {
            fprintf(stdout, "Testing size %lld (%d/%d) with %d processes...\n",
                    sizes[i], i + 1, NUM_SIZES, num_proc);
            fflush(stdout);
        }

        int *array = (int *)safe_alloc(sizes[i] * sizeof(int));
        if (argc == 2)
            test_init_from_file(array, sizes[i], argv[1], num_proc, rank);
        test_init_random(array, sizes[i], num_proc, rank);
        test_sort(array, sizes[i], num_proc, rank);

        free(array);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}



bool elements_in_range(int *array, long long size, int min, int max) {
    for (long long i = 0; i < size; i++)
        if (array[i] < min || array[i] > max)
            return false;
    return true;
}


void test_init_random(int *array, long long size, int num_proc, int rank) {
    array_init_random(array, size, RANGE_MIN, RANGE_MAX, num_proc, rank);

    /* Check that all elements are constrained in the range [min; max]. */
    if (!elements_in_range(array, size, RANGE_MIN, RANGE_MAX)) {
        if (rank == 0)
            fprintf(stderr, "FAILED Initialization Random!\n"
                            "The array elements are not in the range "
                            "[%d, %d]\n", RANGE_MIN, RANGE_MAX);
        free(array);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if (rank == 0)
        fprintf(stdout, "OK Initialization Random.\n");
}


void test_init_from_file(int *array, long long size, const char *file_path,
                         int num_proc, int rank)
{
    array_init_from_file(array, size, file_path, num_proc, rank);

    /* Check that all elements are constrained in the range [min; max]. */
    if (!elements_in_range(array, size, RANGE_MIN, RANGE_MAX)) {
        if (rank == 0)
            fprintf(stderr, "FAILED Initialization From File!\n"
                            "The array elements are not in the range "
                            "[%d, %d]\n", RANGE_MIN, RANGE_MAX);
        free(array);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    if (rank == 0)
        fprintf(stdout, "OK Initialization From File.\n");
}


void test_sort(int *array, long long size, int num_proc, int rank) {
    counting_sort(array, size, num_proc, rank);
    MPI_Bcast(array, size, MPI_INT, 0, MPI_COMM_WORLD);

    /* Check that no element has lesser value than its predecessor. */
    for (long long i = size - 1; i > 0; i--)
        if (array[i] < array[i - 1]) {
            if (rank == 0)
                fprintf(stderr, "FAILED Sorting!\n"
                                "array[%lld] %d > %d array[%lld]\n",
                                i - 1, array[i - 1], array[i], i);
            free(array);
            MPI_Barrier(MPI_COMM_WORLD);
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
    if (rank == 0)
        fprintf(stdout, "OK Sorting.\n");
}
