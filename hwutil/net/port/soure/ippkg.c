/*
 * File         : ippkg.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-11       DongHeng        create
 */

#include "ippkg.h"   
   
#include "lwip/pbuf.h"


/*@{*/  

/*
 * the function will assemble the IP package form a block memory into the 
 * package of LWIP
 *
 * @param src the package layouting memory
 * @param len the length of the package
 *
 * @return the LWIP pbuf point
 */
struct pbuf* ippkg_pack(os_u8 *src, os_u16 len)
{
  struct pbuf *q;
  struct pbuf *p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  os_u16 offset = 0;
  
  if (NULL != p)
  {
    for (q = p; q != NULL; q = q->next)
    {
      memcpy(q->payload, &src[offset], q->len);
      offset = offset + q->len;
    }
  }
    
  return p;
}

/*
 * the function will assemble the IP package form a block memory into the 
 * package of LWIP with limited length
 *
 * @param pbuf the point of the IP package
 * @param src the IP package layouting memory
 * @param len the length of the bytes to be read
 *
 * @return read bytes
 */
size_t ippkg_unpack(struct pbuf *pbuf, os_u8 *src, size_t len)
{
  struct pbuf *q = pbuf;
  os_u16 offset = 0;
  size_t bytes;
  
  for ( ; q != NULL && len; q = q->next)
  {
    bytes = (len < q->len) ? len : q->len;
    
    memcpy(&src[offset], q->payload, bytes);
    offset += bytes;
    len -= bytes;
  }
    
  return offset;
}

/*@}*/
