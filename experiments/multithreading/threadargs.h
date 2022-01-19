#pragma once
#include "pthread.h"
#include "node.h"

typedef struct ThreadArgs 
{
    int threadId;
    pthread_barrier_t* barrier;
    Node*** nodes;
    Header* header;
    int nIterations;
    int xi;
    int xf;
    int yi;
    int yf;
    int zi;
    int zf;
} ThreadArgs;