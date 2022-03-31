#include "main.h"

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	if (argc != 2)
	{
		fprintf(stderr, "Bad arguments\nInvoke should look something like:\nmpirun -n 2 ./mpi <run time>\n");
		exit(EXIT_FAILURE);
	}

	int world_rank, world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	Config cfg = readConfigFile(world_rank);

	Header h = { 0 };
	FILE* inFile = fopen(cfg.roomFileName, "rb");
	fread(&h, sizeof(Header), 1, inFile);
	Node** nodes = allocNodes(&h);
	readNodes(nodes, &h, inFile);
	fclose(inFile);

	for (int i = 0; i < cfg.faceCount; i++)
	{
		setupFaceBuffer(&(cfg.faces[i]), &h);
	}
 
	int iterationCnt = (int)ceil(atof(argv[1]) * h.frequency);

	Node** sources;
	int sourceCnt = getAllNodesOfType(&sources, &h, nodes, SRC_NODE);
	if (sourceCnt != cfg.sourceCnt)
	{
		fprintf(stderr, "Number of source files doesn't match the number of source nodes");
	}

	float** sourceData = readSourceFiles(cfg.sourceFileNames, cfg.sourceCnt, iterationCnt);

	Node** receivers;
	int receiverCnt = getAllNodesOfType(&receivers, &h, nodes, RCVR_NODE);

	float** receiversData = allocReceiversMemory(receiverCnt, iterationCnt);

	printf("Process %d beginning DWM loop. Has %d faces %d sources and %d receivers\n", world_rank, cfg.faceCount, sourceCnt, receiverCnt);
	MonitorData md = {0};
	md.pid = world_rank;
	float totalSendTime = 0.0f, totalRecvTime = 0.0f, totalDelayTime = 0.0f, totalScatterTime = 0.0f;
	struct timespec start, now;
	struct timespec tstart, tnow;
	clock_gettime(CLOCK_MONOTONIC, &tstart);

	// DWM algorithm loop
	for (int i = 0; i < iterationCnt; i++)
	{
		if(i % 250 == 0)
		{
			md.percentage = (i / (float)iterationCnt) * 100.0f;
			md.sendTime = totalSendTime / i;
			md.receptionTime = totalRecvTime / i;
			md.scatterPassTime = totalScatterTime / i;
			md.delayPassTime = totalDelayTime / i;
			monitorSend(&md);
		}
		
		// wait until all processes are here
		MPI_Barrier(MPI_COMM_WORLD);
		
		injectSamples(sources, sourceData, sourceCnt, i);
		
		clock_gettime(CLOCK_MONOTONIC, &start);
		scatterPass(&h, nodes);
		clock_gettime(CLOCK_MONOTONIC, &now);
		totalScatterTime += (now.tv_sec - start.tv_sec) + 1e-9 * (now.tv_nsec - start.tv_nsec);

		readSamples(receivers, receiversData, receiverCnt, i);

		MPI_Request req;
		for (int f = 0; f < cfg.faceCount; f++)
		{
			FaceBuffer* cFace = &(cfg.faces[f]);
			fillFaceBuffer(nodes, &h, cFace);
			MPI_Isend(cFace->outData, cFace->size, MPI_FLOAT, cFace->neighbour, cFace->neighbourFace, MPI_COMM_WORLD, &req);
		}
		clock_gettime(CLOCK_MONOTONIC, &now);
		totalSendTime += (now.tv_sec - start.tv_sec) + 1e-9 * (now.tv_nsec - start.tv_nsec);
		
		clock_gettime(CLOCK_MONOTONIC, &start);
		delayPass(&h, nodes);
		clock_gettime(CLOCK_MONOTONIC, &now);
		totalDelayTime += (now.tv_sec - start.tv_sec) + 1e-9 * (now.tv_nsec - start.tv_nsec);

		clock_gettime(CLOCK_MONOTONIC, &start);
		for (int f = 0; f < cfg.faceCount; f++)
		{
			FaceBuffer* cFace = &(cfg.faces[f]);
			MPI_Recv(cFace->inData, cFace->size, MPI_FLOAT, cFace->neighbour, cFace->face, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			readFaceBuffer(nodes, &h, cFace);
		}
		clock_gettime(CLOCK_MONOTONIC, &now);
		totalRecvTime += (now.tv_sec - start.tv_sec) + 1e-9 * (now.tv_nsec - start.tv_nsec);
	}

	clock_gettime(CLOCK_MONOTONIC, &tnow);
	printf("DWM-2D execution time: %lf\n",
            (tnow.tv_sec - tstart.tv_sec) +
            1e-9 * (tnow.tv_nsec - tstart.tv_nsec)); 

	writeExcitation(receiversData, receiverCnt, iterationCnt);

	freeNodes(&h, nodes);
	freeAllNodesOfType(&receivers);
	freeAllNodesOfType(&sources);
	freeReceiversMemory(&receiversData, receiverCnt);
	freeSourceData(&sourceData, sourceCnt);

	MPI_Finalize();

	return EXIT_SUCCESS;
}

void readSamples(Node** n, float** buf, const int receiverCount, const int iteration) 
{
	for (int i = 0; i < receiverCount; i++)
	{
		buf[i][iteration] = n[i]->p;
	}
}

