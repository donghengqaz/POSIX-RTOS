#ifndef _IPPORT_H_
#define _IPPORT_H_

#include "list.h"
#include "mqueue.h"

#ifdef USING_IPPORT

#include "lwip/netif.h"
#include "lwip/pbuf.h"

/* maxmiun bytes of mac address */
#define IPPORT_MAC_ADDR_MAX                 6

/* maxmiun bytes of ippport name */
#define IPPORT_NAME_MAX                     8

#define PBUF_LENGTH_SET(pbuf, l) \
    pbuf->tot_len = l; \
    pbuf->len = l;
      
#define PBUF_DATA_SET(pbuf, p) \
    pbuf->payload = (void *)p
      
#define PBUF_DATA_POINT(pbuf) \
    pbuf->payload

/* discription of the ipport structure */
struct ipport
{
    /* rx function for the low level layer */    
    struct pbuf* (*hal_rx)   (void);    
    /* tx function for the low level layer */    
    err_t        (*hal_tx)   (struct pbuf *pbuf);
    /* init function for the low level layer */
    err_t        (*hal_init) (struct pbuf *pbuf[]);     
    
    /* net information for the node */
    struct netif netif;
    
    /* tx_thread and rx_thread mutex mailbox */
    mqd_t        rx_mqd;
    mqd_t        tx_mqd;
    
    list_t       list;
};
typedef struct ipport ipport_t; 

err_t ipport_system_init(void);
err_t ipport_create(struct ipport *ipport, 
                    const char *name, 
                    ip_addr_t *ipaddr,
                    ip_addr_t *netmask,
                    ip_addr_t *gw);
#else
    err_t ipport_system_init(void) { return 0; }
#endif

#endif
