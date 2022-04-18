#pragma once

#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#include "facebuffer.h"
#include "config.h"
#include "pl25a1.h"

void usbInit(Config* c);
void usbDestroy();
void usbSend();
void usbRecv();