void injectSamples(Node** n, float** sourceData, const int sourceCount, const int iteration)
{
	float f;
	for (int i = 0; i < sourceCount; i++)
	{
		f = sourceData[i][iteration] / 2;
		n[i]->pRightI += f;
		n[i]->pLeftI += f;
		n[i]->pFrontI += f;
		n[i]->pBackI += f;
	}

	// inject DIRAC
	// if(iteration == 0)
	// {
	// 	n[0]->pRightI += 0.5f;
	// 	n[0]->pLeftI += 0.5f;
	// 	n[0]->pFrontI += 0.5f;
	// 	n[0]->pBackI += 0.5f;
	// }

	// inject sinusoidal frequency
	// const double sinint = 44100.0 / 345.0; // the number of iterations for each sinusoidal period
	// float f = (float)cos( (double)iteration / sinint * 2 * M_PI) / 2;
	// n[0]->pRightI += f;
	// n[0]->pLeftI += f;
	// n[0]->pFrontI += f;
	// n[0]->pBackI += f;
}

void scatterPass(const Header* h, Node** ns) 
{
/*   up
 *    |z
 *    |
 *    |      y
 *   ,.------- right
 *  / 
 *x/ front
 */


	Node *n;
	int x, y;
	float k;
	for (x = 0; x < h->x; x++)
	{
		for (y = 0; y < h->y; y++)
		{
			n = &(ns[x][y]);

			if (n->type == AIR_NODE || n->type == RCVR_NODE || n->type == SRC_NODE)
			{
				n->p = (n->pRightI + n->pLeftI + n->pFrontI + n->pBackI) / 2;

				n->pRightO = n->p - n->pRightI;
				n->pLeftO = n->p - n->pLeftI;
				n->pFrontO = n->p - n->pFrontI;
				n->pBackO = n->p - n->pBackI;
			}
			else
			{
				k = getNodeReflectionCoefficient(n);

				n->pRightO = k * n->pRightI;
				n->pLeftO = k * n->pLeftI;
				n->pFrontO = k * n->pFrontI;
				n->pBackO = k * n->pBackI;
			}
		}
	}
}

void delayPass(const Header* h, Node** ns)
{
/*   up
*    |z
*    |
*    |      y
*   ,.------- right
*  /
*x/ front
*/


	Node* n;
	int x, y;
	
	// this loop can be optimized to not have to do these ifs and only process nodes that have all neighbours but then we'd have to process all the others seperately.
	// its fine for now

	for (x = 0; x < h->x; x++)
	{
		for (y = 0; y < h->y; y++)
		{
			n = &(ns[x][y]);

			if (x != 0) // back
			{
				ns[x - 1][y].pFrontI = n->pBackO;
			}
			if (x < h->x - 1) // front
			{
				ns[x + 1][y].pBackI = n->pFrontO;
			}

			if (y != 0) // left
			{
				ns[x][y - 1].pRightI = n->pLeftO;
			}
			if (y < h->y - 1) // right
			{
				ns[x][y + 1].pLeftI = n->pRightO;
			}
		}
	}
}

void writeExcitation(float** buf, const int receiverCount, const int iterationCnt)
{
	FILE* file;

	for (int i = 0; i < receiverCount; i++)
	{
		// similar to printf but writes to a buffer and returns the number of characters in the resulting string (excluding '\0')
		// passing NULL and 0 as first parameters so we can get the required buffer size
		int size = snprintf(NULL, 0, "receiver_%d.pcm", i) + 1;
		
		// allocating the required memory
		char* filename = malloc(size);
		if (filename == NULL)
		{
			fprintf(stderr, "Out of memory");
			exit(EXIT_FAILURE);
		}

		// write the string to the buffer
		snprintf(filename, size, "receiver_%d.pcm", i);

		// open file, write and close
		file = fopen(filename, "wb");
		if (!file)
		{
			perror("Error");
			exit(EXIT_FAILURE);
		}
		fwrite(buf[i], sizeof(float), iterationCnt, file);
		fclose(file);
		free(filename);
	}
}

float** readSourceFiles(char** argv, const int sourceFileCnt, const int iterationCnt)
{
	if (sourceFileCnt <= 0 || iterationCnt <= 0) return NULL;
	
	FILE* f;

	float** sourceData = malloc(sizeof(float*) * sourceFileCnt);
	if (sourceData == NULL)
	{
		fprintf(stderr, "Out of memory");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < sourceFileCnt; i++)
	{
		f = fopen(argv[i], "rb");
		if (!f)
		{
			perror("Error");
			exit(EXIT_FAILURE);
		}
		
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		sourceData[i] = calloc(sizeof(float), iterationCnt);
		if (sourceData[i] == NULL)
		{
			fprintf(stderr, "Out of memory");
			exit(EXIT_FAILURE);
		}

		if ((size / sizeof(float)) < iterationCnt)
		{
			fread(sourceData[i], sizeof(float), (size / sizeof(float)), f);
		}
		else
		{
			fread(sourceData[i], sizeof(float), iterationCnt, f);
		}

		fclose(f);
	}

	return sourceData;
}

void freeSourceData(float*** buf, int sourceCnt)
{
	if (*buf == NULL) return;
	for (int i = 0; i < sourceCnt; i++)
	{
		free((*buf)[i]);
	}
	free(*buf);
	*buf = NULL;
}