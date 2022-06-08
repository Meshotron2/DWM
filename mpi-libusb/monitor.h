#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 49152
#define SA struct sockaddr

typedef struct MonitorData
{
    int32_t pid;
    float percentage;
    float sendTime;
    float receptionTime; 
    float delayPassTime; 
    float scatterPassTime;
} MonitorData;

void monitorSend(MonitorData* monitorData);