#include "stdlib.h"
#include "stdio.h"
#include "stdint-gcc.h"
#include "mpi.h"

#define BufferSize 10000

int main()
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    int* buffer = (int*)malloc(sizeof(int) * BufferSize);
    if (world_rank == 0)
    {
        for (uint32_t i = 0; i < BufferSize; i++)
        {
            buffer[i] = i * 2;
        }
        
        printf("Process %d sending data\n", world_rank);
        // If the message is of significant size MPI_Send will only return once the message has been received by the destination
        // https://stackoverflow.com/a/7181900
        MPI_Send(buffer, BufferSize, MPI_INT, 1, 0, MPI_COMM_WORLD);
        printf("Process %d finished sending data\n", world_rank);
    }
    else
    {
        printf("Process %d waiting\n", world_rank);
        MPI_Recv(buffer, BufferSize, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Process %d received: \n", world_rank);
        for (uint32_t i = 0; i < BufferSize; i++)
        {
            printf("%05d ", buffer[i]);
            if (i % 25 == 0) printf("\n");
        }
        printf("\n");
    }

    MPI_Finalize();
}