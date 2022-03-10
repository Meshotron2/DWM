#include "facebuffer.h"

// https://stackoverflow.com/a/5901671
void setupFaceBuffer(FaceBuffer* f, Header* h)
{
    int size;

    switch (f->face)
    {
    case Left:
    case Right:
        size = h->x;
        break;
    case Front:
    case Back:
        size = h->y;
        break; 
    default:
        size = 0;
        break;
    }

    float *inData = (float*)malloc(size * sizeof(float));
    float *outData = (float*)malloc(size * sizeof(float));

    f->size = size;
    f->inData = inData;
    f->outData = outData;
}

void freeFaceBuffer(FaceBuffer* buf)
{
    free(buf->outData);
    free(buf->inData);
}

void fillFaceBuffer(Node** nodes, Header* h, FaceBuffer* buf)
{
    int i = 0;

    switch (buf->face)
    {
    case Left:
    {
        for (int x = 0; x < h->x; x++)
        {
            buf->outData[i++] = nodes[x][0].pLeftO;
        }
        break;
    }
    case Right:
    {
        int y = h->y - 1;
        for (int x = 0; x < h->x; x++)
        {
            buf->outData[i++] = nodes[x][y].pRightO;
        }
        break;
    }
    case Front:
    {
        int x = h->x - 1;
        for (int y = 0; y < h->y; y++)
        {
            buf->outData[i++] = nodes[x][y].pFrontO;
        }
        break;
    }
    case Back:
    {
        for (int y = 0; y < h->y; y++)
        {
            buf->outData[i++] = nodes[0][y].pBackO;
        }
        break; 
    }
    default:
        return;
    }
}

void readFaceBuffer(Node** nodes, Header* h, FaceBuffer* buf)
{
    int i = 0;

    switch (buf->face)
    {
    case Left:
    {
        for (int x = 0; x < h->x ; x++)
        {
            nodes[x][0].pLeftI = buf->inData[i++];
        }
        break;
    }
    case Right:
    {
        int y = h->y - 1;
        for (int x = 0; x < h->x ; x++)
        {
            nodes[x][y].pRightI = buf->inData[i++];
        }
        break;
    }
    case Front:
    {
        int x = h->x - 1;
        for (int y = 0; y < h->y; y++)
        {
            nodes[x][y].pFrontI = buf->inData[i++];
        }
        break;
    }
    case Back:
    {
        for (int y = 0; y < h->y; y++)
        {
            nodes[0][y].pBackI = buf->inData[i++];
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