#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
// #include "unp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

void str_cli(FILE *fp, int sockfd)
{
    cout << "str_cli" << endl;
    int n, maxfd;
    bool stdineof = false;
    char buf[128];
    fd_set rset;

    FD_ZERO(&rset);
    while (true) {
        if (!stdineof)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfd = max(sockfd, fileno(fp)) + 1;
        cout << "select()" << endl;
        select(maxfd, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {
            cout << "read()" << endl;
            if ((n = read(sockfd, buf, sizeof(buf))) == 0) {
                if (stdineof) {
                    cout << "norm end" << endl;
                    return;
                }
                else {
                    cout << "str_sli: server terminated prematurely" << endl;
                    return; 
                }
            }
            buf[n] = '\0';  //*
            cout << "n:    " << n << endl;
            cout << "strlen:    " << strlen(buf) << endl;
            cout << "writeSTDOUT():    " << endl;
            write(fileno(stdout), buf, strlen(buf));    //*
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            if ((n = read(fileno(fp), buf, sizeof(buf))) == 0) {
                stdineof = true;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);
                cout << "continue" << endl;
                continue;
            }
            cout << "write()" << endl;
            write(sockfd, buf, n-1);
        }
        cout << "\n/////////////////////////////////////////"<< endl;
    }
    cout << "exit str_cli()" << endl;
}

int main()
{
    int sockfd[5];
    struct sockaddr_in servaddr;

    for(int i = 0; i < 5; i++) {
        sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(8888);
        inet_pton(AF_INET, "172.27.190.29", &servaddr.sin_addr);
        
        connect(sockfd[i], (sockaddr*)&servaddr, sizeof(servaddr));
    }
    cout << "CONNECTED" << endl;

    str_cli(stdin, sockfd[0]);

    cout << "exit()" << endl;
    exit(0);
}