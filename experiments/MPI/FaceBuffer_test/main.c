#include "main.h"

// https://curc.readthedocs.io/en/latest/programming/MPI-C.html

int main()
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    Header h = { 100, 100, 100, 44000 };

    Node*** nodes = alloc_nodes(&h);

    // just 2 processes for now
    if (world_rank == 0)
    {
        FaceBuffer f = allocFaceBuffer(Top, &h);
        FILE* fout = fopen("0.txt", "wb");

        fillFaceBuffer(nodes, &h, &f, fout);

        // https://stackoverflow.com/a/66194073
        // https://stackoverflow.com/a/7181900
        MPI_Request request;
        MPI_Isend(&(f.out[0][0]), f.x*f.y, MPI_FLOAT, 1, 0, MPI_COMM_WORLD, &request);

        MPI_Recv(&(f.in[0][0]), f.x*f.y, MPI_FLOAT, 1, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

        fprintf(fout, "\n");
        for (uint32_t x = 0; x < f.x; x++)
        {
            for (uint32_t y = 0; y < f.y; y++)
            {
                fprintf(fout, "%f ", f.in[x][y]);
            }
            fprintf(fout, "\n");
        }
    }
    else
    {
        FaceBuffer f = allocFaceBuffer(Bottom, &h);
        FILE* fout = fopen("1.txt", "wb");

        fillFaceBuffer(nodes, &h, &f, fout);

        MPI_Request request;
        MPI_Isend(&(f.out[0][0]), f.x*f.y, MPI_FLOAT, 0, 1, MPI_COMM_WORLD, &request);

        MPI_Recv(&(f.in[0][0]), f.x*f.y, MPI_FLOAT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

        fprintf(fout, "\n");
        for (uint32_t x = 0; x < f.x; x++)
        {
            for (uint32_t y = 0; y < f.y; y++)
            {
                fprintf(fout, "%f ", f.in[x][y]);
            }
            fprintf(fout, "\n");
        }
    }

    free_nodes(&h, nodes);

    MPI_Finalize();
}