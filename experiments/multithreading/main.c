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
	pthread_barrier_init(&barrier, NULL, nThreads);

	ThreadArgs* tArgs = setupThreadArgs(&h, nodes, &barrier, XDIVFACTOR, YDIVFACTOR, ZDIVFACTOR, NITERATIONS);

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

// to-do: get rid of the 3d array 
ThreadArgs* setupThreadArgs(Header* h, Node*** nodes, pthread_barrier_t* barrier, int xDivFactor, int yDivFactor, int zDivFactor, int nIterations)
{
	int n = xDivFactor * yDivFactor * zDivFactor;

	int xCount = h->x / xDivFactor;
	int yCount = h->y / yDivFactor;
	int zCount = h->z / zDivFactor;

	ThreadArgs*** tArgs = (ThreadArgs***)malloc(xCount * sizeof(ThreadArgs**));

	for (int i = 0; i < xCount; i++)
	{
		tArgs[i] = (ThreadArgs**)malloc(yCount * sizeof(ThreadArgs*));

		for (int j = 0; j < yCount; j++)
		{
			tArgs[i][j] = (ThreadArgs*)calloc(zCount, sizeof(ThreadArgs));
		}
	}
	
	int xRem = h->x % xDivFactor;
	int i = 0;
	for(int x = 0; x < xDivFactor; x++)
	{
		int yRem = h->y % yDivFactor;
		
		for(int y = 0; y < yDivFactor; y++)
		{
			int zRem = h->z % zDivFactor;
			
			for(int z = 0; z < zDivFactor; z++)
			{
				tArgs[x][y][z].threadId = i;
				tArgs[x][y][z].barrier = barrier;
				tArgs[x][y][z].nodes = nodes;
				tArgs[x][y][z].header = h;
				tArgs[x][y][z].nIterations = nIterations;
				tArgs[x][y][z].xi += xCount * x;
				tArgs[x][y][z].xf += xCount * (x+1) - 1;
				tArgs[x][y][z].yi += yCount * y;
				tArgs[x][y][z].yf += yCount * (y+1) - 1;
				tArgs[x][y][z].zi += zCount * z;
				tArgs[x][y][z].zf += zCount * (z+1) - 1;

				if (xRem > 0)
				{
					for (int m = 0; m < yCount; m++)
					{
						for (int n = 0; n < zCount; n++)
						{
							tArgs[x][m][n].xf++;
						}
					}
					for(int k = x+1; k < xCount; k++)
					{
						for (int m = 0; m < yCount; m++)
						{
							for (int n = 0; n < zCount; n++)
							{
								tArgs[k][m][n].xi++;
								tArgs[k][m][n].xf++;
							}
						}
					}
					xRem--;
				}
				if (yRem > 0)
				{
					for (int m = 0; m < zCount; m++)
					{
						tArgs[x][y][m].yf++;
					}
					for(int k = y+1; k < yCount; k++)
					{
						for (int m = 0; m < yCount; m++)
						{
							tArgs[x][k][m].yi++;
							tArgs[x][k][m].yf++;
						}
					}
					yRem--;
				}
				if (zRem > 0)
				{
					tArgs[x][y][z].zf++;
					for(int k = z+1; k < zCount; k++)
					{
						tArgs[x][y][k].zi++;
						tArgs[x][y][k].zf++;
					}
					zRem--;
				}

				i++;
			}
		}
	}

	ThreadArgs* t = (ThreadArgs*)malloc(n * sizeof(ThreadArgs));

	i = 0;
	for(int x = 0; x < xDivFactor; x++)
	{
		for(int y = 0; y < yDivFactor; y++)
		{
			for(int z = 0; z < zDivFactor; z++)
			{
				t[i] = tArgs[x][y][z];
				
				printf("Thread %d xi %d xf %d yi %d yf %d zi %d zf %d \n", i, tArgs[x][y][z].xi, tArgs[x][y][z].xf, tArgs[x][y][z].yi, tArgs[x][y][z].yf, tArgs[x][y][z].zi, tArgs[x][y][z].zf);
				i++;
			}
		}
	}

	for (int i = 0; i < xDivFactor; i++)
	{
		for (int j = 0; j < yDivFactor; j++)
		{
			free(tArgs[i][j]);
		}
		free(tArgs[i]);
	}
	free(tArgs);	

	return t;
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
