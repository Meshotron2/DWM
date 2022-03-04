#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "facebuffer.h"

typedef struct Config
{
    int faceCount;
    FaceBuffer* faces;
    int sourceCnt;
    char** sourceFileNames;
    char* roomFileName;
} Config;


Config readConfigFile(int rank);
void freeConfig(Config* cfg);
void removeNewLine(char* str);