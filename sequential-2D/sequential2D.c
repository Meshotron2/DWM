#include "sequential2D.h"

int main(int argc, char *argv[])
{
	if (argc < 4)
	{
		fprintf(stderr, "Missing argument <file> <runtime>\n");
		return EXIT_FAILURE;
	}

	FILE *inFile = fopen(argv[1], "rb");
	if (!inFile) 
	{
		perror("Error");
		return EXIT_FAILURE;
	}

	Header h = { 0 };
	fread(&h, sizeof(Header), 1, inFile);

	if (h.z != 1)
	{
		fprintf(stderr, "Invalid room header! Z must be 1.");
		exit(EXIT_FAILURE);
	}

	int iterationCnt = (int)ceil(atof(argv[2]) * h.frequency);
	if (!iterationCnt)
	{
		fprintf(stderr, "Failed to parse <runtime>. Exiting.");
		exit(EXIT_FAILURE);
	}
	
	printf("%d, %d @ %d\n", h.x, h.y, h.frequency);

	Node** nodes = allocNodes(&h);
	readNodes(nodes, &h, inFile);

	fclose(inFile);

	Node** sources;
	int sourceCnt = getAllNodesOfType(&sources, &h, nodes, SRC_NODE);

	if (sourceCnt == 0)
	{
		fprintf(stderr, "No sources found. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	if (argc - 3 != sourceCnt)
	{
		fprintf(stderr, "Not enough source files");
		exit(EXIT_FAILURE);
	}

	float** sourceData = readSourceFiles(&argv[3], sourceCnt, iterationCnt);
	// for DIRAC ignore source files
	// float** sourceData = NULL;

	Node** receivers;
	int receiverCnt = getAllNodesOfType(&receivers, &h, nodes, RCVR_NODE);

	if (receiverCnt == 0)
	{
		fprintf(stderr, "No receivers found. Exiting.\n");
		exit(EXIT_FAILURE);
	}

	float** receiversData = allocReceiversMemory(receiverCnt, iterationCnt);

	// DWM algorithm loop
	struct timespec start, now;
	clock_gettime(CLOCK_MONOTONIC, &start);
	for (int i = 0; i < iterationCnt; i++)
	{
		injectSamples(sources, sourceData, sourceCnt, i);
		
		scatterPass(&h, nodes);

		readSamples(receivers, receiversData, receiverCnt, i);

		delayPass(&h, nodes);
	}
	clock_gettime(CLOCK_MONOTONIC, &now);
    printf("DWM-2D execution time: %lf\n",
            (now.tv_sec - start.tv_sec) +
            1e-9 * (now.tv_nsec - start.tv_nsec)); 

	writeExcitation(receiversData, receiverCnt, iterationCnt);

	freeNodes(&h, nodes);
	freeAllNodesOfType(&receivers);
	freeAllNodesOfType(&sources);
	freeReceiversMemory(&receiversData, receiverCnt);
	freeSourceData(&sourceData, sourceCnt);

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
	if (sourceFileCnt <= 0 || iterationCnt <= 0) exit(EXIT_FAILURE);
	
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