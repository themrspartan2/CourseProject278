// Nathan Farrar

#define MYPORT 8080
#define BUFFER_SIZE 1024
#define IP "127.0.0.1"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>
#include <thread>

using namespace std;

int main(int argc, char const *argv[])
{
    // set up the socket
    int connID;
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(MYPORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP);
    connID = socket(AF_INET, SOCK_STREAM, 0);

    // setting timeout time
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    fd_set rfds;
    int maxfd, retval;

    // connect to the server
    // 0 = success, -1 = error
    if (connect(connID, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("connect");
        exit(1);
    }

    cout << "Connection established." << endl;
    cout << "Type /exit to disconnect." << endl;
    cout << "TYpe /<username> to direct message." << endl;
    cout << "Enter your username: " << flush;

    while (1)
    {
        // clear the collection of readable file descriptors
        FD_ZERO(&rfds);
        // add standard input file descriptors to the collection
        FD_SET(0, &rfds);
        maxfd = 0;
        // add the currently connected file descriptor to the collection
        FD_SET(connID, &rfds);
        // find the largest file descriptor in the file descriptor set
        if (maxfd < connID)
        {
            maxfd = connID;
        }

        // waiting for message
        retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (retval == -1)
        {
            printf("Select error, exiting client program.\n");
            break;
        }
        else if (retval == 0)
        {
            // waiting for input from the client
            // continue;
        }
        else
        {
            // The server sent a message.
            if (FD_ISSET(connID, &rfds))
            {
                char recvbuf[BUFFER_SIZE];
                memset(recvbuf, 0, sizeof(recvbuf));
                recv(connID, recvbuf, sizeof(recvbuf), 0);
                cout << recvbuf << flush;
            }

            // When the user enters information, process the information and send it.
            if (FD_ISSET(0, &rfds))
            {
                char sendbuf[BUFFER_SIZE];
                memset(sendbuf, 0, sizeof(sendbuf));
                fgets(sendbuf, sizeof(sendbuf), stdin);
                send(connID, sendbuf, strlen(sendbuf), 0);
                // if the message is "/exit" then end the connection
                if (sendbuf[0] == '/' &&
                    sendbuf[1] == 'e' &&
                    sendbuf[2] == 'x' &&
                    sendbuf[3] == 'i' &&
                    sendbuf[4] == 't')
                {
                    cout << "Connection terminated.\n";
                    close(connID);
                    return 0;
                }
            }
        }
    }

    cout << "Connection terminated unexpectedly.\n";
    close(connID);

    return 0;
}
