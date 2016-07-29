/*
 * File         : ipport.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-25       DongHeng        create
 */

#include "ipport.h"
#include "ipthread.h"

#include "debug.h"
#include "stdlib.h"
#include "sched.h"

#include "shell.h"

#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"



#define IPPORT_IPADDR(ipport) \
    ipport->netif.ip_addr.addr
     
#define IPPORT_NETMASK(ipport) \
    ipport->netif.netmask.addr
      
#define IPPORT_GW(ipport) \
    ipport->netif.gw.addr

/*@{*/ 

static list_t ipport_list;

/*@}*/

/*@{*/  

/**
  * the function is the port for ip-stack output the data
  *
  * @param netif the net information of LWIP
  * @param p the data point with the structure of LWIP
  * 
  * @return the result of outputing the data
  */
static err_t ipport_output(struct netif *netif, struct pbuf *p)
{
  struct ipport *ipport = (struct ipport *)netif->state;
  os_u32 event;
    
  ipport->hal_tx(p);
   
  mq_receive(ipport->tx_mqd, (char *)&event, 1, 0);
    
  return ERR_OK;
}

/**
  * the function is that ip-stack init the hardware port
  *
  * @param netif the net information of LWIP
  * 
  * @return the result of initing the hardware port
  */
static err_t ipport_init(struct netif *netif)
{
  struct ipport *ipport = (struct ipport *)netif->state;
  int i;
  struct pbuf *pbuf[HW_ETH_RX_BUFFER_NUM_MAX];
  
#if LWIP_NETIF_HOSTNAME
  netif->hostname = "POSIX-RTOS";
#endif

  NETIF_INIT_SNMP( netif, snmp_ifType_ethernet_csmacd, 100000000 );
  
  netif->output  = etharp_output;
  netif->linkoutput = ipport_output;

  netif->hwaddr_len = ETHARP_HWADDR_LEN;
  
  netif->mtu = 1500;
  
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

  for (i = 0; i < HW_ETH_RX_BUFFER_NUM_MAX; i++)
       pbuf[i] = pbuf_alloc(PBUF_RAW, HW_ETH_RX_BUFFER_LENGTH_MAX, PBUF_POOL);
    
  ipport->hal_init(pbuf);

  return ERR_OK;
}
   
/**
  * the function will init the ip stack with the physics port
  *
  * @param port the point of the physics port
  * @param name the name of the hardware port
  * @param ipaddr the ip address
  * @param netmask the netmask of net ip
  * @param gw the gw address
  *
  * @return the result of create a ipport device of hardware
  */   
err_t ipport_create(struct ipport *ipport, 
                    const char *name, 
                    ip_addr_t *ipaddr,
                    ip_addr_t *netmask,
                    ip_addr_t *gw)
{
  os_u32 temp = sched_suspend();
  struct mq_attr mq_attr;
  
  memp_init();
  
  mq_attr.mq_maxmsg  = 16;
  mq_attr.mq_msgsize = 4;
  
  ipport->rx_mqd = mq_open("input", O_CREAT | O_RDWR, O_CREAT | O_RDWR, &mq_attr);
  ipport->tx_mqd = mq_open("input", O_CREAT | O_RDWR, O_CREAT | O_RDWR, &mq_attr);
    
  netif_add(&ipport->netif, ipaddr, netmask, gw, ipport, ipport_init, &ethernet_input);
  netif_set_default(&ipport->netif);
#if (LWIP_DHCP == 1)
  dhcp_start(&ipport->netif);
#endif
  netif_set_up(&ipport->netif);
    
  ipthrtead_init(ipport, name);
    
  list_insert_tail(&ipport_list, &ipport->list);
    
  memcpy(ipport->netif.name, name, 2);
  
  sched_recover(temp);
    
  return 0;
}

/**
  * the function will init the ipport system
  *
  * @return the result
  */ 
err_t ipport_system_init(void)
{
    list_init(&ipport_list);
    
    return 0;
}

static void ifconfig(struct shell_dev *shell_dev)
{
  struct ipport *ipport;
  char *str_num = "";
    
  LIST_FOR_EACH_ENTRY(ipport, 
                      &ipport_list, 
                      struct ipport,
                      list )
  {
    shell_printk(shell_dev, "\r\n%c%c%s -- %d.%d.%d.%d", ipport->netif.name[0], 
                                                           ipport->netif.name[1],
                                                           str_num,
                                                           ITOC_RS0(IPPORT_IPADDR(ipport)),
                                                           ITOC_RS0(IPPORT_IPADDR(ipport)),
                                                           ITOC_RS0(IPPORT_IPADDR(ipport)),
                                                           ITOC_RS0(IPPORT_IPADDR(ipport)));
        
    shell_printk(shell_dev, "\r\n%-2s    %d.%d.%d.%d", str_num, 
                                                       ITOC_RS0(IPPORT_NETMASK(ipport)),
                                                       ITOC_RS0(IPPORT_NETMASK(ipport)),
                                                       ITOC_RS0(IPPORT_NETMASK(ipport)),
                                                       ITOC_RS0(IPPORT_NETMASK(ipport)));
        
    shell_printk(shell_dev, "\r\n%-2s    %d.%d.%d.%d\r\n", str_num, 
                                                           ITOC_RS0(IPPORT_GW(ipport)),
                                                           ITOC_RS0(IPPORT_GW(ipport)),
                                                           ITOC_RS0(IPPORT_GW(ipport)),
                                                           ITOC_RS0(IPPORT_GW(ipport))); 
  }
}
SHELL_CMD_EXPORT(ifconfig, show all ipport ip address, 1);

/*@}*/
