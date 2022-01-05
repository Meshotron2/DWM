#pragma once
#include <stdint-gcc.h>

typedef struct Node
{
    char c;
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
    uint32_t x;
    uint32_t y;
    float** in;
    float** out;
} FaceBuffer;

typedef struct Header 
{
	int x;
	int y;
	int z;
	int frequency;
} Header;