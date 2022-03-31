#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<time.h>
#include<mpi.h>

#include"header.h"
#include"node.h"
#include"config.h"
#include"facebuffer.h"
#include"monitor.h"

void injectSamples(Node** n, float** sourceData, const int sourceCount, const int iteration);

void readSamples(Node** n, float** buf, const int receiverCount, const int iteration);

void scatterPass(const Header* h, Node** ns);

void delayPass(const Header* h, Node** ns);

void writeExcitation(float** buf, const int receiverCount, const int iterationCnt);
	
void fixHeaderEndian(Header*);

float** readSourceFiles(char** argv, const int sourceFileCnt, const int iterationCnt);

void freeSourceData(float*** buf, int sourceCnt);