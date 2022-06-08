#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "facebuffer.h"

/** @file */

/** @struct Config
 *  @brief Contains data associated with a particular configuration.
 *  @var Config::faceCount 
 *  The number of faces that we need to communicate with.
 *  @var Config::faces
 *  An array of Config::faceCount FaceBuffer structs.
 *  @var Config::sourceCnt
 *  The number of source files.
 *  @var Config::sourceFileNames
 *  An array of Config::sourceCnt strings containing the names off source files.
 *  @var Config::roomFileName
 *  The file name for the room file.
 */
typedef struct Config
{
    int faceCount;
    FaceBuffer* faces;
    int sourceCnt;
    char** sourceFileNames;
    char* roomFileName;
} Config;

/** 
 * Reads the config file for a particular rank
 * @param[in] rank The process MPI rank.
 * @param[out] config A Config struct.
*/
Config readConfigFile(int rank);

/** 
 * Frees the memory allocated memory by readConfigFile(int)
 * @param[in] cfg A pointer to a Config to free.
*/
void freeConfig(Config* cfg);

/** 
 * Removes the \\n at the end of a given string if it exists
 * @param[in] str The string (char*) to check.
*/
void removeNewLine(char* str);