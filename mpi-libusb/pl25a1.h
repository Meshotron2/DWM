//
//  libpl25A1.h
//  Library for accessing Prolific 25A1 USB devices
//
//  Created by software team in Prolific on 3/14/2017.
//  Copyright (c) 2017 Prolific Corp.. All rights reserved.
//
#pragma once
#define PROLIFIC_VID                   0x067B   // Prolific Vender ID
#define PL25A1_PID                     0x25A1   // Prolific Product ID
#define PL2501_PID                     0x2501   // Prolific Product IDS
#define BULK_USB2_EP0_IN_ADDR            0x81   // Interrupt endpoint  
#define BULK_USB2_EP1_OUT_ADDR           0x02   // Bulk endpoint 1 Out Address 
#define BULK_USB2_EP1_IN_ADDR            0x83   // Bulk endpoint 1 In Address
#define BULK_USB2_EP1_FIFO_SIZE          2048   // FIFO size in PL25A1 USB device is 2KB 