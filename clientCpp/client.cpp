#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include "unp.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#pragma comment(lib, "ws2_32.lib")
using namespace std;

void str_cli(FILE *fp, int sockfd)
{
    cout << "str_cli" << endl;
    char sendline[128], recvline[128];
    // memset(sendline, 0, sizeof(sendline));
    // memset(recvline, 0, sizeof(recvline));
    while (fgets(sendline, sizeof(sendline), fp)) {
        cout << "write()" << endl;
        write(sockfd, sendline, strlen(sendline));

        if (read(sockfd, recvline, sizeof(recvline)) <= 0) {
            cout << "str_sli: server terminated prematurely" << endl;
            return;
        }
        cout << strlen(recvline) << endl;
        cout << "fputs():    ";
        fputs(recvline, stdout);
    }
    cout << "exit str_cli()" << endl;
}

int main()
{
    int sockfd[5];
    struct sockaddr_in servaddr;

    // sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // bzero(&servaddr, sizeof(servaddr));
    // servaddr.sin_family = AF_INET;
    // servaddr.sin_port = htons(8888);
    // inet_pton(AF_INET, "192.168.54.218", &servaddr.sin_addr);
    // connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
    for(int i = 0; i < 5; i++) {
        sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(8888);
        inet_pton(AF_INET, "192.168.54.218", &servaddr.sin_addr);
        
        connect(sockfd[i], (sockaddr*)&servaddr, sizeof(servaddr));
    }
    cout << "CONNECTED" << endl;

    str_cli(stdin, sockfd[0]);

    cout << "exit()" << endl;
    exit(0);
}