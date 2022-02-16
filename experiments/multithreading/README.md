# Description
This is a multi-threaded simplified version of the DWM algorithm that produces no result and takes no input files.

It takes the number of nodes on each axis. The number of divisions on each axis (the total number of threads is the product of all axis division factors) and the number of iterations of the DWM algorithm.
Example:
`./multithreading xNodes yNodes zNodes xDiv yDiv zDiv nIterations`