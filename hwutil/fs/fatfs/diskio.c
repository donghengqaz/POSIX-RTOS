/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "string.h"

NO_INIT static struct diskio_port diskio_port[DISKIO_DRIVER_MAX];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

err_t diskio_port_init(void)
{
    int i;
  
    for (i = 0; i < DISKIO_DRIVER_MAX; i++)
    {
        diskio_port[i].used = false;
        diskio_port[i].inited = false;
    }
    
    return 0;
}

err_t diskio_port_register (BYTE pdrv, struct diskio_port *port)
{
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        if (true == diskio_port[pdrv].used)
        {
            return STA_NOINIT;
        }
        memcpy(&diskio_port[pdrv], port, sizeof(struct diskio_port));
        diskio_port[pdrv].used = true;
    }

    return STA_NOINIT;
}

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{       
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        return diskio_port[pdrv].disk_status();
    }

    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    DSTATUS ret = RES_OK;
  
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        if (false == diskio_port[pdrv].inited)
        {
            diskio_port[pdrv].inited = true;
            ret =  diskio_port[pdrv].disk_init();
        }
    }

    return ret;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        return diskio_port[pdrv].disk_read(buff, sector, count);
    }

    return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        return diskio_port[pdrv].disk_write(buff, sector, count);
    }

    return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    if (pdrv < DISKIO_DRIVER_MAX)
    {
        return diskio_port[pdrv].disk_ioctl(cmd, buff);
    }

    return RES_PARERR;
}
#endif
