#include "main.h"

int main(int argc, char *argv[])
{
	// most things will be hardcoded for now
	MPI_Init(NULL, NULL);

	int world_rank, world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	// only one communicating face for now
	// hardcoded for now. we'll load this from a config given to us by the "server"
	FILE *inFile;
	Header h = { 0 };
	FaceBuffer face;
	int dest;
	if (world_rank == 0)
	{
		inFile = fopen("room_0.dwm", "rb");
		fread(&h, sizeof(Header), 1, inFile);
		face = allocFaceBuffer(Front, &h, 1);
	}
	else
	{
		inFile = fopen("room_1.dwm", "rb");
		fread(&h, sizeof(Header), 1, inFile);
		face = allocFaceBuffer(Back, &h, 0);
	}

	int iterationCnt = 8000;
	
	printf("%d, %d, %d @ %d\n", h.x, h.y, h.z, h.frequency);

	Node*** nodes = allocNodes(&h);
	readNodes(nodes, &h, inFile);

	fclose(inFile);

	Node** sources;
	int sourceCnt = getAllNodesOfType(&sources, &h, nodes, SRC_NODE);

	char* source = "source.pcm";

	float** sourceData = readSourceFiles(&source, sourceCnt, iterationCnt);

	Node** receivers;
	int receiverCnt = getAllNodesOfType(&receivers, &h, nodes, RCVR_NODE);

	float** receiversData = allocReceiversMemory(receiverCnt, iterationCnt);

	FaceBuffer* connectedFaces = &face;
	int connectedFacesCnt = 1;

	printf("Process %d, sources: %d, receivers: %d\n", world_rank, sourceCnt, receiverCnt);

	// DWM algorithm loop
	for (int i = 0; i < iterationCnt; i++)
	{
		// wait until all processes are here
		MPI_Barrier(MPI_COMM_WORLD);
		
		injectSamples(sources, sourceData, sourceCnt, i);
		
		scatterPass(&h, nodes);

		readSamples(receivers, receiversData, receiverCnt, i);

		//process faces
		MPI_Request req;
		for (int f = 0; f < connectedFacesCnt; f++)
		{
			FaceBuffer* cFace = &(connectedFaces[f]);
			fillFaceBuffer(nodes, &h, cFace);
			// the tag allows us to differenciate between messages
			// the facebuffer struct contains the opposing face so we will use that as the tag
			// async send
			MPI_Isend(cFace->outData, cFace->size, MPI_FLOAT, cFace->neighbour, cFace->neighbourFace, MPI_COMM_WORLD, &req);
		}
		
		// remember to look into optimizing this
		// it won't be pretty but definitely faster
		delayPass(&h, nodes);
		for (int f = 0; f < connectedFacesCnt; f++)
		{
			FaceBuffer* cFace = &(connectedFaces[f]);
			// blocking receive
			MPI_Recv(cFace->inData, cFace->x * cFace->y, MPI_FLOAT, cFace->neighbour, cFace->face, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			// and process it
			readFaceBuffer(nodes, &h, &(connectedFaces[0]));
			MPI_Wait(&req, MPI_STATUS_IGNORE);
		}
	}

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
		n[i]->pUpI += f;
		n[i]->pDownI += f;
		n[i]->pRightI += f;
		n[i]->pLeftI += f;
		n[i]->pFrontI += f;
		n[i]->pBackI += f;
	}
}

void delayPass(const Header* h, Node*** ns)
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
	int x, y, z;

	// this loop can be optimized to not have to do these ifs and only process nodes that have all neighbours but then we'd have to process all the others seperately.
	// its fine for now, but worth looking into after

	for (x = 0; x < h->x; x++)
	for (y = 0; y < h->y; y++)
	for (z = 0; z < h->z; z++)
	{
		n = &(ns[x][y][z]);

		if (x != 0) // back
		{
			ns[x - 1][y][z].pFrontI = n->pBackO;
		}
		if (x < h->x-1) // front
		{
			ns[x + 1][y][z].pBackI = n->pFrontO;
		}

		if (y != 0) // left
		{
			ns[x][y - 1][z].pRightI = n->pLeftO;
		}
		if (y < h->y-1) // right
		{
			ns[x][y + 1][z].pLeftI = n->pRightO;
		}

		if (z != 0) // down
		{
			ns[x][y][z - 1].pUpI = n->pDownO;
		}
		if (z < h->z-1) // up
		{
			ns[x][y][z + 1].pDownI = n->pUpO;
		}
	}
}

void scatterPass(const Header *h, Node ***ns) 
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
	int x, y, z;
	float k;
	for (x = 0; x < h->x; x++)
	for (y = 0; y < h->y; y++)
	for (z = 0; z < h->z; z++)
	{
		n = &(ns[x][y][z]);

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

void writeExcitation(float** buf, const int receiverCount, const int iterationCnt)
{
	FILE* file;

	for (int i = 0; i < receiverCount; i++)
	{
		// similar to printf but writes to a buffer and returns the number of characters in the resulting string (excluding '\0')
		// passing NULL and 0 as first parameters so we can get the required buffer size
		int size = snprintf(NULL, 0, "receiver_%d.pcm", i) + 1;
		
		// allocating the required memory
		char* filename = (char*)malloc(size);
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

	float** sourceData = (float**)malloc(sizeof(float*) * sourceFileCnt);
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

		sourceData[i] = (float*)calloc(sizeof(float), iterationCnt);
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