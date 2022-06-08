#pragma once
#include <stdint-gcc.h>
#include <libusb-1.0/libusb.h>
#include "header.h"
#include "node.h"

/** @file */

/** @enum FaceBuffer
 *  @brief Contains data associated with a face.
 */
typedef enum Faces
{
    Top,    /**< The top face */
    Bottom, /**< The bottom face */
    Left,   /**< The left face */
    Right,  /**< The right face */
    Front,  /**< The front face */
    Back    /**< The back face */
} Faces;

/** @struct FaceBuffer
 *  @brief Contains data associated with a face.
 *  @var FaceBuffer::face
 *  A Faces enum representing the face.
 *  @var FaceBuffer::size
 *  The total size in floats of the face.
 *  @var FaceBuffer::device
 *  The libusb_device_handle associated with this FaceBuffer
 *  @var FaceBuffer::devPort
 *  The port where FaceBuffer::device is connected
 *  @var FaceBuffer::devInterface
 *  The interface where FaceBuffer::device is connected
 *  @var FaceBuffer::in_transfer
 *  The libusb_transfer associated with input to this FaceBuffer
 *  @var FaceBuffer::out_transfer
 *  The libusb_transfer associated with output from this FaceBuffer
 *  @var FaceBuffer::inData
 *  The input buffer for this face where the neighbour data will be read to.
 *  @var FaceBuffer::outData
 *  The output buffer for this face where the neighbour data will be written to.
 */
typedef struct FaceBuffer
{
    Faces face;
    int size;
    libusb_device_handle* device;
    int devPort;
    int devInterface;
    struct libusb_transfer* in_transfer;
    struct libusb_transfer* out_transfer;
    float* inData;
    float* outData;
} FaceBuffer;

/** 
 * Sets up the FaceBuffer f according to its FaceBuffer::face. 
 * Fills all the other fields and allocates memory for both buffers.
 * @param[in] f A FaceBuffer pointer to fill. Must have its FaceBuffer::face correctly set. 
 * @param[in] h A Header pointer containing room information used to calculate the buffer sizes.
*/
void setupFaceBuffer(FaceBuffer* f, Header* h);

/** 
 * Fills the FaceBuffer buf with the appropriate data from nodes
 * @param[in] nodes A 3D Node array representing the room state to read data from. 
 * @param[in] h A Header pointer containing room dimensions.
 * @param[in] buf A FaceBuffer pointer whose FaceBuffer::outData will be filled.
*/
void fillFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf);

/** 
 * Reads the FaceBuffer buf to the appropriate nodes positions.
 * @param[in] nodes A 3D Node array representing the room state to write data to.
 * @param[in] h A Header pointer containing room dimensions.
 * @param[in] buf A FaceBuffer pointer whose FaceBuffer::inData will be read.
*/
void readFaceBuffer(Node*** nodes, Header* h, FaceBuffer* buf);

/** 
 * The returns the opposing face of f.
 * @param[in] f The face to determine the opposite to.
 * @param[out] oppF The opposite face to f.
*/
Faces getOpposingFace(Faces f);