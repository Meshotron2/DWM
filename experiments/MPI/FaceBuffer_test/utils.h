#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include"types.h"

Node*** alloc_nodes(const Header* header)
{
	//https://web.archive.org/web/20210601072857/https://www.techiedelight.com/dynamically-allocate-memory-for-3d-array/
	//main website was down when i tried
	Node*** nodes = NULL;

	if (header->x <= 0 || header->y <= 0 || header->z <= 0)
	{
		fprintf(stderr, "Invalid header! x,y and z cannot be <= 0!");
		exit(EXIT_FAILURE);
	}

	nodes = (Node***)malloc(header->x * sizeof(Node**));

	if (nodes == NULL)
	{
		fprintf(stderr, "Out of memory");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < header->x; i++)
	{
		nodes[i] = (Node**)malloc(header->y * sizeof(Node*));

		if (nodes[i] == NULL)
		{
			fprintf(stderr, "Out of memory");
			exit(EXIT_FAILURE);
		}

		for (int j = 0; j < header->y; j++)
		{
			nodes[i][j] = (Node*)malloc(header->z * sizeof(Node));
			if (nodes[i][j] == NULL)
			{
				fprintf(stderr, "Out of memory");
				exit(EXIT_FAILURE);
			}
		}
	}

	Node n = { ' ' };

	for (int i = 0; i < header->x; i++)
	{
		for (int j = 0; j < header->y; j++)
		{
			for (int k = 0; k < header->z; k++)
			{
				nodes[i][j][k] = n;
			}
		}
	}

	return nodes;
}

void free_nodes(const Header* header, Node*** nodes)
{
	if (header->x <= 0 || header->y <= 0 || header->z <= 0)
	{
		fprintf(stderr, "Invalid header! x,y and z cannot be <= 0!");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < header->x; i++)
	{
		for (int j = 0; j < header->y; j++)
		{
			free(nodes[i][j]);
		}
		free(nodes[i]);
	}
	free(nodes);
}

Faces getOpposingFace(Faces f)
{
    switch (f)
    {
    case Top:
        return Bottom;
    case Bottom:
        return Top;
    case Left:
        return Right;
    case Right:
        return Left;
    case Front:
        return Back;
    case Back:
        return Front;
    }
}

// https://stackoverflow.com/a/5901671
FaceBuffer allocFaceBuffer(Faces f, Header* h, int neighbour)
{
    uint32_t size, x, y;

    switch (f)
    {
    case Top:
    case Bottom:
        size = h->x * h->y;
        x = h->x;
        y = h->y;
        break;
    case Left:
    case Right:
        size = h->x * h->z;
        x = h->x;
        y = h->z;
        break;
    case Front:
    case Back:
        size = h->y * h->z;
        x = h->y;
        y = h->z;
        break; 
    default:
        size = 0;
        x = 0;
        y = 0;
        break;
    }

    float *inData = (float*)malloc(x * y * sizeof(float));
    float **inArray = (float**)malloc(x * sizeof(float*));
    for (uint32_t i = 0; i < x; i++)
    {
        inArray[i] = &(inData[y*i]);
    }

    float *outData = (float*)malloc(x * y * sizeof(float));
    float **outArray = (float**)malloc(x * sizeof(float*));
    for (uint32_t i = 0; i < x; i++)
    {
        outArray[i] = &(outData[y*i]);
    }

    FaceBuffer buf = { f, neighbour, getOpposingFace(f), x, y, x * y, inData, inArray, outData, outArray };
    return buf;
}

void fillFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf)
{
    uint32_t i = 0;

    switch (buf->face)
    {
    case Top:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = h->z-1; z <= h->z-1 ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pUpO;
                }
            }
        }
        break;
    case Bottom:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = 0; z <= 0 ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pDownO;
                }
            }
        }
        break;
    case Left:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y <= 0 ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pLeftO;
                }
            }
        }
        break;
    case Right:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = h->y-1; y <= h->y-1 ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pRightO;
                }
            }
        }
        break;
    case Front:
        for (uint32_t x = h->x-1; x <= h->x-1 ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pFrontO;
                }
            }
        }
        break;
    case Back:
        for (uint32_t x = 0; x <= 0 ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    buf->outData[i++] = nodes[x][y][z].pBackO;
                }
            }
        }
        break; 
    default:
        return;
    }
}

void readFaceBuffer(Node ***nodes, Header *h, FaceBuffer *buf)
{
    uint32_t i = 0;

    switch (buf->face)
    {
    case Top:
        for (uint32_t x = 0; x < h->x; x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                for (uint32_t z = h->z - 1; z <= h->z - 1; z++)
                {
                    nodes[x][y][z].pUpI = buf->inData[i++];
                }
            }
        }
        break;
    case Bottom:
        for (uint32_t x = 0; x < h->x; x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                for (uint32_t z = 0; z <= 0; z++)
                {
                    nodes[x][y][z].pDownI = buf->inData[i++];
                }
            }
        }
        break;
    case Left:
        for (uint32_t x = 0; x < h->x; x++)
        {
            for (uint32_t y = 0; y <= 0; y++)
            {
                for (uint32_t z = 0; z < h->z; z++)
                {
                    nodes[x][y][z].pLeftI = buf->inData[i++];
                }
            }
        }
        break;
    case Right:
        for (uint32_t x = 0; x < h->x; x++)
        {
            for (uint32_t y = h->y - 1; y <= h->y - 1; y++)
            {
                for (uint32_t z = 0; z < h->z; z++)
                {
                    nodes[x][y][z].pRightI = buf->inData[i++];
                }
            }
        }
        break;
    case Front:
    {
        for (uint32_t x = (h->x - 1); x <= (h->x - 1); x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                for (uint32_t z = 0; z < h->z; z++)
                {
                    nodes[x][y][z].pFrontI = buf->inData[i++];
                }
            }
        }
    }
    break;
    case Back:
    {
        for (uint32_t x = 0; x <= 0; x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                for (uint32_t z = 0; z < h->z; z++)
                {
                    // printf("%f \n", buf->inData[i++]);
                    nodes[x][y][z].pBackI = buf->inData[i++];
                    //printf("%d %d %d %f \n", x,y,z,nodes[x][y][z].pBackI);
                }
            }
        }
    }
    break;
    default:
        return;
    }
}