#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <mpi.h>

#include "facebuffer.h"

#define MAX_REQUESTS 12

static pthread_t communicatorThread;

void CommunicatorInit(int* rank, int* size, int* ac, char*** av);
void CommunicatorSetData(FaceBuffer* f, int count);
void CommunicatorDestroy();
void Send();
void Wait();