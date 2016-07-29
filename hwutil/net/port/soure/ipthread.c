/*
 * File         : ipthread.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-11-26       DongHeng        create
 */

#include "ipthread.h"
#include "ippkg.h"
   
#include "pthread.h"
#include "debug.h"
#include "time.h"

#include "ipport_def.h" 
   
#include "lwip/memp.h"
#include "lwip/mem.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "lwip/udp.h"
#include "lwip/dhcp.h"
#include "netif/etharp.h"   


/*@{*/ 

/*
 *
 */
static void* ip_rx_thread_entry(void *p)
{
    ipport_t *ipport = (ipport_t *)p;
    os_u32 event;
    struct pbuf *recv_pbuf;
    
    while( 1 )
    {
        if(mq_receive(ipport->rx_mqd, (char *)event, sizeof(os_u32), 0))
        {
            if ((recv_pbuf = ipport->hal_rx()) != NULL)
            {        
                if (ipport->netif.input(recv_pbuf, &ipport->netif) != ERR_OK)
                {
                    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
                    pbuf_free(recv_pbuf);
                    recv_pbuf = NULL;
                }
            }
        }
    }
}

/*
 * the function will handle the predic event of the ip stack
 *
 * @param p the time buffer point
 */ 
static void ipstk_predic_handle(void *p)
{
    unsigned int *time = (unsigned int *)p;

    time[0] = time[0] + 1;
    
    /* tcp periodic handle every 250ms */
    tcp_tmr();

    /* arp periodic handle every 5s */
    if (!(time[0] % (ARP_TMR_INTERVAL / TCP_TMR_INTERVAL)))
        etharp_tmr();

#if LWIP_DHCP
    /* dhcp fine periodic handle every 500ms */
    if (!(time[0] % (DHCP_FINE_TIMER_MSECS / TCP_TMR_INTERVAL)))
        dhcp_fine_tmr();
    /* dhcp coarse periodic handle every 60s */
    if (time[0] >= (DHCP_COARSE_TIMER_MSECS / TCP_TMR_INTERVAL))
        dhcp_coarse_tmr(), time[0] = 0;
#endif
}

/*
 * the function will init the ipthread
 *
 * @param ipport the point of ip stack
 * @param name the name of the hardware port
 *
 * @return the result of init the ip thread
 */
err_t ipthrtead_init( ipport_t *ipport, const char *name )
{
    int err;
    static unsigned int time;
    int tid;
    pthread_attr_t attr;
    sched_param_t app_sched_param =
      SCHED_PARAM_INIT(PTHREAD_TYPE_KERNEL,
                       IPTHREAD_RX_TICKS,
                       IPTHREAD_RX_PRIORITY);
    
    pthread_attr_setschedparam(&attr, &app_sched_param);
    pthread_attr_setstacksize(&attr, IPTHREAD_RX_STACK_SIZE);
    
    err = pthread_create(&tid,
                         &attr,
                         ip_rx_thread_entry,
                         ipport);
    ASSERT_KERNEL(!err);
/*    
    timer = timer_create( name,
                          ipstk_predic_handle,
                          &time,
                          TCP_TMR_INTERVAL,
                          TIMER_CYCLE,
                          &err );
    ASSERT_KERNEL( err == EOK );
    timer_start( timer );
*/  
    return err;
}

/*@}*/

