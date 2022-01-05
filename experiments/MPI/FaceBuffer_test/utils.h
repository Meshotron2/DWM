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

// https://stackoverflow.com/a/5901671
FaceBuffer allocFaceBuffer(Faces f, Header* h)
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

    FaceBuffer buf = { f, x, y, inArray, outArray };
    return buf;
}

void fillFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf, FILE* debug)
{
    uint32_t bX = 0, bY = 0;
    uint32_t order = 0;
    
    //printf("buf->x %d, buf->y %d\n", buf->x, buf->y);

    switch (buf->face)
    {
    case Top:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = h->z; z <= h->z ; z++)
                {
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
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
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
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
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
                }
            }
        }
        break;
    case Right:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = h->y; y <= h->y ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
                }
            }
        }
        break;
    case Front:
        for (uint32_t x = h->x; x <= h->x ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                for (uint32_t z = 0; z < h->z ; z++)
                {
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
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
                    //fprintf(debug, "bX: %d, bY: %d, x: %d, y: %d, z:%d\n", bX, bY, x,y,z);
                    buf->out[bX][bY++] = (float)order++;
                    if (bY == buf->y) 
                    {
                        bX++;
                        bY = 0;
                    }
                }
            }
        }
        break; 
    default:
        return;
    }
}