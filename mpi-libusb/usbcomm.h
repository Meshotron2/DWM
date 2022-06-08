#pragma once

/** @file */

#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include "facebuffer.h"
#include "config.h"
#include "pl25a1.h"

/** 
 * Initializes the USB communication from a given Config.
 * When you do not need it anymore you should call usbDestroy()
 * @param[in] c A pointer to the Config containing the node configuration details. 
*/
void usbInit(Config* c);

/** 
 * Destroys an already initialized usb communication initialized by usbInit(Config* c)
*/
void usbDestroy();

/** 
 * Sends the data in each FaceBuffer::outData in Config::faces to the corresponding neighbours
*/
void usbSend();

/** 
 * Reads the data from neighbours to the corresponding FaceBuffer::inData in Config::faces
*/
void usbRecv();