"""
File:   evaluate.py
Brief:  This script reads the measured performances data and creates CSV tables
        and plots to visualize the Speedup and Efficiency of each measure.
Author: Marco Plaitano
Date:   27 Nov 2021

COUNTING SORT MPI
Parallelize and Evaluate Performances of "Counting Sort" Algorithm, by using
MPI.

Copyright (C) 2022 Plaitano Marco

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
"""

from os import listdir, path, scandir, walk
from sys import argv
from decimal import Decimal
from pandas import read_csv
from scipy.stats import norm
from prettytable import PrettyTable
from plots import plot_table



def my_round(num: float) -> float:
    """Round up the given float number to its first 5 decimal points."""
    return float(round(Decimal(num), 5))



def calculate_means(files: list) -> dict:
    """Calculate, for every file, the mean of every parameter."""
    # Dictionary in which keys are filenames and values are other dictionaries
    # containing: parameter names as keys and their means as values.
    all_means = {}

    # Parameters to calculate the mean of.
    columns = ["time_init", "time_sort", "sys", "real"]

    for file in files:
        file_mean = {}
        content = read_csv(file, sep=';')

        for col in columns:
            curr_data = content[col]
            # Mean and Standard Deviation
            # .norm generates a Normal Continuos Distribution
            # .fit generates the MLE (Maximum Likelihood Estimation) for the
            # given data by minimizing the negative log-likelihood function.
            # The return values are location and scale parameters. For a normal
            # distribution the location is its mean.
            mean, std = norm.fit(curr_data)
            # Remove values that are too unlikely; in other words, only keep
            # values inside the range  [mean - std, mean + std]
            curr_data = content[(content[col] > (mean - std)) &
                                (content[col] < (mean + std))][col]
            if len(curr_data) == 0:
                mean = my_round(0.0)
            else:
                mean = norm.fit(curr_data)[0]
            file_mean[col] = my_round(mean)
        # Since elapsed time is the sum of init and sort times, it feels more
        # natural to keep this relationship instead of calculating a mean for
        # this parameter.
        file_mean["time_elapsed"] = my_round(file_mean["time_init"] +
                                             file_mean["time_sort"])

        all_means[file] = file_mean

    return all_means



def parse_file_name(filename: str) -> tuple:
    """Get measure info (is_serial, size, n_processes, opt_lvl) from file name.

    An example of a filename is: Sxxxx_Pyy_Oz.csv, where:
     - xxxx  is the Size
     - yy    is the number of Processes
     - z     is the Optimization level used during compilation."""
    filename = path.basename(filename)

    # Extract size.
    size_start = filename.index('S') + 1
    size_end = filename.index('_', size_start)
    size = int(filename[size_start : size_end])

    # Extract number of processes.
    np_start = filename.index('P') + 1
    np_end = filename.index('_', np_start)
    num_procs = int(filename[np_start : np_end])

    # Extract optimization level.
    ol_start = filename.index('O') + 1
    ol_end = filename.index('.', ol_start)
    opt_lvl = int(filename[ol_start : ol_end])

    is_serial = True if num_procs == 0 else False

    return is_serial, size, num_procs, opt_lvl



def create_row(file: str, is_serial: bool, size: int, num_procs: int,
               opt_lvl: int, means: dict) -> list:
    """Create a list containing all the information taken from the arguments."""
    row = []

    if is_serial:
        measure_type = "Default" if opt_lvl == 0 else "Serial"
    else:
        measure_type = "Parallel"

    row.append(measure_type)
    row.append(size)
    row.append(num_procs)
    row.append(opt_lvl)

    # All the means of the measure's parameters.
    for col in means[file]:
        row.append(means[file][col])

    # Speedup and Efficiency.
    if is_serial:
        row.append(1) # speedup
        row.append(1) # efficiency
    # If the measure is parallelized these two parameters will be calculated
    # directly in the make_table() function, taking into account the execution
    # time of the corresponding serial measure.

    return row



def make_table(root_dir: str, files: list, means: dict) -> None:
    """Create a table storing, for every type of test, the mean of the results.

    Each table allows a comparison between every parallel measure, the
    correspondent serial version (with same optimization), and the default case:
    serial program compiled with -O0 flag.
    A new table will be created when either the problem's SIZE or the LEVEL OF
    OPTIMIZATION changes.
    """
    fields = ["Type", "Size", "Processes", "Opt Lvl", "Init", "Sort", "Elapsed",
              "Sys", "Real", "Speedup", "Efficiency"]
    rows = []

    # All subdirectories in the output's root directory.
    subdirs = sorted([d.path for d in scandir(root_dir) if d.is_dir()])

    for subdir in subdirs:
        # If current directory has optimization level 0 it surely contains only
        # 1 file with the measures made serially, and with -O0 compilation flag.
        # This is the current "default case".
        if subdir.endswith("opt_0"):
            # Get filename and parse the details.
            file = listdir(subdir)[0]
            is_serial, size, num_procs, opt_lvl = parse_file_name(file)
            first_row = create_row(subdir + "/" + file, is_serial, size,
                                   num_procs, opt_lvl, means)
            continue

        # Every table's first row will contain the info about the default case
        # of the problem.
        rows.append(first_row)

        # These are all the files in the current subdirectory. They all have
        # measures made with optimization -Ox (x = 1, 2, 3).
        curr_files = sorted([f for f in files if f.find(subdir + "/") >= 0])

        # For every file in the directory create a new row to insert in the
        # current table.
        for file in curr_files:
            is_serial, size, num_procs, opt_lvl = parse_file_name(file)

            # Add all details in a list.
            row = create_row(file, is_serial, size, num_procs, opt_lvl, means)

            # Because the list of files is sorted, the first one will always be
            # serial; thus, the time_serial variable will always be available
            # when calculating speedup and efficiency of a parallelized measure.
            if is_serial:
                time_serial = float(means[file]['time_elapsed'])
                # Speedup and Efficiency for a serial execution are both 1 and
                # have therefore already been added to the row in create_row().
            else:
                time_parallel = float(means[file]['time_elapsed'])
                # Speedup and Efficiency
                speedup = my_round(time_serial / time_parallel)
                efficiency = my_round(speedup / num_procs)
                row.append(speedup)
                row.append(efficiency)

            rows.append(row)

        # When all the rows have been built, write the table onto a file.
        write_table(fields, rows, subdir)

        # Create two images plotting Speedup and Efficiency over Number of
        # Processes.
        plot_table(subdir, fields, rows)

        rows.clear()



def write_table(fields: list, rows: list, directory: str) -> None:
    """Write a table with fields and rows onto the given file."""
    table = PrettyTable()
    table.field_names = fields
    table.add_rows(rows)
    # Write table in standard CSV format, separator is ','.
    with open(directory + "/table.csv", "w", encoding="UTF-8") as file:
        file.write(table.get_csv_string())



def main() -> None:
    """Main function."""

    if len(argv) < 2:
        print("usage: python3 ./evaluate.py dir")
        print("'dir' is the directory containing the output produced by "
              "measures.sh")
        exit(1)
    directory = argv[1]

    # Get all files contained in the given directory
    files = []
    for root, dirs, files_found in walk(directory):
        for file in files_found:
            if file.endswith(".csv") and file.find("table") == -1:
                files.append(path.join(root, file))
    files = sorted(files)

    if len(files) == 0:
        print("No output files found. Exiting the script.")
        exit(1)

    print("Calculating means...")
    means = calculate_means(files)

    print("Creating tables and plots...")
    make_table(directory, files, means)



if __name__ == "__main__":
    main()
