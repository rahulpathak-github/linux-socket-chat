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
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>

#define PORT 8001
#define BUFF_SIZE 256
#define MAX_CLIENTS 5

using namespace std;

pthread_t sender, reciever;

void handleSig(int sig_num)
{
    signal(SIGINT, handleSig);
    fflush(stdout);
    return;
}

void *sender_func(void *args)
{
    cout << "+----------MENU-----------+\n"
         << "1. Show Menu --> 'menu' \n"
         << "2. Show Nodes' Stats --> 'stats' \n"
         << "3. Pair a node --> 'connect <NODE_SOCKET_FD>' \n"
         << "4. Unpair --> 'goodbye' \n"
         << "5. Disconnect from Server --> 'close' \n";
    int sockfd = *((int *)args);
    while (1)
    {
        signal(SIGINT, handleSig);
        char buffer[BUFF_SIZE];
        bzero(buffer, BUFF_SIZE);
        // printf("Enter message: ");
        // fflush(stdout);
        fgets(buffer, BUFF_SIZE, stdin);
        if (strncmp(buffer, "menu", 4) == 0)
        {
            cout << "+----------MENU-----------+\n"
                 << "1. Show Menu --> 'menu' \n"
                 << "2. Show Nodes' Stats --> 'stats' \n"
                 << "3. Pair a node --> 'connect <NODE_SOCKET_FD>' \n"
                 << "4. Unpair --> 'goodbye' \n"
                 << "5. Disconnect from Server --> 'close' \n";
        }
        write(sockfd, buffer, BUFF_SIZE);
        // if (strncmp(buffer, "close", 5) == 0)
        // {
        //     break;
        // }
    }
}

void *reciever_func(void *args)
{
    int sockfd = *((int *)args);
    while (1)
    {
        signal(SIGINT, handleSig);
        char buffer[BUFF_SIZE];
        bzero(buffer, BUFF_SIZE);
        read(sockfd, buffer, BUFF_SIZE);
        printf("MESSAGE: %s\n", buffer);

        if (strncmp(buffer, "CONN_TRUE", 9) == 0)
        {
            char *param1 = strtok(buffer, " "), *param2 = strtok(NULL, " ");
            cout << "Connected to fd " << param2 << endl;
            bzero(buffer, BUFF_SIZE);
            bcopy("CONN_RECOG", buffer, 10);
            write(sockfd, buffer, BUFF_SIZE);
        }
        else if (strncmp(buffer, "goodbye", 7) == 0)
        {
            // printf("MESSAGE: %s\n", buffer);
            write(sockfd, buffer, BUFF_SIZE);
            cout << "Unpaired! (Disconnected from client)\n";
        }
        else if (strncmp(buffer, "DISCONN_TRUE", 12) == 0)
        {
            pthread_cancel(sender);
            break;
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
    serverAdd.sin_addr.s_addr = inet_addr("10.10.39.47");

    signal(SIGINT, handleSig);
    // cout << "DEBUG_63" << endl;

    if (connect(sockfd, (struct sockaddr *)&serverAdd, sizeof(serverAdd)) == 0)
    {

        // cout << "DEBUG_68" << endl;
        // sleep(10);

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
