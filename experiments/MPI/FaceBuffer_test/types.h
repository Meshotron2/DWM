#pragma once
#include <stdint-gcc.h>

typedef struct Node {
	float pUpI;
	float pUpO;
	float pDownI;
	float pDownO;
	float pLeftI;
	float pLeftO;
	float pRightI;
	float pRightO;
	float pFrontI;
	float pFrontO;
	float pBackI;
	float pBackO;

	float p;
	char type;
} Node;

// typedef struct Buffer
// {
//     int in;
//     int out;
// } Buffer;

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

typedef struct Header 
{
	int x;
	int y;
	int z;
	int frequency;
} Header;