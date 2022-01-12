#pragma once
#include <stdint-gcc.h>
#include "header.h"
#include "node.h"

typedef enum Faces{Top, Bottom, Left, Right, Front, Back} Faces;

typedef struct FaceBuffer
{
    Faces face;
    int neighbour;
    Faces neighbourFace;
    uint32_t x;
    uint32_t y;
    uint32_t size;
    float* inData;
    float** in;
    float* outData;
    float** out;
} FaceBuffer;

FaceBuffer allocFaceBuffer(Faces f, Header* h, int neighbour);
void freeFaceBuffer(FaceBuffer* buf);
void fillFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf);
void readFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf);
Faces getOpposingFace(Faces f);