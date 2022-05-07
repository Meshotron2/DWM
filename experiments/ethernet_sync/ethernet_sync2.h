#pragma once

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#define INTERFACE_NAME	    "ens33"
#define RECV_BUF_SIZE		8192
#define ETHER_TYPE          0xAAAA

struct ether_sync_packet
{
    struct ether_header ether_header;
    int16_t process_id;
    int8_t is_hello;
    int8_t pad;
};

static pthread_t eth_thread;
static pthread_mutex_t mut;
static pthread_cond_t cond;
static bool is_initialized;
static bool is_ready;
static int16_t total_processes;
static int16_t process_id;

static int sockfd;
static struct sockaddr_ll send_socket_addr;
static struct ether_sync_packet ether_sync_packet;

static char recvBuffer[RECV_BUF_SIZE];
// static int16_t* ready_state;
// static int16_t ready_count;

void Init(int16_t num_processes, int16_t process);
static void InitInternal();
void Destroy();
void Barrier();
static void *Loop(void *args);
static void firstSetup(int16_t* ready_state);