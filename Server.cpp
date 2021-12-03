//Nathan Farrar
#define MYSQLPP_MYSQL_HEADERS_BURIED
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
#include <mysql++/mysql++.h>

#define MYPORT 8080
#define BUFFER_SIZE 1024
#define IP "127.0.0.1"

using namespace std;

//ID of current connection
int connID;
//Server address
struct sockaddr_in serverAddr;
//Socket length
socklen_t length;
//List of active connections
list<int> activeConn;
list<int> queuedConn;
//Login database
mysqlpp::Connection myDB("CourseProject", "localhost", "cse278", "S3rul3z");

void Login()
{
    //server checks for message every 2 seconds
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;

    while (1)
    {
        //check every queued connection for a username
        list<int>::iterator it;
        for (it = queuedConn.begin(); it != queuedConn.end(); ++it)
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
                printf("Socket error\n");
            }
            else if (retval == 0)
            {
                //No messages were recieved from this connection
                continue;
            }
            else
            {
                //get sent a username from the user
                char ubuf[BUFFER_SIZE];
                memset(ubuf, 0, sizeof(ubuf));
                recv(*it, ubuf, sizeof(ubuf), 0);

                //If the message is /exit,
                //remove the user from the list of active connections
                //and announce their disconnection
                if (ubuf[0] == '/' &&
                    ubuf[1] == 'e' &&
                    ubuf[2] == 'x' &&
                    ubuf[3] == 'i' &&
                    ubuf[4] == 't')
                {
                    cout << "Qeueud Client #" << *it << " disconnected.\n";
                    queuedConn.erase(it--);
                    continue;
                }

                //This prints the client's username on the server terminal
                cout << "Login attempt for ";
                cout << ubuf;

                //find a user with that username and store the result
                mysqlpp::Query login = myDB.query();
                login << "SELECT * FROM Users WHERE Username = '%0';";
                login.parse();
                mysqlpp::StoreQueryResult result = login.store("Nathan");

                cout << result.num_rows() << endl;

                for (size_t row = 0; row < result.num_rows(); row++)
                {
                    string username = result[row][0].c_str();
                    string password = result[row][1].c_str();
                    cout << username << endl
                         << password << endl;
                }

                //if there is no result ask again, then move on
                if (result.num_rows() == 0)
                {
                    char response[BUFFER_SIZE] = "Username does not exist, try again.\n";
                    send(*it, response, sizeof(response), 0);
                }
                else
                {
                    //if the username exists, ask for the password
                    char pbuf[BUFFER_SIZE] = "Enter your password: ";
                    send(*it, pbuf, sizeof(pbuf), 0);
                    //get the password from the user
                    memset(pbuf, 0, sizeof(pbuf));
                    recv(*it, pbuf, sizeof(pbuf), 0);

                    string s(pbuf);
                    if (s != result[0][1].c_str())
                    {
                        char response[BUFFER_SIZE] = "Incorrect password.\n";
                        send(*it, response, sizeof(response), 0);
                    }
                    else
                    {
                        activeConn.push_back(*it);
                        string newuser = result[0][0].c_str();
                        newuser = newuser + " has joined.\n";
                        char announce[BUFFER_SIZE];
                        strcpy(announce, newuser.c_str());

                        list<int>::iterator it;
                        for (it = activeConn.begin(); it != activeConn.end(); ++it)
                        {
                            send(*it, announce, sizeof(announce), 0);
                        }
                    }
                }
            }
        }

        // activeConn.push_back(newConn);
        // list<int>::iterator it;
        // for (it = activeConn.begin(); it != activeConn.end(); ++it)
        // {
        //size_t row = 0;
        //string s = result[row][1].c_str();
        //char announce[BUFFER_SIZE] = response[0][0].c_str();
        // }
        //cout << "New Connection: Client #";
        //printf("%d\n", newConn);
        sleep(1);
    }
}

void getConnection()
{
    while (1)
    {
        int newConn = accept(connID, (struct sockaddr *)&serverAddr, &length);
        queuedConn.push_back(newConn);
        cout << "Queued Connection: Client #";
        printf("%d\n", newConn);
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
        //For every connection that is stored, iterate over all of them
        //Check each connection for a message
        list<int>::iterator it;
        for (it = activeConn.begin(); it != activeConn.end(); ++it)
        {
            fd_set rfds;
            FD_ZERO(&rfds);
            int maxfd, retval;
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
                //This client sent a message
                char buf[BUFFER_SIZE];
                memset(buf, 0, sizeof(buf));
                int len = recv(*it, buf, sizeof(buf), 0);
                //This prints the client's message on the server terminal
                printf("%s", buf);

                //If the message is /exit,
                //remove the user from the list of active connections
                //and announce their disconnection
                if (buf[0] == '/' &&
                    buf[1] == 'e' &&
                    buf[2] == 'x' &&
                    buf[3] == 'i' &&
                    buf[4] == 't')
                {
                    cout << "Client #" << *it << " disconnected.\n";
                    activeConn.erase(it--);
                }
                else
                {
                    string s(buf);
                    s = "Client #" + to_string(*it) + " said: " + s;
                    strcpy(buf, s.c_str());

                    //Send this to everyone but the sender
                    list<int>::iterator it2;
                    for (it2 = activeConn.begin(); it2 != activeConn.end(); ++it2)
                    {
                        if (it == it2)
                        {
                            continue;
                        }
                        send(*it2, buf, sizeof(buf), 0);
                    }
                }
            }
        }
        sleep(1);
    }
}

void serverMessage()
{
    while (1)
    {
        char buf[BUFFER_SIZE];
        fgets(buf, sizeof(buf), stdin);
        list<int>::iterator it;
        for (it = activeConn.begin(); it != activeConn.end(); ++it)
        {
            send(*it, buf, sizeof(buf), 0);
        }
    }
}

int main()
{
    //Create the socket for the connection number - ipv4 internet socket, TCP
    connID = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(MYPORT);
    serverAddr.sin_addr.s_addr = inet_addr(IP);

    //bind the socket with error checking
    if (bind(connID, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("bind");
        exit(1);
    }
    if (listen(connID, 20) == -1)
    {
        perror("listen");
        exit(1);
    }

    length = sizeof(serverAddr);

    //thread : while ==>> accpet
    thread t(getConnection);
    t.detach();

    //thread : input ==>> send
    thread t1(serverMessage);
    t1.detach();

    //thread : recv ==>> show
    thread t2(getData);
    t2.detach();

    thread t3(Login);
    t3.detach();

    while (1)
    {
    }
    return 0;
}