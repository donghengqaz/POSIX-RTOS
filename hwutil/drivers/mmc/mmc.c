/*
 * File         : mmc.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2016, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-14       DongHeng        create
 */

#include "mmc.h"
#include "mmc_cmd.h"
#include "mmc_err.h"

#include "debug.h"
#include "errno.h"

/* data exchange */
#define DUMMY_BYTE                       (0xff)  

/* card version */
#define VER_IS_SDHC                      (0x01)

/**
  * the function will send the cmd and get the rsponses
  *
  * @param mmc  the point of the mmc discription
  * @param cmd  the command
  * @param arg  the paramter of the command
  * @param crc  the crc of the command and the paramter
  *
  * @return the result or the SD_RESPONSE_FAILURE
  */
static sd_err mmc_send_cmd(struct mmc *mmc, os_u8 cmd, os_u32 arg, os_u8 crc)
{
    int i;
    os_u8 frame[6];
    sd_err resp;
    
    mmc->select();
    
    /* fill the frame to be sent */
    frame[0] = (cmd | 0x40);
    frame[1] = (os_u8)(arg >> 24);
    frame[2] = (os_u8)(arg >> 16);
    frame[3] = (os_u8)(arg >>  8);
    frame[4] = (os_u8)(arg >>  0);
    frame[5] = crc;
    /* send the frame byte by byte */
    for(i = 0; i < 6; i++)
    {
        mmc->read_write(frame[i]);
    }
    
    /* get the responses */
    i = 2000;
    do{
        resp = mmc->read_write(DUMMY_BYTE);
        i--;
    }while (resp == DUMMY_BYTE && i);
    
    mmc->release();
    
    if (i)
    {
        return resp;
    }
    else
    {
        return SD_RESPONSE_FAILURE;
    }
}

/**
  * the function will init the sdhc
  *
  * @param mmc  the point of the mmc discription
  *
  * @return the result
  */
static sd_err mmc_sdhc_init(struct mmc *mmc)
{
    sd_err err;
    int i;
    __IO os_u8 buf[10];
    
    /* get mmc voltage */
    for (i = 0; i < 5; i++)
    {  
        buf[i] = mmc->read_write(DUMMY_BYTE);
    }  
    
    mmc->release();
    mmc->read_write(DUMMY_BYTE);
    
    mmc->select();
    mmc->read_write(DUMMY_BYTE);
    mmc->read_write(DUMMY_BYTE);
    
    i = 200;
    do{
        err = mmc_send_cmd(mmc, SD_CMD_SET_EX_MODE, 0, 0xff);
        err = mmc_send_cmd(mmc, SD_CMD_SET_ACTIVE,  0x40000000, 0xff);
        i--;
    }while (err == SD_RESPONSE_FAILURE && i);
    if (err == SD_RESPONSE_FAILURE)
    {
        return SD_RESPONSE_FAILURE;
    }
    
    mmc->select();
    buf[0] = mmc_send_cmd(mmc, SD_CMD_READ_OCR, 0, 0xff);
    for (i = 1; i < 10; i++)
    {
        buf[i] = mmc->read_write(DUMMY_BYTE);
    }
    mmc->release();
    
    mmc->card_type = VER_IS_SDHC;
      
    err = SD_RESPONSE_NO_ERROR;
    
    return err;
}

/**
  * the function will open a hardwre and init it as a mmc device
  *
  * @param mmc  the point of the mmc discription
  * 
  * @return the result
  */
sd_err mmc_open(struct mmc *mmc)
{
    sd_err err;
    sd_err ver;
    int i;
  
    /* open and init the hardware */
    err = mmc->open(mmc);
    
    mmc->release();
     
    /* wakeup the mmc with 80 clock */
    for (i = 0; i < 9; i++)
    {  
        mmc->read_write(DUMMY_BYTE);
    }
    
    /* set mmc to spi mode */
    err = mmc_send_cmd(mmc, SD_CMD_GO_IDLE_STATE, 0, 0x95);
    ASSERT_KERNEL(err == SD_IN_IDLE_STATE);
    
    /* check card version and handle */
    ver = mmc_send_cmd(mmc, SD_CMD_CHECK_VER, 0x1AA, 0x87);
    ASSERT_KERNEL(ver != SD_RESPONSE_FAILURE);    
    switch(ver)
    {
        case VER_IS_SDHC : 
                ver = mmc_sdhc_init(mmc);
                break;
                
        default :
                ver = SD_RESPONSE_FAILURE;
                break;
    }
  
    return err;
}
