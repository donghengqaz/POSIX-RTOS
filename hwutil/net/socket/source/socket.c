/*
 * File         : socket.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-24       DongHeng        create
 */

#include "socket.h"
#include "ipport.h"
#include "ippkg.h"

#include "stdlib.h"

#include "lwip/udp.h"
#include "lwip/tcp.h"

/* the structure description of socket */
struct socket
{ 
    int domain;
    int type;
    int protocol;
    
    /* for LWIP TCP or UDP PCB */
    void *pcb;

    mqd_t        rx_mqd;
    mqd_t        tx_mqd;
};

/**
  * socket_udp_recv - the function will be callbacked by LWIP when LWIP recieve 
  *                   a udp
  *
  * @param arg the user private point
  * @param pcb the point of the target udp pcb
  * @param pbuf the package of the udp
  * @param addr the address of the remote IP address
  * @param port the port number of the remote IP device
  */
static void socket_udp_recv(void *arg, 
                            struct udp_pcb *pcb,
                            struct pbuf *pbuf,
                            struct ip_addr *addr,
                            u16_t port)
{
    if (pbuf != NULL)
    {
        char ret = 0;
      
        struct socket *socket= (struct socket *)arg;
      
        mq_send(socket->rx_mqd, &ret, 1, 0);
    }
}

/**
  * socket_tcp_recv - the function will be callbacked by LWIP when LWIP recieve 
  *                   a tcp
  *
  * @param arg  the user private point
  * @param pcb  the point of the target tcp pcb
  * @param pbuf the package of the tcp, when it is NULL, losing tcp connnection
  * @param err  the error recieving
  *
  * @return the result
  */
static err_t socket_tcp_recv(void *arg, 
                             struct tcp_pcb *pcb,
                             struct pbuf *pbuf,
                             err_t err)
{
    struct socket *socket = (struct socket *)arg;
    char ret = 0;
  
    mq_send(socket->rx_mqd, &ret, 1, 0);
    
    return 0;
}

/**
  * socket_tcp_accept - the function will be callbacked by LWIP when connection
  *                     is actived
  *
  * @param arg  the user private point
  * @param pcb  the point of the target tcp pcb
  * @param err  the error recieving
  *
  * @return the result
  */
static err_t socket_tcp_accept(void *arg, struct tcp_pcb *pcb, err_t err)
{     
    tcp_recv(pcb, socket_tcp_recv);
  
    return 0;
}

/**
  * socket - the function will create a socket
  *                    
  *
  * @param domain   the family
  * @param type     the type of the socket
  * @param protocol the protocol of the socket transcieving
  *
  * @return the handle of the socket
  */
int socket(int domain, int type, int protocol)
{
    struct socket *socket = malloc(sizeof(struct socket));
    struct mq_attr mq_attr;
  
    mq_attr.mq_maxmsg  = 16;
    mq_attr.mq_msgsize = 4;
  
    socket->rx_mqd = mq_open("sockt", O_CREAT | O_RDWR, O_CREAT | O_RDWR, &mq_attr);
    socket->tx_mqd = mq_open("sockt", O_CREAT | O_RDWR, O_CREAT | O_RDWR, &mq_attr);
  
    /* fill the attribute */
    socket->domain = domain;
    socket->type = type;
    socket->protocol = protocol;
    
    if (SOCK_STREAM == type)
    {
        /* create a tcp pcb */
        if (!(socket->pcb = tcp_new()))
            goto free_socket;
    }
    else if (SOCK_PACKET == type)
    {
        /* create a udp pcb */
        if (!(socket->pcb = udp_new()))
            goto free_socket;
    }  
  
    return (int)socket;
    
free_socket:
    free(socket);
    
    return 0;
}

/**
  * bind - the function will bind the socket to local IP address and port, when
  *        system needs listenning to port, we use the function
  *                    
  * @param sockfd   the handle of the socket
  * @param addr     structure filled of ip address and port
  * @param addrlen  the length of the addr structure
  *
  * @return the result
  */
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct socket *socket = (struct socket *)sockfd;
    int ret = -1;
   
    if (SOCK_STREAM == socket->type)
    {
         ret = tcp_bind(socket->pcb, IP_ADDR_ANY, addr->sin_port); 
    }
    else if (SOCK_PACKET == socket->type)
    {
         ret = udp_bind(socket->pcb, IP_ADDR_ANY, addr->sin_port); 
    }
  
    return ret;
}

/**
  * listen - the function will make the tcp pcb viable to be connected
  *                    
  * @param sockfd  the handle of the socket
  * @param backlog the maximum socket back queue
  *
  * @return the result
  */
