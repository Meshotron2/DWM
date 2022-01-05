#pragma once

#include"types.h"
#include<stdlib.h>
#include<stdio.h>

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

//node types
#define AIR_NODE ' '
#define SRC_NODE 'S'
#define RCVR_NODE 'R'
#define RHO_0 'A'
#define RHO_01 'B'
#define RHO_02 'C'
#define RHO_03 'D'
#define RHO_04 'E'
#define RHO_05 'F'
#define RHO_06 'G'
#define RHO_07 'H'
#define RHO_08 'I'
#define RHO_09 'J'
#define RHO_091 '1'
#define RHO_092 '2'
#define RHO_093 '3'
#define RHO_094 '4'
#define RHO_095 '5'
#define RHO_096 '6'
#define RHO_097 '7'
#define RHO_098 '8'
#define RHO_099 '9'
#define RHO_1 'Z'

// Allocates a node 3D array of Nodes ( not Node* ) with dimensions specified in the Header
// This function allocates memory that must be released by calling free_nodes()
Node*** allocNodes(const Header* header);
void freeNodes(const Header* header, Node*** nodes);
// Returns the reflection coefficient for a given node based on its type
float getNodeReflectionCoefficient(const Node* n);
// Makes buf point to a Node* array of type 
// Returns the number of nodes of type found.
// This function allocates memory that must be released by calling freeAllNodesOfType()
int getAllNodesOfType(Node*** buf, const Header* header, Node*** nodes, const char type);
// Returns an array of float arrays each holding iterationCount floats.
// This function allocates memory that must be released by calling freeReceiversMemory()
float** allocReceiversMemory(const int receiverCount, const int iterationCount);

void freeAllNodesOfType(Node*** buf);
void freeReceiversMemory(float*** buf, const int receiverCount);