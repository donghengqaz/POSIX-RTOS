#ifndef _SHELL_H_
#define _SHELL_H_

#include "list.h"
#include "init.h"

/* shell buffer paramter */
#define SHELL_BUFFER_LENGTH_MAX        64
#define SHELL_BUFFER_NUM_MAX           8
#define SHELL_PRINT_BUF_MAX            128

/* keyboard expand value */
#define KEYBOARD_EXPAND_HEAD           0x1B
#define KEYBOARD_EXPAND_TYPE           0x5b
#define KEYBOARD_EXPAND_UP             0x41
#define KEYBOARD_EXPAND_DOWN           0x42
#define KEYBOARD_EXPAND_RIGHT          0x43
#define KEYBOARD_EXPAND_LEFT           0x44

/* keyboard check result */
#define INPUT_IS_NOMAL                  0
#define INPUT_IS_EXPAND_HEAD            -1
#define INPUT_IS_EXPAND_TYPE            -2
#define INPUT_IS_DIRECTION              -3

/* shell device controller command and type */
typedef os_u32                          shell_cmd_t;

/* shell device ack */
#define SHELL_DEVICE_SET_ACK            1

#define SHELL_DEVICE_NO_ACK             0
#define SHELL_DEVICE_ACK                1

/* shell device data buffer structure */
struct shell_dev_buff
{
    char    data[SHELL_BUFFER_LENGTH_MAX];
    os_u16  offset;
};

/* shell device structure */
struct shell_dev
{
    /* recieve command charactor buffer with history */
    struct shell_dev_buff buffer[SHELL_BUFFER_NUM_MAX];
    /* point to the oldest buffer */
    os_u8                 tail_point;
    /* point to current buffer */
    os_u8                 head_point;
    /* point to newest buffer */
    os_u8                 history_point;
    
    /* the user access pririty */
    os_u8                 priority;
    
    /* for keyboard expand code */
    os_u8                 recv_status;
    
    /* check if shell device need to ack, receiving test charactor */
    os_u8                 ack;
    
    /* virtual file device port */
    int                   file;
    
    /* insert to shell device global list */
    list_t                list;                 
};

/* the type of shell function */
typedef void (*shell_func_t) (struct shell_dev *shell_dev);

/* shell command structure */
struct shell_cmd
{
    const char   *name;
    const char   *desc;
    shell_func_t func;
    os_u16       priority;
};

#if USING_SHELL
    /* shell register command structure */
    #define SHELL_CMD_EXPORT(func, desc, priority)  \
        static const char __shell##func##_func[] = #func;  \
        static const char __shell##func##_desc[] = #desc;  \
        const struct shell_cmd __shell_cmd##func __SHELL_CMD_SECTION = \
        { \
            __shell##func##_func, \
            __shell##func##_desc, \
            (shell_func_t)func, \
            priority \
        };
        
    struct shell_dev* shell_dev_find(int file);
    err_t shell_init(void);
    struct shell_dev* shell_dev_create(const char *name, err_t *err, os_u8 ack);
    ssize_t shell_printk( struct shell_dev *shell_dev, const char *fmt, ... );
    err_t shell_dev_set(struct shell_dev *shell_dev, shell_cmd_t cmd, void *arg);
#else
    #define SHELL_CMD_EXPORT(func, desc, priority)    
    
    static struct shell_dev* shell_dev_find(int file) { return NULL; }
    static err_t shell_init(void) { return 0; }   
    static struct shell_dev* shell_dev_create(const char *name, err_t *err, os_u8 ack) { return 0; }
    static ssize_t shell_printk( struct shell_dev *shell_dev, const char *fmt, ... ) { return 0; }
    static err_t shell_dev_set(struct shell_dev *shell_dev, shell_cmd_t cmd, void *arg) { return 0; }
#endif

#endif
