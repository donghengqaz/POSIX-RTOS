#ifndef _SOCKET_H_
#define _SOCKET_H_

#include "rtos.h"
#include "lwip/ip_addr.h"

#define SOCK_STREAM                                 1
#define SOCK_PACKET                                 2
#define SOCK_DGRAM                                  3
#define SOCK_RAW                                    4
#define SOCK_SEQPACKET                              5

#define AF_INET                                     0

typedef int                          socklen_t;
typedef int                          fd_set;
typedef unsigned short               in_port_t;
typedef unsigned int                 in_addr_t;

struct in_addr
{
   in_addr_t s_addr;
};

struct sockaddr_in
{
    unsigned short _sa;
    
    /* net connection port */
    in_port_t sin_port;
    
    /* net connection IP address */
    struct in_addr sin_addr;
};

struct sockaddr
{
    unsigned short _sa;
    
    /* net connection port */
    in_port_t sin_port;
    
    /* net connection IP address */
    struct in_addr sin_addr; 
};

#define in_addr                      sockaddr

struct timeval
{
    unsigned int timeout;
};

#define INADDR_ANY    ip_addr_any.addr

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int listen(int sockfd, int backlog);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
int close(int fd);
err_t socket_init(void);

#endif
