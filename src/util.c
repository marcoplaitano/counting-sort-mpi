/**
 * @file util.c
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

#include "util.h"

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


void *safe_alloc(long long size) {
    if (size < 1) {
        fprintf(stderr, "Can not allocate memory of %lld bytes.\n", size);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Could not allocate memory of %lld bytes.\n", size);
        MPI_Barrier(MPI_COMM_WORLD);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }
    return ptr;
}


void array_init_random(int *array, long long size, int min, int max,
                       int num_proc, int rank)
{
    /* Every process will have a different seed. */
    unsigned seed = time(NULL) ^ rank;

    /* Divide the total size evenly among every process. */
    const long long local_size = size / num_proc;
    /*
     * The size might not always be perfectly divisible by the number of
     * processes; in such cases, a portion of the array might be left out. This
     * is the position in the array where the left out elements start.
     */
    long long index_leftout = local_size * num_proc;

    /* Each process will fill a local array with local_size elements. */
    int *local_array = (int *)safe_alloc(local_size * sizeof(int));
    for (long long i = 0; i < local_size; i++)
        local_array[i] = rand_r(&seed) % (max + 1 - min) + min;

    /* All the local_array are then merged into the one global input array. */
    MPI_Allgather(local_array, local_size, MPI_INT, array, local_size, MPI_INT,
                  MPI_COMM_WORLD);
    free(local_array);

    /*
     * Initialize all the remaining elements.
     * This section is not parallelized because the number of remaining elements
     * is, at most, equal to NUM_PROC-1. Using MPI for such a small number would
     * certainly slow everything down.
     */
    if (index_leftout < size) {
        /* Every process will have the same seed. */
        srand(max - min + num_proc);
        for (long long i = index_leftout; i < size; i++)
            array[i] = rand() % (max + 1 - min) + min;
    }
}


void array_init_from_file(int *array, long long size, const char *file_path,
                          int num_proc, int rank)
{
    MPI_File file;

    /* Divide the total size evenly among every process. */
    const long long local_size = size / num_proc;
    /*
     * The size might not always be perfectly divisible by the number of
     * processes; in such cases, a portion of the array might be left out. This
     * is the position in the array where the left out elements start.
     */
    long long index_leftout = local_size * num_proc;

    /* Each process will fill a local array with local_size elements. */
    int *local_array = (int *)safe_alloc(local_size * sizeof(int));

    MPI_File_open(MPI_COMM_WORLD, file_path, MPI_MODE_RDONLY, MPI_INFO_NULL,
                  &file);
    /*
     * Process with rank N reads local_size elements starting at position
     * N * local_size.
     */
    MPI_File_seek(file, local_size * (rank * sizeof(int)), MPI_SEEK_SET);
    MPI_File_read(file, local_array, local_size, MPI_INT, MPI_STATUS_IGNORE);

    /* All the local_array are then merged into the one global input array. */
    MPI_Allgather(local_array, local_size, MPI_INT, array, local_size, MPI_INT,
                  MPI_COMM_WORLD);
    free(local_array);

    /*
     * Read all the remaining elements.
     * This section is not parallelized because the number of remaining elements
     * is, at most, equal to NUM_PROC-1. Using MPI for such a small number would
     * certainly slow everything down.
     */
    if (index_leftout < size) {
	    MPI_File_seek(file, index_leftout * sizeof(int), MPI_SEEK_SET);
        MPI_File_read(file, array + index_leftout, size - index_leftout,
                      MPI_INT, MPI_STATUS_IGNORE);
    }

    MPI_File_close(&file);
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
