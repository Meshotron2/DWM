#include "facebuffer.h"

// https://stackoverflow.com/a/5901671
void setupFaceBuffer(FaceBuffer* f, Header* h)
{
    uint32_t size;

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
    uint32_t i = 0;

    switch (buf->face)
    {
    case Left:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y <= 0 ; y++)
            {
                buf->outData[i++] = nodes[x][y].pLeftO;
            }
        }
        break;
    case Right:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = h->y-1; y <= h->y-1 ; y++)
            {
                buf->outData[i++] = nodes[x][y].pRightO;
            }
        }
        break;
    case Front:
        for (uint32_t x = (h->x-1); x <= (h->x-1) ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                buf->outData[i++] = nodes[x][y].pFrontO;
            }
        }
        break;
    case Back:
        for (uint32_t x = 0; x <= 0 ; x++)
        {
            for (uint32_t y = 0; y < h->y ; y++)
            {
                buf->outData[i++] = nodes[x][y].pBackO;
            }
        }
        break; 
    default:
        return;
    }
}

void readFaceBuffer(Node** nodes, Header* h, FaceBuffer* buf)
{
    uint32_t i = 0;

    switch (buf->face)
    {
    case Left:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = 0; y <= 0 ; y++)
            {
                nodes[x][y].pLeftI = buf->inData[i++];
            }
        }
        break;
    case Right:
        for (uint32_t x = 0; x < h->x ; x++)
        {
            for (uint32_t y = h->y-1; y <= h->y-1 ; y++)
            {
                nodes[x][y].pRightI = buf->inData[i++];
            }
        }
        break;
    case Front:
        for (uint32_t x = (h->x - 1); x <= (h->x - 1); x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                nodes[x][y].pFrontI = buf->inData[i++];
            }
        }
        break;
    case Back:
        for (uint32_t x = 0; x <= 0; x++)
        {
            for (uint32_t y = 0; y < h->y; y++)
            {
                nodes[x][y].pBackI = buf->inData[i++];
            }
        }
        break; 
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