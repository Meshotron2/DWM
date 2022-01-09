if [ $# -eq 1 ]
then
    mpic++ main2.c -o main2
    mpirun -np 2 main2
else
    mpic++ main.c -o main
    mpirun -np 2 main
fi