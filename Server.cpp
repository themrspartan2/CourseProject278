// compile:  g++ -pthread Server.cpp -o Server
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
#include <list>

#define PORT 7000
#define IP "127.0.0.1"

using namespace std;

int s;
struct sockaddr_in servaddr;
socklen_t len;
list<int> li;

void getConn()
{
    while (1)
    {
        int conn = accept(s, (struct sockaddr *)&servaddr, &len);
        li.push_back(conn);
        printf("%d\n", conn);
    }
}

void getData()
{
    //server checks for message every 2 seconds
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (1)
    {
        list<int>::iterator it;
        for (it = li.begin(); it != li.end(); ++it)
        {
            std::cout << *it;
            fd_set rfds;
            FD_ZERO(&rfds);
            int maxfd = 0;
            int retval = 0;
            FD_SET(*it, &rfds);
            
            if (maxfd < *it)
            {
                maxfd = *it;
            }
            retval = select(maxfd + 1, &rfds, NULL, NULL, &tv);
            if (retval == -1)
            {
                printf("Socket error\n");
            }
            else if (retval == 0)
            {
                //No messages were recieved from this connection
            }
            else
            {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                int len = recv(*it, buf, sizeof(buf), 0);
                printf("%s", buf);
                //now send it to everyone but the sender
                //send(*it, buf, sizeof(buf), 0);
            }
        }
        sleep(1);
    }
}

void sendMess()
{
    while (1)
    {
        char buf[1024];
        fgets(buf, sizeof(buf), stdin);
        list<int>::iterator it;
        for (it = li.begin(); it != li.end(); ++it)
        {
            send(*it, buf, sizeof(buf), 0);
        }
    }
}

int main()
{
    //Create the socket - ipv4 internet socket, TCP
    s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);

    //bind the socket with error checking
    if (bind(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    if (listen(s, 20) == -1)
    {
        perror("listen");
        exit(1);
    }
    len = sizeof(servaddr);

    //thread : while ==>> accpet
    thread t(getConn);
    t.detach();

    //thread : input ==>> send
    thread t1(sendMess);
    t1.detach();

    //thread : recv ==>> show
    thread t2(getData);
    t2.detach();
    
    while (1)
    {
    }
    return 0;
}