#include "node.h"
#include "threadargs.h"
#include "pthread.h"

void* thread(void* args);
void delayPass(ThreadArgs* tArgs);
void scatterPass(ThreadArgs* tArgs);
ThreadArgs* setupThreadArgs(Header* h, Node*** nodes, pthread_barrier_t* barrier, int xDivFactor, int yDivFactor, int zDivFactor, int nIterations);