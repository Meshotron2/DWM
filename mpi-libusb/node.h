#pragma once

#include"header.h"
#include<stdlib.h>
#include<stdio.h>

/** @file */

/** @struct Node
 *  @brief Contains data associated with a particular Node.
 *  @var Node::pUpI
 *  The up input buffer.
 *  @var Node::pUpO
 * 	The up output buffer.
 *  @var Node::pDownI
 *  The down input buffer.
 *  @var Node::pDownO
 *  The down output buffer.
 *  @var Node::pLeftI
 *  The left input buffer.
 *  @var Node::pLeftO
 *  The left output buffer.
 *  @var Node::pRightI
 *  The right input buffer.
 *  @var Node::pRightO
 *  The right output buffer.
 *  @var Node::pFrontI
 *  The front input buffer.
 *  @var Node::pFrontO
 *  The front output buffer.
 *  @var Node::pBackI
 *  The back input buffer.
 *  @var Node::pBackO
 *  The back output buffer.
 *  @var Node::p
 *  The pressure.
 *  @var Node::type
 *  The type
 */
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

/** @brief Air Node type. */
#define AIR_NODE ' '
/** @brief Source Node type. */
#define SRC_NODE 'S'
/** @brief Receiver Node type. */
#define RCVR_NODE 'R'
/** @brief 0 ρ Node type (fully absortvent). */
#define RHO_0 'A'
/** @brief 0.1 ρ Node type. */
#define RHO_01 'B'
/** @brief 0.2 ρ Node type. */
#define RHO_02 'C'
/** @brief 0.3 ρ Node type. */
#define RHO_03 'D'
/** @brief 0.4 ρ Node type. */
#define RHO_04 'E'
/** @brief 0.5 ρ Node type. */
#define RHO_05 'F'
/** @brief 0.6 ρ Node type. */
#define RHO_06 'G'
/** @brief 0.7 ρ Node type. */
#define RHO_07 'H'
/** @brief 0.8 ρ Node type. */
#define RHO_08 'I'
/** @brief 0.9 ρ Node type. */
#define RHO_09 'J'
/** @brief 0.91 ρ Node type. */
#define RHO_091 '1'
/** @brief 0.92 ρ Node type. */
#define RHO_092 '2'
/** @brief 0.93 ρ Node type. */
#define RHO_093 '3'
/** @brief 0.94 ρ Node type. */
#define RHO_094 '4'
/** @brief 0.95 ρ Node type. */
#define RHO_095 '5'
/** @brief 0.96 ρ Node type. */
#define RHO_096 '6'
/** @brief 0.97 ρ Node type. */
#define RHO_097 '7'
/** @brief 0.98 ρ Node type. */
#define RHO_098 '8'
/** @brief 0.99 ρ Node type. */
#define RHO_099 '9'
/** @brief 1 ρ Node type (fully reflective). */
#define RHO_1 'Z'

/** 
 * Allocates a 3D Node (not Node*) array with dimensions specified in the Header
 * This function allocates memory that should be released by calling freeNodes(const Header*, Node***)
 * @param[in] header A pointer to the Header containing room dimensions. 
 * @param[out] nodes A 3D Node array representing the room.
*/
Node*** allocNodes(const Header* header);

/** 
 * Frees a 3D Node array allocated by allocNodes(const Header*).
 * @param[in] header A pointer to the Header containing room dimensions. 
 * @param[in] nodes A 3D Node array representing the room.
*/
void freeNodes(const Header* header, Node*** nodes);

/** 
 * Reads the nodes from a file into a 3D Node array.
 * @param[in] nodes A 3D Node array representing the room.
 * @param[in] h A pointer to the Header containing room dimensions. 
 * @param[in] inFile A valid FILE pointer to a .dwm file.
*/
void readNodes(Node ***nodes, Header *h, FILE *inFile);

/** 
 * Returns the reflection coefficient for a given node based on its type.
 * @param[in] n A pointer to a Node.
 * @param[out] f The reflection coefficient.
*/
float getNodeReflectionCoefficient(const Node* n);

/** 
 * Looks for all nodes of a given type.
 * Returns the number of nodes of type found. If return value is greater then 0 buf will point to a valid Node** array.
 * This function allocates memory that must be released by calling freeAllNodesOfType(Node***).
 * @param[in] buf A tripe Node pointer.
 * @param[in] header A pointer to the Header containing room dimensions. 
 * @param[in] nodes A 3D Node array representing the room.
 * @param[in] type A char representing the type to look for.
 * @param[out] nodeCount The number of Nodes of type found.
*/
int getAllNodesOfType(Node*** buf, const Header* header, Node*** nodes, const char type);

/** 
 * Allocates memory for all receivers file in a 2D float array.
 * This function allocates memory that must be released by calling freeReceiversMemory(float***, const int)
 * @param[in] receiverCount The number of receivers to allocate memory for.
 * @param[in] iterationCount The number of iterations the model will process.
 * @param[out] receiversData A 2D float array representing [receiver][iteration].
*/
float** allocReceiversMemory(const int receiverCount, const int iterationCount);

/** 
 * Frees memory allocated by getAllNodesOfType(Node***, const Header*, Node***, const char).
 * @param[in] buf A pointer to the 2D array.
*/
void freeAllNodesOfType(Node*** buf);

/** 
 * Frees memory allocated by allocReceiversMemory(const int, const int).
 * @param[in] buf A pointer to the 2D array.
 * @param[in] receiverCount The number of receivers.
*/
void freeReceiversMemory(float*** buf, const int receiverCount);