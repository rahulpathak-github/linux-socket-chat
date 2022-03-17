#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <string>
#include <map>
#include <iostream>

#define PORT 8001
#define MAX_CLIENTS 5
#define BUFF_SIZE 256

using namespace std;

struct clientStruct
{
    int fd;
    int status; // -1->available, fd->busy
};

map<int, int> fd_idx;
pthread_mutex_t muts[MAX_CLIENTS];
bool availableSlots[MAX_CLIENTS];
clientStruct clients[MAX_CLIENTS];

void showStats(int fd)
{
    char buffer[BUFF_SIZE];
    bzero(buffer, BUFF_SIZE);
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        string msg = "";
        if (fd_idx.find(clients[i].fd) != fd_idx.end())
        {
            msg = "Client_fd " + to_string(clients[i].fd) + ": ";

            if (clients[i].status == -1)
                msg = msg + "FREE";
            else
                msg = msg + "BUSY";

            msg = msg + "\n";
        }

        strcat(buffer, msg.c_str());
    }
    write(fd, buffer, BUFF_SIZE);
}

void communicator(int src_fd, int dest_fd)
{
    char buffer[BUFF_SIZE];
    while (1)
    {
        bzero(buffer, BUFF_SIZE);
        read(src_fd, buffer, BUFF_SIZE);
        if (strncmp(buffer, "close", 5) == 0)
        {
            string msg = "Send 'goodbye' to unpair another client before closing the client!";
            bzero(buffer, BUFF_SIZE);
            bcopy(msg.c_str(), buffer, msg.size());
            write(src_fd, buffer, BUFF_SIZE);
            continue;
        }
        else if (strncmp(buffer, "goodbye", 7) == 0)
        {
            write(dest_fd, buffer, BUFF_SIZE);
            return;
        }
        else if (strncmp(buffer, "stats", 5) == 0)
        {
            showStats(src_fd);
            continue;
        }
        // if (buffer[buffer.size() - 1] != '#')
        // cout << "DEBUG_41" << endl;
        //     continue;
        write(dest_fd, buffer, BUFF_SIZE);
    }
}

void *connector(void *args)
{
    int slot_idx = *((int *)args), senderFd = clients[slot_idx].fd;
    char buffer[BUFF_SIZE];
    cout << "[ / ] Client_fd " << clients[slot_idx].fd << " connected!" << endl;

    showStats(senderFd);
    while (1)
    {
        bzero(buffer, BUFF_SIZE);

        // wait for connection with another client
        read(senderFd, buffer, BUFF_SIZE);

        // printf("%s\n", buffer);

        char *param1 = strtok(buffer, " "), *param2 = strtok(NULL, " ");

        int destFd = -1;

        if (param2 != NULL)
            destFd = atoi(param2);

        // cout << "DEBUG_77" << endl;
        if (strncmp(param1, "close", 5) == 0)
        {
            char tempBuff[BUFF_SIZE];
            string msg = "DISCONN_TRUE";
            bzero(buffer, BUFF_SIZE);
            bcopy(msg.c_str(), tempBuff, msg.size());
            write(senderFd, tempBuff, BUFF_SIZE);
            break;
        }
        else if (destFd != -1 && (fd_idx.find(destFd) == fd_idx.end() || destFd == senderFd))
        {
            char err[BUFF_SIZE] = "INVALID_FD";
            write(senderFd, &err, sizeof(err));
        }
        else if (strncmp(param1, "stats", 5) == 0)
        {
            showStats(senderFd);
        }
        else if (param2 != NULL && strncmp(param1, "connect", 7) == 0)
        {
            // cout << "DEBUG_80" << endl;
            if (pthread_mutex_trylock(&muts[slot_idx]) == 0)
            {
                // cout << "DEBUG_83" << endl;
                if (pthread_mutex_trylock(&muts[fd_idx[destFd]]) == 0)
                {
                    // cout << "DEBUG_86" << endl;
                    clients[slot_idx].status = destFd;
                    clients[fd_idx[destFd]].status = senderFd;
                    char connectedMsg[BUFF_SIZE];
                    string connMsg = "CONN_TRUE " + to_string(senderFd);
                    bcopy(connMsg.c_str(), connectedMsg, connMsg.size());
                    // bcopy("CONN_TRUE", connectedMsg, 9);
                    write(destFd, connectedMsg, BUFF_SIZE);
                    communicator(senderFd, destFd);
                }
                else
                {
                    char err[BUFF_SIZE] = "N_BUSY";
                    write(senderFd, &err, sizeof(err));
                }
            }
            else
            {
                char err[BUFF_SIZE] = "N_BUSY";
                write(senderFd, &err, sizeof(err));
            }
        }
        else if (param2 == NULL && strncmp(param1, "CONN_RECOG", 10) == 0 && clients[slot_idx].status != -1)
        {
            // cout << "DEBUG_108" << endl;
            char conn_msg[BUFF_SIZE];
            string connected = "Connected to fd " + to_string(senderFd);
            bcopy(connected.c_str(), conn_msg, connected.size());
            write(clients[slot_idx].status, &conn_msg, BUFF_SIZE);
            communicator(senderFd, clients[slot_idx].status);
        }

        if (clients[slot_idx].status != -1)
        {
            clients[slot_idx].status = -1;
            pthread_mutex_unlock(&muts[slot_idx]);
        }
    }
    pthread_mutex_unlock(&muts[slot_idx]);
    close(senderFd);
    fd_idx.erase(senderFd);
    availableSlots[slot_idx] = 1;
    cout << "[ / ] Client_fd " << clients[slot_idx].fd << " disconnected!" << endl;
}

