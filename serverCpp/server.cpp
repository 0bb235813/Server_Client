#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
// #include "unp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <strings.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

void str_echo(int sockfd)
{
    cout << "str_echo" << endl;
    ssize_t n;
    char buf[128];

    for (;;) {
        cout << "read()" << endl;
        if ((n = read(sockfd, buf, sizeof(buf))) <= 0)
        {
            cout << "if()" << endl;
            return;
        }
        buf[n] = '\0';
        cout << "n:    " << n << endl;
        cout << "buf:    " << buf << endl;
        cout << "write()" << endl;
        write(sockfd, buf, n);
        cout << "/////////////////////////////////////////"<< endl;
    }
}

void sig_chld(int signo)
{
    cout << "sig_chld()" << endl;
    pid_t pid;
    int stat;

    // pid = wait(&stat);
    // printf("child %d terminated\n", pid);
    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}

int main()
{
    int listenfd, connfd;
    pid_t childpid;
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));

    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8888);
    servaddr.sin_family = AF_INET;

    bind(listenfd, (sockaddr*)&servaddr, sizeof(servaddr));
    cout << "listing" << endl; 
    listen(listenfd, 5);
    signal(SIGCHLD, sig_chld);

    for(;;)
    {
        clilen = sizeof(cliaddr);
        cout << "accept()" << endl;
        if ( (connfd = accept(listenfd, (sockaddr*)&cliaddr, &clilen)) < 0) {
            if (errno == EINTR) {
                    cout << "EINTR" << endl;
                    continue;
                }
                else
                    cout << "accept error" << endl;
                    return 0;
        }
        cout << "client connected" << endl;
        if ((childpid = fork()) == 0) {
            cout << "fork(): " << childpid << endl;
            close(listenfd);
            str_echo(connfd);
            cout << "child exit()" << endl;
            exit(0);
        }
        cout << "main procces" << endl;
        close(connfd);
    }
    cout << "exit(0)" << endl;
    exit(0);
}
