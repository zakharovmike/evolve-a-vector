# Evolve-a-Vector

Evolve-a-Vector rapidly calculates the n-th evolution of vector _V_, evolved by matrix _A_ following this formula: _V(n+1) = A x V(n)_.  
There are numerous potential applications for Evolve-a-Vector, the most obvious being to calculate Markov chains. Where _A_ is the transition matrix and _V(n)_ is the state after _n_ transitions.

## Objectives

- Speed up a calculation by parallelizing it with multithreading
- Observe performance difference between row-wise and column-wise calculation

## Usage

To use Evolve-a-Vector, you must first define your _A_ matrix and initial _V_ vector in `main.c`.

In `main.c` towards the top, change the definition of `D` to the desired dimension for you vector and matrix.

In `main.c`, in the function `init_A_matrix` where it is labeled `// Initialize values`, change the loops as necessary to create your desired _A_ matrix. By default, it is initialized to an identity matrix _Id_ \* 0.999.

In `main.c`, in the function `init_V_vector` where it is labeled `// Initialize values`, change the loop as necessary to create your desired initial _V_ vector. By default, it is a series of integers from 1 to the dimension D (1, 2, 3, …, D-1, D).

Compile your configuration by runnning:
`gcc -lpthread -O3 -Wall -o eav main.c`

Execute your configuration by running:
`./eav method threads iterations`

where:

- **method**: 1 or 2. Method 1 calculates the next iteration row by row. Method 2 calculates the next iteration column by column. Both methods return the same result but method 2 is slower (see note 1).
- **threads**: between 1 and the number of threads available on your processor. For example, a quad-core processor will have 8 available threads while a 6-core processor will have 12 available threads. Using more threads generally makes the calculation faster (see note 2).
- **iterations**: an integer _n_, where _n_ is the number of evolutions you want to perform on vector _V_.

## Example

_A = Id \* 0.999_  
_V = (1, 2, 3, ..., 999, 1000)_  
10000 iterations performed on a 6-core Intel Core i7:  
| Number of threads | Method 1 (sec) | Method 2 (sec) |
| :---------------: | :------------: | :------------: |
| 1 | 10.147 | 20.382 |
| 2 | 5.303 | 10.012 |
| 4 | 3.251 | 5.753 |
| 6 | 2.551 | 4.741 |
| 8 | 2.216 | 4.973 |
| 10 | 2.014 | 5.590 |
| 12 | 1.924 | 5.867 |

## Notes

1. Since a matrix is represented in memory as one row after another, calculating row by row allows for sequential memory access in a given row which aids predictability and therefore speed. Calculating column by column on the other hand, means each elementary calculation in `iterate_line` requires a jump in memory (from one row to another) which slows down the memory access. While these jumps may be predictable, their existence hurts performance nonetheless.
1. The law of diminishing returns applies here too. If you have 12 available threads, using two instead of one will be faster, four faster still, while after six or eight the performance improvements are less notable.
1. With the purpose of Evolve-a-Vector being to optimize the calculation speed, it seems odd to have so many function calls (`iterate` > `iterate_lines` > `iterate_line`). This was done primarily for readability. A modern compiler is capable of optimizing in such a way that function calls are minimized. Therefore, readability of code took precedence and optimizations are left up to the compiler. (Compiling with level 3 optimization reduces calculation time threefold.)

## Potential Extensions

- Define matrix _A_ and initial vector _V_ independently of the program by reading input from a file. Likewise, make the final result easier to examine by writing it to an output file.
- Wrap Evolve-a-Vector into an API and create an easy-to-use front end for it.
