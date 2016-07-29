/*
 * File         : iic.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2016-01-29       DongHeng        create
 */

#include "iic.h"
#include "errno.h"

/**
  * iic_wr_reg - write a data into iic bus slave device
  *
  * @param iic iic deivce point
  * @param order iic deivce writing order
  * @param reg iic deivce writing target address
  * @param data iic deivce writing data
  *
  * @return the result
  */
int iic_write_reg (struct iic *iic, os_u8 reg, os_u8 data)
{
    if (iic->start(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &iic->write_order, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &reg, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &data, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->stop(iic))
        return -EINVAL;
  
    return 0;
}

/**
  * iic_wr_reg - read a data form iic bus slave device
  *
  * @param iic iic deivce point
  * @param order iic deivce reading order
  * @param reg iic deivce reading target address
  * @param data iic deivce reading data
  *
  * @return the result
  */
int iic_read_reg (struct iic *iic, os_u8 reg, os_u8 data)
{
    if (iic->start(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &iic->write_order, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &reg, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->start(iic))
        return -EINVAL;
    
    if (iic->send_bytes(iic, &iic->read_order, 1) <= 0)
        return -EINVAL;
    
    if (iic->wait(iic))
        return -EINVAL;
    
    if (iic->read_bytes(iic, &data, 1, 0) <= 0)
        return -EINVAL;
    
    if (iic->stop(iic))
        return -EINVAL;
  
    return 0;
}
