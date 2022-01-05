#pragma once

#include<stdio.h>
#include<stdlib.h>
#include<math.h>

#include"types.h"
#include"node.h"
void readNodes(Node ***node, Header *h, FILE *inFile); 

void injectSamples(Node** n, float** sourceData, const int sourceCount, const int iteration);

void readSamples(const Node** n, float** buf, const int receiverCount, const int iteration);

void scatterPass(const Header* h, Node*** ns);

void delayPass(const Header* h, Node*** ns);

void writeExcitation(float** buf, const int receiverCount, const int iterationCnt);
	
void fixHeaderEndian(Header*);

float** readSourceFiles(char** argv, const int sourceFileCnt, const int iterationCnt);