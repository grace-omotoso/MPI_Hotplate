# MPI_Hotplate
A program that solves the hotplate problem in a parallel fashion using MPI.

The hotplate problem requires computing a solution for the steady
state heat equation in a 2D rectangular region.  

A common way to initialize the interior points of the grid is to set them
all to the average of all the values along the edges.  As the computation
progresses, the interior points are altered as described below.
However, the values along the edges never change.

The region is covered with a grid of M by N nodes, and an M by N array
named W is used to record the temperature.  The correspondence between
array indices and locations in the region is suggested by giving the
indices of the four corners:

The steady state solution to the discrete heat equation satisfies the
following condition at an interior grid point:
  W[Current] = (1/4) * ( W[North] + W[South] + W[East] + W[West] )

where "Current" is the index of the current grid point currently being
considered, "North" is the index of its immediate neighbor to the "north",
and so on.

Given an approximate solution of the steady state heat equation, a
"better" solution is given by replacing each interior point by the
average of its 4 neighbors - in other words, by using the condition
as an ASSIGNMENT statement:

        W[Current] = (1/4) * ( W[North] + W[South] + W[East] + W[West] )

As discussed above, the values along the edges never change.

If this process is repeated often enough, the difference between
successive estimates of the solution will approach zero.  The user will
specify a value typically called epsilon which is the error tolerance
used to halt the iteration.  The program will halt when successive iterations
yield no changes larger than epsilon at any grid point.

### Inputs
* number of rows in the grid
* number of columns in the grid
* top grid temperature (constant)
* left grid temperature (constant)
* right grid temperature (constant)
* bottom grid temperature (constant)
* epsilon

### Processing
If the process discussed above is repeated often enough, the difference between
successive estimates of the solution will approach zero.  The user will
specify a value typically called epsilon which is the error tolerance
used to halt the iteration.  The program will halt when successive iterations
yield no changes larger than epsilon at any grid point.


### Output
To avoid large output, we will display only iterations that has the power of 2 where the the maximum difference is greater than epsilon and the last iteration before a value smaller than epsilon is attained.  A sample is shown below;
Given
* num_rows = 500
* num_cols = 500
* top_temp = 100
* left_temp = 200
* right_temp = 300
* bottom_temp = 400
* epsilon = 0.01

![output](output.png)

