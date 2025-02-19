#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <iostream>
#include <wait.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
typedef void Sigfunc(int);

char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen);
ssize_t readn(int fd, void *vptr, size_t n);
ssize_t writen(int fd, const void *vptr, size_t n);
ssize_t readlineOneByte(int fd, void *vptr, size_t maxlen);
static ssize_t my_read(int fd, char *ptr);
ssize_t readline(int fd, void *ptr, size_t maxlen);
ssize_t readlinebuf(void **vptrptr);
void handler (int signo);
Sigfunc *signal(int signo, Sigfunc *func);
void sig_chld(int signo);
void Listen(int fd, int backlog);
static int read_cnt;
static char *read_ptr;
static char read_buf[128];

// void (*signal(int signo, void (*func)(int)))(int);


void Listen(int fd, int backlog)
{
    char *ptr;
    if ((ptr = getenv("LISTENQ")) != NULL)
        backlog = atoi(ptr);

    if (listen(fd, backlog) < 0) {
        std::cout << "listen error" << std::endl;
        return;
    }
}

char *sock_ntop(const struct sockaddr *sa, socklen_t addrlen)
{
    char portstr[7];
    static char str[128];

    switch (sa->sa_family)
    {
        case AF_INET:
        {
            sockaddr_in *sin = (sockaddr_in *) sa;

            if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == 0)
                return (0);
            if (ntohs(sin->sin_port) != 0) {
                snprintf(portstr, sizeof(portstr), "%d", ntohs(sin->sin_port));
                strcat(str, portstr);
            }
            //// return str;
        }
    }
    return str;
}

ssize_t readn(int fd, void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nread;
    char *ptr;

    ptr = static_cast<char*>(vptr);
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        }
        else if (nread == 0)
            break;  /* EOF*/

        nleft -= nread;
        ptr += nread;
    }
    return n-nleft; /* возвращает значени >= 0 */
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = static_cast<const char*>(vptr);
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (errno == EINTR)
                nwritten = 0; /* и снова вызывает функцию write() */
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

ssize_t readlineOneByte(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = static_cast<char*>(vptr);
    for (n = 1; n < maxlen; n++)
    {
        again:
            if ( (rc = read(fd, &c, 1)) == 1){
                * ptr++ = c;
                if (c == '\n')
                    break;          /* Записан символ новой строки, как в fgets() */
            } else if (rc == 0) {
                if (n == 1)
                    return 0;       /* EOF, данные не считаны */
                else
                    break;          /* EOF, некоторые данные были считаны */
            } else {
                if (errno == EINTR)
                    goto again;
                return -1;          /* ошибка, errno задается функцией read()*/
            }
    }
    *ptr = 0;       /* завершаем нулем, как в fgets() */
    return n;
}

static ssize_t my_read(int fd, char *ptr)
{
    if (read_cnt <= 0) {
        again:
            if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0) {
                if (errno == EINTR)
                    goto again;
                return -1;
            }
            else if (read_cnt == 0)
                return 0;
            read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;

    ptr = static_cast<char*>(vptr);
    for (n = 1; n < maxlen; n++) {
        if ((rc = my_read(fd, &c)) == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        }
        else if (rc == 0) {
            *ptr = 0;
            return n-1;
        }
        else
            return -1;
    }

    *ptr = 0;
    return n;
}

ssize_t readlinebuf(void **vptrptr)
{
    if (read_cnt)
        *vptrptr = read_ptr;
    return read_cnt;
}

Sigfunc *signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;   /* SunOS 4.x */
#endif
    }
    else {
#ifdef SA_RESTART
    act.sa_flags |= SA_RESTART;     /* SVR4, 4.4BSD */
#endif
    }
    if (sigaction(signo, &act, &oact) < 0)
        return SIG_ERR;
    return oact.sa_handler;
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    // pid = wait(&stat);
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated\n", pid);
    return;
}