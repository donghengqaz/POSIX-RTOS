#ifndef _IIC_H_
#define _IIC_H_

#include "types.h"

/* structure description of iic */
struct iic
{
    int     (*start)        (struct iic *iic);
    int     (*wait)         (struct iic *iic);
    int     (*stop)         (struct iic *iic);
    int     (*send_bytes)   (struct iic *iic, os_u8 *pbuf, size_t size);
    int     (*read_bytes)   (struct iic *iic, os_u8 *pbuf, size_t size, int ack);
    
    os_u8   read_order;
    os_u8   write_order;
};

#endif
