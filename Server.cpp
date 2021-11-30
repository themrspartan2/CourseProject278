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

//check if a username is present in the database
string lookup(string username)
{
    //change this to SQL query later
    if (username == "farrarng")
    {
        return "qwerty";
    }
    else
    {
        cout << "User not present in database. Create account? Y/N\n";
        return "";
    }
}

//send a comment recieved from a user to the chat
void postComment(string comment)
{
    //prepend '<username of sender>: ' to 'comment'
    //send 'comment' to all clients
}

void login()
{
    string username;
    string password;
    bool found = false;
    while (!found)
    {
        //this is just a test, not acutal code
        cout << "Enter username: ";
        cin >> username;
        password = lookup(username);
        if (password == "")
        {
            cout << "Username not found in the database, try again.\n";
        }
        else
        {
            found = true;
        }
    }

    cout << "Enter password: ";
    string input;
    cin >> input;
    while (input != password)
    {
        cout << "Password does not match, try again.\n";
        cout << "Enter password: ";
        cin >> input;
    }
}

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
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    while (1)
    {
        std::list<int>::iterator it;
        for (it = li.begin(); it != li.end(); ++it)
        {
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
                printf("select error\n");
            }
            else if (retval == 0)
            {
                //printf("not message\n");
            }
            else
            {
                char buf[1024];
                memset(buf, 0, sizeof(buf));
                int len = recv(*it, buf, sizeof(buf), 0);
                printf("%s", buf);
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
        //printf("you are send %s", buf);
        std::list<int>::iterator it;
        for (it = li.begin(); it != li.end(); ++it)
        {
            send(*it, buf, sizeof(buf), 0);
        }
    }
}

int main()
{
    //new socket
    s = socket(AF_INET, SOCK_STREAM, 0);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
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
    std::thread t(getConn);
    t.detach();
    //printf("done\n");
    //thread : input ==>> send
    std::thread t1(sendMess);
    t1.detach();
    //thread : recv ==>> show
    std::thread t2(getData);
    t2.detach();
    while (1)
    {
    }
    return 0;
}