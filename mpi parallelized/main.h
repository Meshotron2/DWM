#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <time.h>

#include "node.h"
#include "facebuffer.h"
#include "header.h"
#include "config.h"

/** @file */

/** 
 * Injects the values from sourceData into source nodes pressure Node::p.
 * @param[in] sources An array of Node* to the source nodes returned by getAllNodesOfType(Node***, const Header*, Node***, const char).
 * @param[in] sourceData Source nodes data returned by readSourceFiles(char**, const int, const int).
 * @param[in] sourceCount The number of source files.
 * @param[in] iteration THe current iteration.
*/
void injectSamples(Node** sources, float** sourceData, const int sourceCount, const int iteration);

/** 
 * Reads the current pressure Node::p into receiverData
 * @param[in] receivers An array of Node* to the receiver nodes returned by getAllNodesOfType(Node***, const Header*, Node***, const char).
 * @param[in] receiverData Receiver nodes data returned by allocReceiversMemory(const int, const int).
 * @param[in] receiverCount The number of receiver files.
 * @param[in] iteration THe current iteration.
*/
void readSamples(Node** receivers, float** receiverData, const int receiverCount, const int iteration);

/** 
 * Does the DWM algorithm scatter pass.
 * @param[in] h A pointer to the Header containing room dimensions.
 * @param[in] ns A 3D Node array representing the room.
*/
void scatterPass(const Header* h, Node*** ns);

/** 
 * Does the DWM algorithm delay pass.
 * @param[in] h A pointer to the Header containing room dimensions.
 * @param[in] ns A 3D Node array representing the room.
*/
void delayPass(const Header* h, Node*** ns);

/** 
 * Writes receiverData to files in a RAW audio format. One file for each receiver.
 * @param[in] receiverData The receiver data.
 * @param[in] receiverCount The number of receivers.
 * @param[in] iterationCnt The number of iterations.
*/
void writeExcitation(float** receiverData, const int receiverCount, const int iterationCnt);

/** 
 * Reads souce file data into memory. 
 * This function allocates momory that should be freed by calling freeSourceData(float***, int).
 * @param[in] argv An array of strings contaning source file names.
 * @param[in] sourceFileCnt The number of source files.
 * @param[in] iterationCnt The number of iterations.
 * @param[out] sourceData The sourceData 2D array representing [source][iteration].
*/
float** readSourceFiles(char** argv, const int sourceFileCnt, const int iterationCnt);

/** 
 * Frees memory allocated by readSourceFiles(char**, const int, const int).
 * @param[in] sourceData The source data.
 * @param[in] sourceCnt The number of sources.
*/
void freeSourceData(float*** sourceData, int sourceCnt);