int listen(int sockfd, int backlog)
{
    struct socket *socket = (struct socket *)sockfd;
    int ret = -1;
   
    if (SOCK_STREAM == socket->type)
    {
         struct tcp_pcb *tcp_pcb;
         
         if ((tcp_pcb = tcp_listen(socket->pcb)))
         {
             socket->pcb = tcp_pcb;
             ret = 0; 
         }
    }
  
    return ret;
}

/**
  * connect - the function will connect loacl socket to target IP device
  *                    
  * @param sockfd   the handle of the socket
  * @param addr     structure filled of target ip address and port
  * @param addrlen  the length of the target addr structure
  *
  * @return the result
  */
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    struct socket *socket = (struct socket *)sockfd;
    int ret = -1;
   
    if (SOCK_STREAM == socket->type)
    {
         ret = tcp_connect(socket->pcb, (struct ip_addr *)&addr->sin_addr, addr->sin_port, NULL);
    }
    else if (SOCK_PACKET == socket->type)
    {
         ret = udp_connect(socket->pcb, (struct ip_addr *)&addr->sin_addr, addr->sin_port); 
         udp_recv(socket->pcb, socket_udp_recv, socket);         
    }
  
    return ret;
}

/**
  * accept - the function will accept the request of tcp client connecting 
  *                    
  * @param sockfd   the handle of the socket
  * @param addr     structure filled of tcp client ip address and port
  * @param addrlen  the length of the tcp client addr structure
  *
  * @return the result
  */
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    struct socket *socket = (struct socket *)sockfd;
    int ret = -1;
   
    if (SOCK_STREAM == socket->type)
    {
         tcp_accept(socket->pcb, socket_tcp_accept);
         tcp_arg(socket->pcb, socket);
         ret = 0;
    }
    else if (SOCK_PACKET == socket->type)
    {
         ret = 0;
    }
  
    return ret;
}

/**
  * read - the function will read bytes from the file
  *                    
  * @param fd    the file handle
  * @param buf   the recieving buffer point
  * @param count the maximum bytes to be read
  *
  * @return the result
  */
ssize_t _read(int fd, void *buf, size_t count)
{
    struct socket *socket = (struct socket *)fd;
    ssize_t size = -1;

    os_u32 data;
   
    if (mq_receive(socket->rx_mqd, (char *)&data, sizeof(os_u32), 0))
    {
        struct pbuf *pbuf = (struct pbuf *)data;
                
        size = ippkg_unpack(pbuf, (os_u8 *)buf, count);
                
        pbuf_free(pbuf);
    }
  
    return size;
}

/**
  * read - the function will write bytes to the file
  *                    
  * @param fd    the file handle
  * @param buf   the sending buffer point
  * @param count the maximum bytes to be sent
  *
  * @return the result
  */
ssize_t _write(int fd, const void *buf, size_t count)
{
    struct socket *socket = (struct socket *)fd;
    ssize_t size = -1;
   
    if (SOCK_STREAM == socket->type)
    {
        size = tcp_write(socket->pcb, buf, count, TCP_WRITE_FLAG_COPY);
    }
    else if (SOCK_PACKET == socket->type)
    {
        struct pbuf *pbuf = pbuf_alloc(PBUF_RAW, count, PBUF_POOL);
        
        if ((pbuf = ippkg_pack((os_u8 *)buf, count)) == NULL)
            return -1;
              
        size = udp_send(socket->pcb, pbuf);
    }
  
    return size;
}

/**
  * close - the function will close the file
  *                    
  * @param fd the file handle
  *
  * @return the result
  */
int _close(int fd)
{
    struct socket *socket = (struct socket *)fd;
    int ret = -1;
   
    if (SOCK_STREAM == socket->type)
    {
        ret = tcp_close(socket->pcb);
    }
    else if (SOCK_PACKET == socket->type)
    {
        udp_disconnect(socket->pcb);
        ret = 0;
    }
    
    free(socket);
  
    return ret;
}


/**
  * select - the function will select the target file handle and take it
  *
  * @param maxfdp the maximum number of the file handle
  * @param 
  */
int select(int maxfdp, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout)
{ 
    os_u32 data;
    struct socket *socket = (struct socket *)maxfdp;
  
    if (mq_receive(socket->rx_mqd, (char *)&data, sizeof(os_u32), 0))
    {
        if (!data)
        {
           *errorfds = -1;
           
           return -1;
        }
        
        *readfds = (fd_set)data;
    } 
  
    return 0;
}

/**
  * the function will init the socket
  */
err_t socket_init(void)
{
    
    return 0;
}

/**
  * the function will transform the IP addrss of string formart to structure of in_addr
  * 
  * @param string the IP addrss of string formart
  * @param addr   the in socket addr
  *
  * @return the result
  */
int inet_aton(const char *string, struct in_addr *addr)
{
    
    return 0;
}
