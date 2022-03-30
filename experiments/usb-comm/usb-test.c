#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "pl25a1.h"

// https://github.com/Mathias-L/STM32F4-libusb-example/blob/master/async.c
// https://stackoverflow.com/questions/49605169/libusb-race-condition-using-asynchronous-i-o
// https://falsinsoft.blogspot.com/2015/02/asynchronous-bulk-transfer-using-libusb.html

int sendCount = 0;
int recvCount = 0;
int size = 0;
int rounds = 0;
struct libusb_transfer* send;
struct libusb_transfer* recv;

void callbackSendComplete(struct libusb_transfer *xfr)
{
    switch(xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED: 
        {
            if(xfr->actual_length == size * sizeof(int))
            {
                if(sendCount < rounds)
                {
                    sendCount++;
                    libusb_submit_transfer(send);
                }
            }
            else
            {
                // re-submit
                libusb_submit_transfer(send);
            }
            break;
        }
        case LIBUSB_TRANSFER_TIMED_OUT:
        case LIBUSB_TRANSFER_CANCELLED:
        case LIBUSB_TRANSFER_NO_DEVICE:
        case LIBUSB_TRANSFER_ERROR:
        case LIBUSB_TRANSFER_STALL:
        case LIBUSB_TRANSFER_OVERFLOW:
            abort();
            break;
    }
}

void callbackReceiveComplete(struct libusb_transfer *xfr)
{
    switch(xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
        {
            if(xfr->actual_length == size * sizeof(int))
            {
                if(recvCount < rounds)
                {
                    int* buf = (int*)xfr->buffer;
                    recvCount++;
                    libusb_submit_transfer(recv);
                }
            }
            else
            {
                // re-submit
                libusb_submit_transfer(recv);
            }
            break;
        }
        case LIBUSB_TRANSFER_TIMED_OUT:
        case LIBUSB_TRANSFER_CANCELLED:
        case LIBUSB_TRANSFER_NO_DEVICE:
        case LIBUSB_TRANSFER_ERROR:
        case LIBUSB_TRANSFER_STALL:
        case LIBUSB_TRANSFER_OVERFLOW:
            abort();
            break;
    }
}

int main(int argc, char** argv)
{
    libusb_device_handle *dev_handle = NULL;
    libusb_device **list;
    ssize_t count, i;
    
    if(argc != 3)
    {
        printf("Usage: ./usb-test <size> <rounds>\n");
        return EXIT_FAILURE;
    }

    size = atoi(argv[1]);
    rounds = atoi(argv[2]);

    printf("Running %d rounds of %d bytes each\n", rounds, size * sizeof(int));
    
    // Initialize library
    printf("Initializing libusb\n");
    libusb_init(NULL);

    // Get list of USB devices currently connected
    count = libusb_get_device_list(NULL, &list);
    printf("Found %ld USB devices\n", count);

    uint8_t interface;
    bool found = false;
    for(i = 0; i < count; i++) 
    {
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(list[i], &desc);
        
        // Is this our device?
        if(desc.idVendor == PROLIFIC_VID && desc.idProduct == PL25A1_PID)
        {
            // Open USB device and get it's handle
            printf("Found a Prolific Host to Host Bridge Controller\n");
            libusb_open(list[i], &dev_handle);
            
            struct libusb_config_descriptor* config_desc;
            libusb_get_active_config_descriptor(list[i], &config_desc);

            printf("Total number of interfaces is %d\n", config_desc->bNumInterfaces);
            printf("Found device on interface %d\n", config_desc->interface->altsetting->bInterfaceNumber);
            interface = config_desc->interface->altsetting->bInterfaceNumber;
            libusb_free_config_descriptor(config_desc);

            found = true;

            printf("Device port number is: %d\n", libusb_get_port_number(list[i]));

            break;
        }
    }

    if(!found)
    {
        printf("Failed to find a Prolific Host to Host Bridge Controller\n");
        libusb_exit(NULL);
        return EXIT_FAILURE;
    }

    printf("Checking if kernel driver is active.\n");
    if(libusb_kernel_driver_active(dev_handle, (int)interface))
    {
        printf("Kernel driver is attached. Detaching.\n");
        libusb_detach_kernel_driver(dev_handle, (int)interface);
    }
    printf("Claiming interface for ourselves\n");
    int t = libusb_claim_interface(dev_handle, (int)interface);
    printf("Claim interface returned %d\n", t);

    struct timespec start, now;

    int* sendData = (int*)libusb_dev_mem_alloc(dev_handle, sizeof(int) * size);
    int* recvData = (int*)libusb_dev_mem_alloc(dev_handle, sizeof(int) * size);
    for(int i = 0; i < size; i++)
    {
        sendData[i] = i;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    send = libusb_alloc_transfer(0);
    recv = libusb_alloc_transfer(0);

    libusb_fill_bulk_transfer(send,
            dev_handle,
            BULK_USB2_EP1_OUT_ADDR ,
            (unsigned char*)sendData,
            size * sizeof(int),
            callbackSendComplete,
            NULL,
            0
            );
    libusb_fill_bulk_transfer(recv,
            dev_handle,
            BULK_USB2_EP1_IN_ADDR , 
            (unsigned char*)recvData,
            size * sizeof(int),
            callbackReceiveComplete,
            NULL,
            0
            );

    if(libusb_submit_transfer(send) < 0)
    {
        libusb_free_transfer(send);
        abort();
    }
    if(libusb_submit_transfer(recv) < 0)
    {
        libusb_free_transfer(recv);
        abort();
    }

    while(recvCount < rounds || sendCount < rounds)
    {
        if(libusb_handle_events_completed(NULL, NULL) != LIBUSB_SUCCESS) break;
    }

    libusb_dev_mem_free(dev_handle, (unsigned char*)sendData, sizeof(int) * size);
    libusb_dev_mem_free(dev_handle, (unsigned char*)recvData, sizeof(int) * size);

    libusb_free_transfer(send);
    libusb_free_transfer(recv);

    clock_gettime(CLOCK_MONOTONIC, &now);
	printf("Time Taken: %lf\n",
        (now.tv_sec - start.tv_sec) +
        1e-9 * (now.tv_nsec - start.tv_nsec)); 


    printf("Releasing the interface.\n");
    libusb_release_interface(dev_handle, (int)interface);

    // printf("Re-attaching the kernel driver.\n");
    // libusb_attach_kernel_driver(dev_handle, (int)interface);

    printf("Freeing device list.\n");
    libusb_free_device_list(list, 1);

    printf("Exiting libusb.\n");
    libusb_exit(NULL);

    return EXIT_SUCCESS;
}