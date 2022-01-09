#include "main.h"

#define ITERATIONS 2

// https://curc.readthedocs.io/en/latest/programming/MPI-C.html

int main()
{
    MPI_Init(NULL, NULL);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    Header h = { 10, 10, 10, 44000 };

    Node*** nodes = alloc_nodes(&h);

    // just 2 processes for now
    uint32_t destination;
    char* logFile;
    Faces face;
    if (world_rank == 0)
    {
        destination = 1;
        logFile = "0.txt";
        face = Front;
    }
    else
    {
        destination = 0;
        logFile = "1.txt";
        face = Back;
    }

    FaceBuffer f = allocFaceBuffer(face, &h, destination);
    FILE *fout = fopen(logFile, "wb");

    int cnt = 0;
    for (int i = 0; i < h.x; i++)
    {
        for (int j = 0; j < h.x; j++)
        {
            for (int k = 0; k < h.x; k++)
            {
                Node n = {0};
                n.pUpO = cnt * (world_rank+1);
                n.pDownO = cnt * (world_rank+1);
                n.pRightO = cnt * (world_rank+1);
                n.pLeftO = cnt * (world_rank+1);
                n.pFrontO = cnt * (world_rank+1);
                n.pBackO = cnt * (world_rank+1);
                nodes[i][j][k] = n;
                fprintf(fout, "%f ", n.pFrontO);
                cnt++;
            }
            fprintf(fout, "\n");
        }
        fprintf(fout, "\n------------------\n");
    }

    printf("Process: %d beginning main loop\n", world_rank);

    struct timespec start, now;

    for (uint32_t i = 0; i < ITERATIONS; i++)
    {
        MPI_Barrier(MPI_COMM_WORLD);
        printf("Process: %d beginning iteration: %d\n", world_rank, i);
        clock_gettime(CLOCK_MONOTONIC, &start);

        fillFaceBuffer(nodes, &h, &f);

        MPI_Request request;
        MPI_Isend(&(f.out[0][0]), f.x * f.y, MPI_FLOAT, destination, f.neighbourFace, MPI_COMM_WORLD, &request);

        MPI_Recv(&(f.in[0][0]), f.x * f.y, MPI_FLOAT, destination, f.face, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        readFaceBuffer(nodes, &h, &f);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

        fprintf(fout, "Sent:\n");
        for (uint32_t x = 0; x < f.x; x++)
        {
            for (uint32_t y = 0; y < f.y; y++)
            {
                fprintf(fout, "%f ", f.out[x][y]);
            }
            fprintf(fout, "\n");
        }
        fprintf(fout, "Got:\n");
        if (world_rank == 0)
        {
            for (int i = (h.x - 1); i <= (h.x - 1); i++)
            {
                for (int j = 0; j < h.x; j++)
                {
                    for (int k = 0; k < h.x; k++)
                    {
                        fprintf(fout, "%f ", nodes[i][j][k].pFrontI);
                    }
                    fprintf(fout, "\n");
                }
                fprintf(fout, "\n------------------\n");
            }
        }
        else
        {
            for (int i = 0; i <= 0; i++)
            {
                for (int j = 0; j < h.x; j++)
                {
                    for (int k = 0; k < h.x; k++)
                    {
                       fprintf(fout, "%f ", nodes[i][j][k].pBackI);
                    }
                    fprintf(fout, "\n");
                }
                fprintf(fout, "\n------------------\n");
            }
        }
        // for (uint32_t x = 0; x < f.x; x++)
        // {
        //     for (uint32_t y = 0; y < f.y; y++)
        //     {
        //         fprintf(fout, "%f ", f.in[x][y]);
        //     }
        //     fprintf(fout, "\n");
        // }

        // usleep(rand() % 1000000);
        // if (world_rank == 0)
        // {
        //     usleep(rand() % 1000000);
        // }

        // MPI_Barrier(MPI_COMM_WORLD);

        clock_gettime(CLOCK_MONOTONIC, &now);
        printf("Time taken: %lf\n",
               (now.tv_sec - start.tv_sec) +
               1e-9 * (now.tv_nsec - start.tv_nsec));
    }

    free_nodes(&h, nodes);
    MPI_Finalize();
}