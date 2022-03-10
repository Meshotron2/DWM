#pragma once

#include <stdint-gcc.h>
#include "header.h"
#include "node.h"

typedef enum Faces{Left, Right, Front, Back} Faces;

typedef struct FaceBuffer
{
    Faces face;
    int neighbour;
    Faces neighbourFace;
    int size;
    float* inData;
    float* outData;
} FaceBuffer;

void setupFaceBuffer(FaceBuffer* f, Header* h);
void freeFaceBuffer(FaceBuffer* buf);
void fillFaceBuffer(Node** nodes, Header* h, FaceBuffer* buf);
void readFaceBuffer(Node** nodes, Header* h, FaceBuffer* buf);
Faces getOpposingFace(Faces f);