/**
 * @file main.c
 * @brief Main file of the source program used to generate and sort an array
 *        using a parallelized version of the Counting Sort algorithm.
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

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "counting_sort.h"
#include "util.h"


int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int num_proc;
    MPI_Comm_size(MPI_COMM_WORLD, &num_proc);

    /* Check for the correct amount of command line arguments. */
    if (argc != 2) {
        if (rank == 0)
            fprintf(stderr, "ERROR! usage: bin/parallel.out array_size\n");
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    /* Check for the correctness of the range. */
    if (RANGE_MAX <= RANGE_MIN) {
        if (rank == 0)
            fprintf(stderr, "ERROR! can't have RANGE_MAX <= RANGE_MIN.\n");
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    /* Create the array with size given as command line argument. */
    const long long size = atoll(argv[1]);
    int *array = (int *)safe_alloc(size * sizeof(int));

    /* To store execution time measurements. */
    double time_init = 0, time_sort = 0, time_elapsed = 0;

    /*
     * Initialize the array by filling it with integers, either generated
     * randomly or taken from a file.
     */
    START_TIME(time_init);
    array_init_random(array, size, RANGE_MIN, RANGE_MAX, num_proc, rank);
    // array_init_from_file(array, size, INPUT_FILE_PATH, num_proc, rank);
    END_TIME(time_init);

    /* Sort the array. */
    START_TIME(time_sort);
    counting_sort(array, size, num_proc, rank);
    END_TIME(time_sort);

    MPI_Finalize();
    free(array);

    if (rank == 0) {
        /* Only consider the initialization and sorting times. */
        time_elapsed = time_init + time_sort;
        fprintf(stdout, "%lld;%d;%.5f;%.5f;%.5f;", size, num_proc, time_init,
                time_sort, time_elapsed);
    }

    return EXIT_SUCCESS;
}
