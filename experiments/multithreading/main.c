#include "main.h"

// #define XDIVFACTOR 1
// #define YDIVFACTOR 1
// #define ZDIVFACTOR 1
// #define NITERATIONS 1000

int main(int argc, char** argv)
{
    int x = atoi(argv[1]);
	int y = atoi(argv[2]);
	int z = atoi(argv[3]);

	int XDIVFACTOR = atoi(argv[4]);
	int YDIVFACTOR = atoi(argv[5]);
	int ZDIVFACTOR = atoi(argv[6]);

	int NITERATIONS = atoi(argv[7]);
	
	Header h = {x, y, z, 24000};

    Node*** nodes = allocNodes(&h);

    unsigned int nThreads = XDIVFACTOR * YDIVFACTOR * ZDIVFACTOR;

    pthread_t* threads = (pthread_t*)malloc(sizeof(pthread_t) * nThreads);

    pthread_barrier_t barrier; 
    // pthread_barrierattr_t attr;

	ThreadArgs* tArgs = setupThreadArgs(&h, nodes, &barrier, XDIVFACTOR, YDIVFACTOR, ZDIVFACTOR, NITERATIONS);

    pthread_barrier_init(&barrier, NULL, nThreads);

	struct timespec start, now;
	clock_gettime(CLOCK_MONOTONIC, &start);

    for (unsigned int i = 0; i < nThreads; i++)
    {
        pthread_create(&(threads[i]), NULL, &thread, &(tArgs[i]));
    }

    for (unsigned int i = 0; i < nThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

	clock_gettime(CLOCK_MONOTONIC, &now);
    printf("Time taken: %lf\n",
            (now.tv_sec - start.tv_sec) +
            1e-9 * (now.tv_nsec - start.tv_nsec)); 

	pthread_barrier_destroy(&barrier);
    // pthread_barrierattr_destroy(&attr);

	free(tArgs);
    free(threads);
    freeNodes(&h, nodes);

    return 0;
}

ThreadArgs* setupThreadArgs(Header* h, Node*** nodes, pthread_barrier_t* barrier, int xDivFactor, int yDivFactor, int zDivFactor, int nIterations)
{
	int n = xDivFactor * yDivFactor * zDivFactor;

	ThreadArgs* tArgs = (ThreadArgs*)calloc(n, sizeof(ThreadArgs));

	int xCount = h->x / xDivFactor;
	int yCount = h->y / yDivFactor;
	int zCount = h->z / zDivFactor;
	int xRem = h->x % xDivFactor;
	int yRem = h->y % yDivFactor;
	int zRem = h->z % zDivFactor;

	int i = 0;
	for(int x = 0; x < xDivFactor; x++)
	{
		for(int y = 0; y < yDivFactor; y++)
		{
			for(int z = 0; z < zDivFactor; z++)
			{
				tArgs[i].threadId = i;
				tArgs[i].barrier = barrier;
				tArgs[i].nodes = nodes;
				tArgs[i].header = h;
				tArgs[i].nIterations = nIterations;
				tArgs[i].xi += xCount * x;
				tArgs[i].xf += xCount * (x+1) - 1;
				tArgs[i].yi += yCount * y;
				tArgs[i].yf += yCount * (y+1) - 1;
				tArgs[i].zi += zCount * z;
				tArgs[i].zf += zCount * (z+1) - 1;

				if (xRem > 0)
				{
					tArgs[i].xf++;
					for(int k = i+1; k < n; k++)
					{
						tArgs[k].xi++;
						tArgs[k].xf++;
					}
					xRem--;
				}
				if (yRem > 0)
				{
					tArgs[i].yf++;
					for(int k = i+1; k < n; k++)
					{
						tArgs[k].yi++;
						tArgs[k].yf++;
					}
					yRem--;
				}
				if (zRem > 0)
				{
					tArgs[i].zf++;
					for(int k = i+1; k < n; k++)
					{
						tArgs[k].zi++;
						tArgs[k].zf++;
					}
					zRem--;
				}

				printf("Thread %d xi %d xf %d yi %d yf %d zi %d zf %d \n", i, tArgs[i].xi, tArgs[i].xf, tArgs[i].yi, tArgs[i].yf, tArgs[i].zi, tArgs[i].zf);

				i++;
			}
		}
	}

	return tArgs;
}

void* thread(void* args)
{
    ThreadArgs* tArgs = (ThreadArgs*)args;

	struct timespec start, now;
	struct timespec start2, now2;

	printf("Hello from thread %d\n", tArgs->threadId);
    for (int i = 0; i < tArgs->nIterations; i++)
    {	
		// clock_gettime(CLOCK_MONOTONIC, &start2);
		pthread_barrier_wait(tArgs->barrier);
		// clock_gettime(CLOCK_MONOTONIC, &now2);
		// printf("Wait time: %lf\n",
        //     (now2.tv_sec - start2.tv_sec) +
        //     1e-9 * (now2.tv_nsec - start2.tv_nsec)); 

		// if (i % 1000 == 0)
		// {
		// 	printf("Thread %d in iteration %d\n", tArgs->threadId, i);
		// }

		// clock_gettime(CLOCK_MONOTONIC, &start);

		scatterPass(tArgs);
        
        delayPass(tArgs);  

		// clock_gettime(CLOCK_MONOTONIC, &now);
		// printf("Iteration time: %lf\n",
        //     (now.tv_sec - start.tv_sec) +
        //     1e-9 * (now.tv_nsec - start.tv_nsec)); 
    }

	return NULL;
}

void delayPass(ThreadArgs* tArgs)
{
	Node* n;
	int x, y, z;

	for (x = tArgs->xi; x <= tArgs->xf; x++)
	{
		for (y = tArgs->yi; y <= tArgs->yf; y++)
		{
			for (z = tArgs->zi; z <= tArgs->zf; z++)
			{
				n = &(tArgs->nodes[x][y][z]);

				if (x != 0)
				{
					tArgs->nodes[x - 1][y][z].pFrontI = n->pBackO;
				}
				if (x < tArgs->header->x - 1)
				{
					tArgs->nodes[x + 1][y][z].pBackI = n->pFrontO;
				}

				if (y != 0)
				{
					tArgs->nodes[x][y - 1][z].pRightI = n->pLeftO;
				}
				if (y < tArgs->header->y - 1)
				{
					tArgs->nodes[x][y + 1][z].pLeftI = n->pRightO;
				}

				if (z != 0)
				{
					tArgs->nodes[x][y][z - 1].pUpI = n->pDownO;
				}
				if (z < tArgs->header->z - 1)
				{
					tArgs->nodes[x][y][z + 1].pDownI = n->pUpO;
				}
			}
		}
	}
}

void scatterPass(ThreadArgs* tArgs) 
{
	Node *n;
	int x, y, z;
	float k;

	for (x = tArgs->xi; x <= tArgs->xf; x++)
		for (y = tArgs->yi; y <= tArgs->yf; y++)
			for (z = tArgs->zi; z <= tArgs->zf; z++)
			{
				n = &(tArgs->nodes[x][y][z]);

				if (n->type == AIR_NODE || n->type == RCVR_NODE || n->type == SRC_NODE)
				{
					n->p = (n->pUpI + n->pDownI + n->pRightI + n->pLeftI + n->pFrontI + n->pBackI) / 3;
					
					n->pUpO = n->p - n->pUpI;
					n->pDownO = n->p - n->pDownI;
					n->pRightO = n->p - n->pRightI;
					n->pLeftO = n->p - n->pLeftI;
					n->pFrontO = n->p - n->pFrontI;
					n->pBackO = n->p - n->pBackI;
				} 
				else 
				{
					k = getNodeReflectionCoefficient(n);
					
					n->pUpO = k * n->pUpI;
					n->pDownO = k * n->pDownI;
					n->pRightO = k * n->pRightI;
					n->pLeftO = k * n->pLeftI;
					n->pFrontO = k * n->pFrontI;
					n->pBackO = k * n->pBackI;
				}
			}
}