int init()
{
    struct sockaddr_in server_addr;
    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd <= 0)
    {
        perror("[ X ] Socket creation failed!\n");
        exit(1);
    }

    printf("[ / ] Socket created successfully!\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        cout << "error in sockopt\n";
        exit(1);
    }

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("[ X ] Bind failed!\n");
        exit(1);
    }

    printf("[ / ] Bind successful on port: %d\n", (int)PORT);

    if (listen(sockfd, MAX_CLIENTS) != 0)
    {
        perror("[ X ] Listen attempt failed!\n");
        exit(1);
    }

    printf("[ / ] Listening on port: %d\nWaiting for incoming connections...\n", (int)PORT);

    return sockfd;
}

int main()
{

    int sockfd = init(), freeSlot = 0;
    struct sockaddr_in client_addr;
    socklen_t cliSize;

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        availableSlots[i] = 1;
        muts[i] = PTHREAD_MUTEX_INITIALIZER;
    }
    // cout << availableSlots[3] << endl;

    while (1)
    {
        cout << freeSlot << endl;
        clients[freeSlot].fd = accept(sockfd, (struct sockaddr *)&client_addr, &cliSize);
        if (clients[freeSlot].fd < 0)
        {
            perror("[ X ] Something went wrong while accepting\n");
            exit(1);
        }
        clients[freeSlot].status = -1;
        pthread_t tid;

        int curr_idx = freeSlot;
        if (pthread_create(&tid, NULL, connector, (void *)&curr_idx) == 0)
        {
            availableSlots[freeSlot] = 0;
            fd_idx[clients[freeSlot].fd] = freeSlot;

            // CAN BE DONE IN O(1) WITH QUEUE
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (availableSlots[i] == 1)
                {
                    freeSlot = i;
                    break;
                }
            }
            printf("[ / ] Successfully created new client thread!\n");
        }
        else
        {
            perror("[ X ] Error in creating client thread!\n");
            // fd_idx.erase(clients[freeSlot].fd);
            // availableSlots[freeSlot] = 1;
            close(clients[freeSlot].fd);
        }
    }
    return 0;
}