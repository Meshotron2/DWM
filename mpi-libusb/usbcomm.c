#include "usbcomm.h"

static pthread_t usb_thread;
static pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

static bool initialized = false;
static bool ready = false;
static int completedCount;
static int transferCount;
static Config* config;

static void callbackReceiveComplete(struct libusb_transfer *xfr);
static void callbackSendComplete(struct libusb_transfer *xfr);
static void usbDestroyDevice(FaceBuffer* fb);
static void usbPrepareDevice(FaceBuffer* fb);
static void* threadLoop(void* a);

static void callbackReceiveComplete(struct libusb_transfer *xfr)
{
    // printf("receive called\n");
    if(xfr->status == LIBUSB_TRANSFER_COMPLETED)
    {
        if(xfr->actual_length == ((FaceBuffer*)xfr->user_data)->size * (int)sizeof(float))
        {
            completedCount++;
        }
        else
        {
            printf("Weird receive transfer size received of %d bytes\n", xfr->actual_length);
        }
    }
}

static void callbackSendComplete(struct libusb_transfer *xfr)
{
    // printf("send called\n");
    if(xfr->status == LIBUSB_TRANSFER_COMPLETED)
    {
        if(xfr->actual_length == ((FaceBuffer*)xfr->user_data)->size * (int)sizeof(float))
        {
            completedCount++;
        }
        else
        {
            printf("Weird send transfer size sent\n");
        }
    }
}

static void usbDestroyDevice(FaceBuffer* fb)
{
    libusb_dev_mem_free(fb->device, (unsigned char*)fb->inData, fb->size * sizeof(float));
    libusb_dev_mem_free(fb->device, (unsigned char*)fb->outData, fb->size * sizeof(float));
    libusb_free_transfer(fb->in_transfer);
    libusb_free_transfer(fb->out_transfer);
    libusb_release_interface(fb->device, fb->devInterface);
    libusb_attach_kernel_driver(fb->device, fb->devInterface);
    libusb_close(fb->device);
}

static void usbPrepareDevice(FaceBuffer* fb)
{
    printf("Face %d is %d bytes\n", fb->face, fb->size);
    
    libusb_device** list;
    ssize_t count = libusb_get_device_list(NULL, &list);
    printf("Prepare device found %d USB devices\n", count);

    uint8_t interface;
    bool found = false;
    for(int i = 0; i < count; i++) 
    {
        struct libusb_device_descriptor desc;

        libusb_get_device_descriptor(list[i], &desc);
        
        if(desc.idVendor == PROLIFIC_VID && (desc.idProduct == PL25A1_PID || desc.idProduct == PL2501_PID))
        {
            printf("Found a Prolific Host to Host Bridge Controller\n");
            libusb_open(list[i], &(fb->device));
            
            struct libusb_config_descriptor* config_desc;
            libusb_get_active_config_descriptor(list[i], &config_desc);

            printf("Total number of interfaces is %d\n", config_desc->bNumInterfaces);
            printf("Found device on interface %d\n", config_desc->interface->altsetting->bInterfaceNumber);
            interface = config_desc->interface->altsetting->bInterfaceNumber;
            libusb_free_config_descriptor(config_desc);

            uint8_t port = libusb_get_port_number(list[i]);

            if(port == fb->devPort)
            {
                found = true;
                printf("Found a device on port %d for face %d\n", port, fb->face);
                break;
            }
        }
    }

    if(!found)
    {
        printf("Failed to find a Prolific Host to Host Bridge Controller\n");
        libusb_exit(NULL);
        abort();
    }

    printf("Checking if kernel driver is active.\n");
    if(libusb_kernel_driver_active(fb->device, (int)interface))
    {
        printf("Kernel driver is attached. Detaching.\n");
        libusb_detach_kernel_driver(fb->device, (int)interface);
    }
    printf("Claiming interface for ourselves\n");
    int t = libusb_claim_interface(fb->device, (int)interface);
    printf("Claim interface returned %d\n", t);

    fb->inData = (float*)libusb_dev_mem_alloc(fb->device, sizeof(float) * fb->size);
    fb->outData = (float*)libusb_dev_mem_alloc(fb->device, sizeof(float) * fb->size);

    fb->in_transfer = libusb_alloc_transfer(0);
    fb->out_transfer = libusb_alloc_transfer(0);

    fb->devInterface = interface;

    libusb_fill_bulk_transfer(fb->out_transfer,
            fb->device,
            BULK_USB2_EP1_OUT_ADDR ,
            (unsigned char*)fb->outData,
            fb->size * sizeof(float),
            callbackSendComplete,
            fb,
            0
            );

    libusb_fill_bulk_transfer(fb->in_transfer,
            fb->device,
            BULK_USB2_EP1_IN_ADDR ,
            (unsigned char*)fb->inData,
            fb->size * sizeof(float),
            callbackReceiveComplete,
            fb,
            0
            );

    // flush IN FIFO
    // sometimes multiple transfers can remain inside the fifo (especially if the kernel driver (usbnet) was attached before)
    // the max number of remaining transfers that i managed to find was 4. we try 8 just to be safe.
    unsigned char* tmp = (unsigned char*)malloc(BULK_USB2_EP1_FIFO_SIZE);
    for(int i = 0; i < 8; i++)
    {   
        libusb_bulk_transfer(fb->device, BULK_USB2_EP0_IN_ADDR, tmp, BULK_USB2_EP1_FIFO_SIZE, NULL, 50);
    }
    free(tmp);

    // libusb_free_device_list(list, 1);
}

static void* threadLoop(void* a)
{
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 100000; // 100 msec 
    
    while(initialized)
    {
        libusb_handle_events_timeout_completed(NULL, &tv, NULL); // timeout value so this doesn't block forever when we're trying to break the loop

        if(completedCount == transferCount)
        {
            pthread_mutex_lock(&mut);
            completedCount = 0;
            ready = true;
            pthread_cond_broadcast(&cond);
            pthread_mutex_unlock(&mut);
        }
    }
    
    return NULL;
}

void usbInit(Config* c)
{
    libusb_init(NULL);
    
    config = c;
    transferCount = c->faceCount * 2;
    for(int f = 0; f < c->faceCount; f++)
    {
        usbPrepareDevice(&c->faces[f]);
    }

    initialized = true;

    pthread_create(&usb_thread, NULL, threadLoop, NULL);
}

void usbDestroy()
{
    initialized = false;
    pthread_join(usb_thread, NULL);
    for(int f = 0; f < config->faceCount; f++)
    {
        usbDestroyDevice(&config->faces[f]);
    }
    libusb_exit(NULL);
}

void usbSend()
{
    if(ready == true)
    {
        printf("Bad!!\n");
        abort();
    }
    for(int f = 0; f < config->faceCount; f++)
    {
        libusb_submit_transfer((&(config->faces[f]))->in_transfer);
        libusb_submit_transfer((&(config->faces[f]))->out_transfer);
    }
}

void usbRecv()
{
    pthread_mutex_lock(&mut);
    while(!ready)
    {
        pthread_cond_wait(&cond, &mut);
    }
    ready = false;
    pthread_mutex_unlock(&mut);
}