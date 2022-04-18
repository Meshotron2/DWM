#include "facebuffer.h"

// https://stackoverflow.com/a/5901671
void setupFaceBuffer(FaceBuffer* f, Header* h)
{
    int size;

    switch (f->face)
    {
    case Top:
    case Bottom:
        size = h->x * h->y;
        break;
    case Left:
    case Right:
        size = h->x * h->z;
        break;
    case Front:
    case Back:
        size = h->y * h->z;
        break; 
    default:
        size = 0;
        break;
    }

    f->size = size;
}

void fillFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf)
{
    int i = 0;

    switch (buf->face)
    {
    case Top:
    {
        int z = h->z - 1;
        for (int x = 0; x < h->x ; x++)
        {
            for (int y = 0; y < h->y ; y++)
            {
                buf->outData[i++] = nodes[x][y][z].pUpO;
            }
        }
        break;
    }
    case Bottom:
    {
        for (int x = 0; x < h->x ; x++)
        {
            for (int y = 0; y < h->y ; y++)
            {
                buf->outData[i++] = nodes[x][y][0].pDownO;
            }
        }
        break;
    }
    case Left:
    {
        for (int x = 0; x < h->x ; x++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                buf->outData[i++] = nodes[x][0][z].pLeftO;
            }
        }
        break;
    }
    case Right:
    {
        int y = h->y - 1;
        for (int x = 0; x < h->x ; x++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                buf->outData[i++] = nodes[x][y][z].pRightO;
            }
        }
        break;
    }
    case Front:
    {
        int x = h->x - 1;
        for (int y = 0; y < h->y ; y++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                buf->outData[i++] = nodes[x][y][z].pFrontO;
            }
        }
        break;
    }
    case Back:
    {
        for (int y = 0; y < h->y ; y++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                buf->outData[i++] = nodes[0][y][z].pBackO;
            }
        }
        break; 
    }
    default:
        return;
    }
}

void readFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf)
{
    int i = 0;

    switch (buf->face)
    {
    case Top:
    {
        int z = h->z - 1;
        for (int x = 0; x < h->x ; x++)
        {
            for (int y = 0; y < h->y ; y++)
            {
                nodes[x][y][z].pUpI = buf->inData[i++];
            }
        }
        break;
    }
    case Bottom:
    {
        for (int x = 0; x < h->x ; x++)
        {
            for (int y = 0; y < h->y ; y++)
            {
                nodes[x][y][0].pDownI = buf->inData[i++];
            }
        }
        break;
    }
    case Left:
    {
        for (int x = 0; x < h->x ; x++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                nodes[x][0][z].pLeftI = buf->inData[i++];
            }
        }
        break;
    }
    case Right:
    {
        int y = h->y - 1;
        for (int x = 0; x < h->x ; x++)
        {
            for (int z = 0; z < h->z ; z++)
            {
                nodes[x][y][z].pRightI = buf->inData[i++];
            }
        }
        break;
    }
    case Front:
    {
        int x = h->x - 1;
        for (int y = 0; y < h->y; y++)
        {
            for (int z = 0; z < h->z; z++)
            {
                nodes[x][y][z].pFrontI = buf->inData[i++];
            }
        }
        break;
    }
    case Back:
    {
        for (int y = 0; y < h->y; y++)
        {
            for (int z = 0; z < h->z; z++)
            {
                nodes[0][y][z].pBackI = buf->inData[i++];
            }
        }
        break; 
    }
    default:
        return;
    }
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