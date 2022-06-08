#include "monitor.h"

// based on https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/

// int main()
// {
//     MonitorData data = { 123456, 50.0f, 0.0f, 1.0f, 2.0f, 3.0f };

//     monitorSend(&data);

//     return EXIT_SUCCESS;
// }

void monitorSend(MonitorData* monitorData)
{
    int sockfd;
    struct sockaddr_in servaddr;
   
    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) 
    {
        printf("socket creation failed...\n");
        exit(0);
    }
    else printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));
   
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);
   
    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
    {
        printf("connection with the server failed...\n");
        return;
    }
    else printf("connected to the server..\n");
   
    // debug to file
    // FILE* f = fopen("monitor.dbg", "w");
    // int fileDescriptor = fileno(f);
    // ssize_t ret = write(fileDescriptor, monitorData, sizeof(MonitorData));
    // printf("Written %ld bytes to file\n", ret);
    // close(fileDescriptor);
    // fclose(f);

    // send data to socket
    if(write(sockfd, monitorData, sizeof(MonitorData)) != sizeof(MonitorData))
    {
        printf("Something went wrong sending data to the monitor\n");
    }
   
    // close the socket
    close(sockfd);
}