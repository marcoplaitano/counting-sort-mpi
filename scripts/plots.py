"""
File:   plots.py
Brief:  This script contains the functions needed to plot a table.
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

import matplotlib.pyplot as pyplot



def plot_table(directory: str, fields: list, rows: list) -> None:
    """Create a plot for the Speedup and one for the Efficiency and write them
    onto two different files."""
    plot_speedup(directory, fields, rows)
    plot_efficiency(directory, fields, rows)



def plot_speedup(directory: str, fields: list, rows: list) -> None:
    """Create an image file plotting Speedup per Number of Processes."""
    x, y = get_axes_data(fields, rows, "Speedup")

    pyplot.figure(figsize=(7, 5))
    pyplot.plot(x, x, color="blue", marker="x", label="Speedup Ideal")
    pyplot.plot(x, y, color="red", marker="o", label="Speedup Experimental")

    # Write precise Y value next to every point
    for i in range(1, len(x)):
        x_pos = x[i] - 0.25
        y_pos = y[i] + 0.4
        # Don't write over the ideal line or the string won't be readable.
        if abs(x_pos - y_pos) < 0.6:
            y_pos -= 1 if y_pos > 1 else 0.8
        pyplot.text(x_pos, y_pos, "%.3f" %y[i])

    # Plot configuration
    pyplot.grid(b=True, which='major', color='#bbbbbb', linestyle='-')
    pyplot.autoscale(enable=True, axis='x', tight=True)
    pyplot.autoscale(enable=True, axis='y', tight=True)
    pyplot.legend()
    pyplot.xlabel("Number of Processes")
    pyplot.ylabel("Speedup")

    pyplot.savefig(directory + "/plot_speedup.jpg")
    pyplot.close()



def plot_efficiency(directory: str, fields: list, rows: list) -> None:
    """Create an image file plotting Efficiency per Number of Processes."""
    x, y = get_axes_data(fields, rows, "Efficiency")
    y[0] = 1

    pyplot.figure(figsize=(7, 5))
    pyplot.plot(x, y, color="green", marker="s", label="Efficiency")

    # Write precise Y value next to every point
    for i in range(1, len(x)):
        pyplot.text(x[i] - 0.15, y[i] + 0.05, "%.3f" %y[i])

    # Plot configuration
    pyplot.grid(b=True, which='major', color='#bbbbbb', linestyle='-')
    pyplot.autoscale(enable=True, axis='x', tight=True)
    pyplot.autoscale(enable=True, axis='y', tight=True)
    pyplot.legend()
    pyplot.xlabel("Number of Processes")
    pyplot.ylabel("Efficiency")

    pyplot.savefig(directory + "/plot_efficiency.jpg")
    pyplot.close()




def get_axes_data(fields: list, rows: list, y_data: str) -> tuple:
    """Collect data for the X and Y axes of the plot."""
    x = [0]
    y = [0]

    for row in rows:
        # Only consider Parallelized measures.
        if row[fields.index("Type")] == "Parallel":
            # The X axis will display the number of processes.
            x.append(row[fields.index("Processes")])
            # The Y axis will display the needed data (either "Speedup" or
            # "Efficiency").
            y.append(row[fields.index(y_data)])
    return x, y
