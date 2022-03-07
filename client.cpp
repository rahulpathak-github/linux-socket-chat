#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <string>
#include <iostream>

#define PORT 8001
#define BUFF_SIZE 256
#define MAX_CLIENTS 5

using namespace std;

void *sender_func(void *args)
{
    int sockfd = *((int *)args);
    while (1)
    {
        char buffer[BUFF_SIZE];
        bzero(buffer, BUFF_SIZE);
        // printf("Enter message: ");
        // fflush(stdout);
        fgets(buffer, BUFF_SIZE, stdin);
        write(sockfd, buffer, BUFF_SIZE);
        if (strncmp(buffer, "close", 5) == 0)
        {
            break;
        }
    }
}

void *reciever_func(void *args)
{
    int sockfd = *((int *)args);
    while (1)
    {
        char buffer[BUFF_SIZE];
        bzero(buffer, BUFF_SIZE);
        read(sockfd, buffer, BUFF_SIZE);
        printf("MESSAGE: %s\n", buffer);
        if (strncmp(buffer, "CONN_TRUE", 9) == 0)
        {
            bzero(buffer, BUFF_SIZE);
            bcopy("CONN_RECOG", buffer, 10);
            write(sockfd, buffer, BUFF_SIZE);
        }
        else if (strncmp(buffer, "goodbye", 7) == 0)
        {
            // printf("MESSAGE: %s\n", buffer);
            write(sockfd, buffer, BUFF_SIZE);
        }
        // fflush(stdout);
    }
}

int main()
{
    int sockfd;
    struct sockaddr_in serverAdd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd <= 0)
    {
        perror("[ X ] Socket creation failed!\n");
        exit(1);
    }

    printf("[ / ] Socket created successfully!\n");

    memset(&serverAdd, '\0', sizeof(serverAdd));
    serverAdd.sin_family = AF_INET;
    serverAdd.sin_port = htons(PORT);
    serverAdd.sin_addr.s_addr = INADDR_ANY;

    // cout << "DEBUG_63" << endl;

    if (connect(sockfd, (struct sockaddr *)&serverAdd, sizeof(serverAdd)) == 0)
    {

        // cout << "DEBUG_68" << endl;
        // sleep(10);
        pthread_t sender, reciever;
        pthread_create(&sender, NULL, sender_func, (void *)&sockfd);
        pthread_create(&reciever, NULL, reciever_func, (void *)&sockfd);

        // pthread_join(reciever, NULL);
        pthread_join(sender, NULL);
    }
    else
    {
        perror("[ X ]Connection with the server failed...\n");
        exit(1);
    }

    close(sockfd);
}
