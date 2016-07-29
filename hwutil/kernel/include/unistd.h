#ifndef _UNISTD_H_
#define _UNISTD_H_

#include "rtos.h"
#include "list.h"

enum stdobj_type
{
    STDOBJ_DEVICE = 0,
    STDOBJ_SOCKET = 1,
    STDOBJ_FS     = 2
};

#define STDOBJ_NUM_MAX          (STDOBJ_FS + 1)

#define STDOBJ_NAME_MAX         12

/******************************************************************************/

#define O_RDONLY                (1UL << 0)
#define O_WRONLY                (1UL << 1)
#define O_RDWR                  (1UL << 2)
#define O_CREAT                 (1UL << 3)
#define O_NONBLOCK              (1UL << 4)

#define SEEK_SET                (0)
#define SEEK_CUR                (1)
#define SEEK_END                (2)
/******************************************************************************/

struct stdops
{
    int                         type;
    
    int                         flag;
    int                         open_flag;
  
    /**  I/O opearion  **/
    err_t   (*open)             (struct stdops *ops);
    ssize_t (*read)             (struct stdops *ops, char __user *buffer, size_t size, loff_t loff);
    ssize_t (*write)            (struct stdops *ops, const char __user *buffer, size_t size, loff_t loff);
    err_t   (*release)          (struct stdops *ops);
    err_t   (*control)          (struct stdops *ops, int cmd, void *paramer);
    loff_t  (*lseek)            (struct stdops *ops, loff_t loff, int from);
    
    ssize_t (*aio_read)         (struct stdops *ops, char __user *buffer, size_t size, loff_t loff);
    ssize_t (*aio_write)        (struct stdops *ops, const char __user *buffer, size_t size, loff_t loff);
    
    /**  call back function  **/
    void    (*sync)             (struct stdops *ops);
    void    (*rx_start)         (struct stdops *ops, size_t size);
    void    (*tx_end)           (struct stdops *ops, const char __user *buffer);
    
    /**  system power management  **/
    err_t   (*suspend)          (struct stdops *ops);
    err_t   (*resume)           (struct stdops *ops);
    
    /**  help function  **/
    err_t   (*help)             (struct stdops *ops);
    err_t   (*info)             (struct stdops *ops);
    
    /** user private data point **/
    void                        *priv;
};

int stdobj_create (enum stdobj_type, const char *name, struct stdops *stdops);

int access (const char *name, int flag);
int close (int fildes);
ssize_t read (int fildes, void *buf, size_t nbyte);
ssize_t write (int fildes, const void *buf, size_t nbyte);
loff_t lseek (int fildes, loff_t loff, int from);
int ioctl (int fildes, int request, ...);
int lock (int fildes, loff_t offset, size_t length);
int unlock (int fildes, loff_t offset, size_t length);

unsigned int msleep(unsigned int milliseconds);
unsigned int sleep(unsigned int seconds);

#define device_register(name, stdops) \
            stdobj_create(STDOBJ_DEVICE, name, stdops)

#endif
