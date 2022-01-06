#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

#include "node.h"
#include "facebuffer.h"
#include "header.h"

void injectSamples(Node** n, float** sourceData, const int sourceCount, const int iteration);

void readSamples(Node** n, float** buf, const int receiverCount, const int iteration);

void scatterPass(const Header* h, Node*** ns);

void internalDelayPass(const Header* h, Node*** ns);

void writeExcitation(float** buf, const int receiverCount, const int iterationCnt);
	
void fixHeaderEndian(Header*);

float** readSourceFiles(char** argv, const int sourceFileCnt, const int iterationCnt);