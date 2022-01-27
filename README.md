# COUNTING SORT MPI

This project presents a version of [Counting Sort] Algorithm that has been
parallelized by use of the [MPI] library for C.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## Counting Sort

Counting Sort is an algorithm for sorting a collection of objects according to
their respective keys (small, positive integers).  
The assumption made on the input array is that it must be filled either with
integers in a range **[min, max]** or any other type of elements which can be
represented each with a unique key in that range.  
This algorithm does not compare the elements but rather counts their frequency
in the array to determine their new position.

Below, the pseudocode for the algorithm:

```c
1   initialize array[N]
    // find min and max in the array
2   min <- array[0]
3   max <- array[0]
4   for i <- 1 to N
5       if array[i] < min
6           min <- array[i]
7       if array[i] > max
8           max <- array[i]
9   end for
    // initialize count array with a size equal to the [min, max] range
10  range <- max - min + 1
11  initialize count[range]
    // count the number of occurrences of each element
12  for i <- 0 to N
13      count[array[i] - min] <- count[array[i] - min] + 1
14  end for
    // reposition the elements based on their number of occurrences
15  initialize z <- 0
16  for i <- min to max
17      for j <- 0 to count[i - min]
18          array[z] <- i
19          z <- z + 1
20      end for
21  end for
```

Worst-case Time Complexity: **O(n + k)**  
Where *n* is the number of elements in the array and *k* is the size of the
**[min, max]** range.

Worst-case Space Complexity: **O(n + k)**.  
If the range **[min, max]** is far greater than the number of elements, the
auxiliary array introduces a considerable amount of wasted space.

Another trait of this algorithm is its *Stability*: elements sharing the same
key will retain their relative positions.

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## Performance Evaluation

To verify and estimate the benefits of parallelization, a [shell cript] can
[be executed] to run the C program multiple times, with different combinations
of these parameters:

+ Array size
+ Compiler optimization level
+ Number of processes

The shell script calls (if not [told otherwise]) a [Python script] to
calculate *Speedup* and *Efficiency*, to create tables and draw plots to compare
the measurements' results.

### Speedup

In parallel computing, speedup is a number that measures the improvement in
speed of the execution of a task by comparing the serial and the parallel
versions of said execution.

```
Speedup = Serial_Execution_Time / Parallel_Execution_Time
```

### Efficiency

Calculating the efficiency of a parallel program is a way to relate the
*Speedup* with the number of processes needed to achieve that improvement on the
performance. The higher the efficiency, the better.

```
Efficiency = Speedup / Num_Processes
```

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## Usage

In order to be able to execute any of the following scripts, *cd* into the
project's root directory and give them executable permissions:

```shell
chmod +x ./scripts/*.sh
```

### Generate the data

To generate the data regarding the execution times evaluation and comparison,
launch the following shell script.  
Take a look at the [dependencies] required to run the script.

```shell
./scripts/measures.sh [OPTION]...
```

#### Options and Parameters for Measures

The script supports the definition of various options and/or parameters via
command line arguments.

| Argument                       | Description               |
| :---                           | :----                     |
| -h, --help                     | Show guide and quit.      |
| -n **N**, --num-measures **N** | Run every measure **N** times. (default is 50)\* |
| -d **DIR**, --dir **DIR**      | Specify output directory. (default is *./output*) |
| --no-plot                      | Do not run the Python script to create the plots and tables. |

\* *The higher the number, the more precise the mean value is.*

Other parameters, like the number of processes to use or the array size(s), can
be modified inside the script [itself].


### Compile source files

To compile source files without the help of the shell script, use the makefile.

Compile sources to generate parallel version:

```shell
make parallel
```

Or compile serial version:

```shell
make serial
```

In both cases the executable file produced is *bin/main.out*.


### Run tests

To run the test file(s) and check that the code produced works as expected,
launch the following script.

```shell
./scripts/test.sh [OPTION]...
```

#### Options and Parameters for Test

The script supports the definition of various options and/or parameters via
command line arguments.

| Argument                   | Description               |
| :---                       | :----                     |
| -h, --help                 | Show guide and quit.      |
| --silent                   | Suppress all output except failure messages. |
| -n **N**, --numproc **N**  | Run test with **N** processes. (default is 4) |


### Generate random integers

The program can initialize the array by reading integers from a binary file.
Since such file would unnecessarily weight down the repository, a shell [script]
that creates the file on demand is provided instead.  
It is automatically called by the *test.sh* script in order to test the array
initialization; to manually generate the numbers anyway, execute the following
command:

```shell
./scripts/generate_numbers.sh
```

This script creates and runs a C program that fills an array with random
numbers (speeded up with OpenMP parallelization) and writes them onto the
*data/numbers.dat* file.


### Clean

To delete object files and executables:

```shell
make clean
```

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## Dependencies

A list of requirements to run the program and the scripts:

+ Bash Shell 4.2+
+ gcc 9+
+ make
+ OpenMP 4.5+
+ OpenMPI 4.0+
+ Python 3.7+

### Python modules

The following modules are needed to handle CSV files and tables and perform
some calculations:

+ scipy
+ numpy
+ pandas
+ prettytable

To draw plots:

+ matplotlib

Use Pip to install these with the following command:

```shell
pip install *module_name*
```

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## Author

Marco Plaitano

- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

## License

Distributed under the [GPLv3] license.


<!-- LINKS -->

[Counting Sort]:
https://en.wikipedia.org/wiki/Counting_sort
"Wikipedia article"

[MPI]:
https://www.open-mpi.org/
"Main website"

[be executed]:
#generate-the-data
"Anchor to header"

[shell cript]:
scripts/measures.sh
"Repository file"

[told otherwise]:
#options-and-parameters-for-measures
"Anchor to header"

[Python script]:
scripts/evaluate.py
"Repository file"

[dependencies]:
#dependencies
"Anchor to header"

[itself]:
scripts/measures.sh
"Repository file"

[script]:
scripts/generate_numbers.sh
"Repository file"

[GPLv3]:
LICENSE
"Repository file"
