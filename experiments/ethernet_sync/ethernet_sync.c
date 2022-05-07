#include "ethernet_sync.h"

void Init(int num_processes, int process)
{
    /**
     * Create a non-blocking raw socket.
    */
    if ((sockfd = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK, htons(ETH_P_ALL))) == -1) {
	    perror("Socket Error: ");
	}

    struct ifreq if_idx;
	struct ifreq if_mac;

    /** 
     * Get the index of the interface to send on 
     */
	memset(&if_idx, 0, sizeof(struct ifreq));
	strncpy(if_idx.ifr_name, INTERFACE_NAME, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFINDEX, &if_idx) < 0)
	{
        perror("SIOCGIFINDEX Error: ");
    }

	/**
     * Get the MAC address of the interface to send on 
     */
	memset(&if_mac, 0, sizeof(struct ifreq));
	strncpy(if_mac.ifr_name, INTERFACE_NAME, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &if_mac) < 0)
	{
        perror("SIOCGIFHWADDR Error: ");
    }

    /**
     * Clear receiving buffers
     */
    memset(recvBuffer, 0, RECV_BUF_SIZE);

    /**
     * Fill the Ethernet header with our interface mac address and our
     * destination mac address.
     * Our destination address is always FF:FF:FF:FF:FF:FF (broadcast)
     * so the switch sends it to everyone else.
     */
    ether_sync_packet.ether_header.ether_shost[0] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[0];
	ether_sync_packet.ether_header.ether_shost[1] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[1];
	ether_sync_packet.ether_header.ether_shost[2] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[2];
	ether_sync_packet.ether_header.ether_shost[3] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[3];
	ether_sync_packet.ether_header.ether_shost[4] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[4];
	ether_sync_packet.ether_header.ether_shost[5] = ((uint8_t *)&if_mac.ifr_hwaddr.sa_data)[5];
	ether_sync_packet.ether_header.ether_dhost[0] = 0xFF;
	ether_sync_packet.ether_header.ether_dhost[1] = 0xFF;
	ether_sync_packet.ether_header.ether_dhost[2] = 0xFF;
	ether_sync_packet.ether_header.ether_dhost[3] = 0xFF;
	ether_sync_packet.ether_header.ether_dhost[4] = 0xFF;
	ether_sync_packet.ether_header.ether_dhost[5] = 0xFF;

    /**
     * Set Ethernet header type to a custom one.
     */
    ether_sync_packet.ether_header.ether_type = ETHER_TYPE;

    size_t packetSize = sizeof(struct ether_header);

    /**
     * Our packet data.
     * Increase packetSize.
     */
    ether_sync_packet.process_id = (int16_t)process;
    ether_sync_packet.is_hello = 0x1;

    /**
     * Set the index of the network device
     */
    send_socket_addr.sll_ifindex = if_idx.ifr_ifindex;
	
    /**
	 * Set Ethernet Address length
	 */
	send_socket_addr.sll_halen = ETH_ALEN;
	
    /**
     * Set the destination MAC address again.
     * Our destination address is always FF:FF:FF:FF:FF:FF (broadcast)
     * so the switch sends it to everyone else.
     */
	send_socket_addr.sll_addr[0] = 0xFF;
	send_socket_addr.sll_addr[1] = 0xFF;
	send_socket_addr.sll_addr[2] = 0xFF;
	send_socket_addr.sll_addr[3] = 0xFF;
	send_socket_addr.sll_addr[4] = 0xFF;
	send_socket_addr.sll_addr[5] = 0xFF;

    ready_state = (int16_t*)calloc(num_processes, sizeof(int16_t));
    ready_count = 1;

    while(ready_count < num_processes)
    {
        usleep(250);   
        
        if (sendto(sockfd, &ether_sync_packet, sizeof(struct ether_sync_packet), 0, (struct sockaddr*)&send_socket_addr, sizeof(struct sockaddr_ll)) < 0)
		{
			printf("Send failed\n");
		}

        ssize_t packet_len;
        struct sockaddr recv_socket_address;
        socklen_t s = sizeof(struct sockaddr_ll);
		if ((packet_len = recvfrom(sockfd, recvBuffer, RECV_BUF_SIZE, 0, &recv_socket_address, &s)) == sizeof(struct ether_sync_packet))	 
        {
            struct ether_sync_packet* ether_packet = (struct ether_sync_packet*)recvBuffer;

            if(ether_packet->ether_header.ether_type != ETHER_TYPE || !ether_packet->is_hello || ether_packet->process_id == process) continue;
            if(ether_packet->process_id > num_processes)
            {
                printf("Got an unexpected process id. Exiting\n");
                abort();
            }
            else if(ready_state[ether_packet->process_id] == 0)
            {
                printf("Process %d now ready\n", ether_packet->process_id);
                ready_state[ether_packet->process_id] = 1;
                ready_count++;
            }
        }    
    }

    ether_sync_packet.is_hello = 0x0;
}

void Destroy()
{
    if (ready_state != NULL)
    {
        free(ready_state);
    }
    close(sockfd);
}

void Barrier()
{

}

static void* Loop(void* p)
{
    
}

int main(int argc, char** argv)
{
    int num_processes = atoi(argv[1]);
    int process = atoi(argv[2]);
    
    Init(num_processes, process);

    printf("Process %d initialized\n", process);

    Destroy();
}