/*
 * File         : shell.c
 * This file is part of POSIX-RTOS
 * COPYRIGHT (C) 2015 - 2018, DongHeng
 * 
 * Change Logs:
 * DATA             Author          Note
 * 2015-12-14       DongHeng        create
 */

#include "shell.h" 
   
#if USING_SHELL

#include "errno.h"
#include "debug.h"
#include "stdlib.h"
#include "pthread.h"  
#include "string.h"
#include "stdio.h"
#include "unistd.h"
#include "init.h"
   

#if 0
    #define SHELL_DEBUG( x )     printk x
#else
    #define SHELL_DEBUG( x )
#endif

/*@{*/ 

#define SHELL_THREAD_STACK_SIZE          512
#define SHELL_THREAD_PRIORITY            22
#define SHELL_THREAD_TICKS               2

#define SHELL_BUFFER_POINT(shell_dev) \
    shell_dev->head_point  
      
#define SHELL_BUFFER_HISTORY_POINT(shell_dev) \
    shell_dev->history_point
      
#define SHELL_BUFFER_TAIL_POINT(shell_dev) \
    shell_dev->tail_point
      
#define SHELL_POINT_IS_END(shell_dev) \
    (SHELL_BUFFER_POINT(shell_dev) == SHELL_BUFFER_TAIL_POINT(shell_dev))
   
#define SHELL_BUFFER(shell_dev) \
    shell_dev->buffer[SHELL_BUFFER_POINT(shell_dev)].data
      
#define SHELL_BUFFER_OFFSET(shell_dev) \
    shell_dev->buffer[SHELL_BUFFER_POINT(shell_dev)].offset

#define SHELL_FREE_BUFFER(shell_dev) \
    SHELL_BUFFER(shell_dev) + SHELL_BUFFER_OFFSET(shell_dev)
    
#define SHELL_FREE_BUFFER_LENGTH(shell_dev) \
    (SHELL_BUFFER_LENGTH_MAX - 1 - SHELL_BUFFER_OFFSET(shell_dev))
      
#define SHELL_BUFFER_OFFSET_ADD(shell_dev, num) \
    SHELL_BUFFER_OFFSET(shell_dev) += num;
 
#define SHELL_BUFFER_IS_FULL(shell_dev) \
    (SHELL_BUFFER_OFFSET(shell_dev) >= (SHELL_BUFFER_LENGTH_MAX - 2))

#define SHELL_BUFFER_IS_EMPTY(shell_dev) \
    (0 == SHELL_BUFFER_OFFSET(shell_dev))
      
#define SHELL_BUFFER_INSERT_CH(shell_dev, ch) \
    SHELL_BUFFER(shell_dev)[SHELL_BUFFER_OFFSET(shell_dev)++] = ch
      
#define SHELL_BUFFER_DELETE_CH(shell_dev) \
    --SHELL_BUFFER_OFFSET(shell_dev);

#define SHELL_DEV_WRITE(shell_dev, buffer, size) \
    write(shell_dev->file, buffer, size)
      
#define SHELL_BUFFER_CLEAR_LAST(shell_dev) \
    SHELL_BUFFER_OFFSET(shell_dev) = 0
      
#define SHELL_BUFFER_POINT_INC(shell_dev) \
    SHELL_BUFFER_POINT(shell_dev) = (SHELL_BUFFER_POINT(shell_dev) + 1) % SHELL_BUFFER_NUM_MAX; \
    if (SHELL_POINT_IS_END(shell_dev)) \
        SHELL_BUFFER_TAIL_POINT(shell_dev) = (SHELL_BUFFER_TAIL_POINT(shell_dev) + 1) % SHELL_BUFFER_NUM_MAX; \
    SHELL_BUFFER_HISTORY_POINT(shell_dev) = SHELL_BUFFER_POINT(shell_dev)

#define SHELL_BUFFER_CURRENT_POINT_INC(shell_dev) \
    if (SHELL_BUFFER_HISTORY_POINT(shell_dev) != SHELL_BUFFER_POINT(shell_dev)) \
        SHELL_BUFFER_POINT(shell_dev) = (SHELL_BUFFER_POINT(shell_dev) + 1) % SHELL_BUFFER_NUM_MAX;
      
#define SHELL_BUFFER_CURRENT_POINT_SUB(shell_dev) \
    if (SHELL_BUFFER_POINT(shell_dev) != SHELL_BUFFER_TAIL_POINT(shell_dev)) \
    { \
        if (SHELL_BUFFER_POINT(shell_dev)) \
            --SHELL_BUFFER_POINT(shell_dev); \
        else \
            SHELL_BUFFER_POINT(shell_dev) =  SHELL_BUFFER_NUM_MAX - 1; \
    }
       
#define SHELL_BUFFER_PRINT(shell_dev) \
    SHELL_DEV_WRITE(shell_dev, SHELL_BUFFER(shell_dev), SHELL_BUFFER_OFFSET(shell_dev))
  

#define SHELL_DEV_NEED_ACK(shell_dev) \
    (SHELL_DEVICE_ACK == shell_dev->ack)

/* for shell device check */      
NO_INIT static list_t shell_dev_list KERNEL_SECTION;
      
/*@{*/ 

/*
 * the function will find the shell device through the name
 *
 * @param name the name of the shell device to be found
 *
 * @return the shell device point
 */
struct shell_dev* shell_dev_find(int file)
{
    struct shell_dev *shell_dev;
    
    LIST_FOR_EACH_ENTRY(shell_dev,
                        &shell_dev_list, 
                        struct shell_dev,
                        list)
    {   
        if (shell_dev->file == file)
            return shell_dev;
    }
  
    return NULL;
}

/**
  * the function will check the input charactor whether it is the keyboard expand value
  *
  * @param shell_dev the shell device point
  * @param ch the input charactor
  *
  * @return the result
  */
INLINE int shell_recv_keybaord_expand(struct shell_dev *shell_dev, char ch)
{
    /* the status of recieve the "KEYBOARD_EXPAND_HEAD" charactor */
    if (!shell_dev->recv_status)
    {
        if (KEYBOARD_EXPAND_HEAD == ch)
        {
            shell_dev->recv_status = KEYBOARD_EXPAND_HEAD;
            
            return INPUT_IS_EXPAND_HEAD;
        }
    }
    /* the status of recieve the "KEYBOARD_EXPAND_HEAD" charactor */
    else if (KEYBOARD_EXPAND_HEAD == shell_dev->recv_status )
    {
        if (KEYBOARD_EXPAND_TYPE == ch)
        {
            shell_dev->recv_status = KEYBOARD_EXPAND_TYPE;
            
            return INPUT_IS_EXPAND_TYPE;
        }
        else
        {
            shell_dev->recv_status = 0;
            
            return INPUT_IS_NOMAL;
        }
    }
    /* the status of recieve the "direction" charactor */
    else if (KEYBOARD_EXPAND_TYPE == shell_dev->recv_status )
    {
        shell_dev->recv_status = 0;
      
        if (KEYBOARD_EXPAND_UP == ch 
            || KEYBOARD_EXPAND_DOWN == ch
            || KEYBOARD_EXPAND_RIGHT == ch
            || KEYBOARD_EXPAND_LEFT == ch)
           return INPUT_IS_DIRECTION;
        else
           return INPUT_IS_NOMAL;
    }
    
    return INPUT_IS_NOMAL;
}
      
/**
  * the function will find the target shell command through the input string
  *
  * @param shell_dev the shell device point
  *
  * @return the shell command structure point
  */
INLINE struct shell_cmd* shell_cmd_find(struct shell_dev *shell_dev)
{
    struct shell_cmd *pshell_cmd = SHELL_CMD_START_ADDR;
    os_u32 shell_cmd_num = SHELL_CMD_NUM;
    os_u32 i;
    
    for (i = 0; i < shell_cmd_num; i++)
    {
        if (strstr(SHELL_BUFFER(shell_dev), pshell_cmd->name) == SHELL_BUFFER(shell_dev))
        {
            return pshell_cmd;
        }
        pshell_cmd++;
    }
    
    return NULL;
}

/**
  * the function is the entry of the shell script recieve and handle
  *
  * @param param the shell device point
  */
void* shell_thread_entry(void *param)
{
    struct shell_dev *shell_dev = (struct shell_dev *)param;
    char ch;
    int ch_type;
    struct shell_cmd *shell_cmd;

    while(1)
    {
        /* read a character from device */
        if (read(shell_dev->file, &ch, 1) > 0)
        {
            const char *usr_log[] = 
            {
                "",
                "Usr >:",
                "Admin >:",
                "Master >:",
                "Root >:"
            };
          
            SHELL_DEBUG(("shell recv [0x%x]\r\n", ch));
          
            ch_type = shell_recv_keybaord_expand(shell_dev, ch);
            if (ch_type == INPUT_IS_NOMAL)
            {
                /* calssify the charactor */
                ch_type = ch_classify(ch);
            }
            
            switch (ch_type)
            {
                case CH_IS_TEXT:
                    /* insert text to buffer if buffer is not full */
                    if (!SHELL_BUFFER_IS_FULL(shell_dev))
                    {
                        SHELL_BUFFER_INSERT_CH(shell_dev, ch);
                        if (SHELL_DEV_NEED_ACK(shell_dev))
                            SHELL_DEV_WRITE(shell_dev, &ch, 1);
                    }
                    break;
                    
                case CH_IS_BACKSPACE:
                        /* delete a character if buffer is not empty */
                    if (!SHELL_BUFFER_IS_EMPTY(shell_dev))
                    {
                        const char buffer[] = {0x08, ' ', 0x08};
                      
                        SHELL_BUFFER_DELETE_CH(shell_dev);
                        if (SHELL_DEV_NEED_ACK(shell_dev))
                            SHELL_DEV_WRITE(shell_dev, buffer, sizeof(buffer));
                    }
                    break;
                  
                case CH_IS_ENTER:
                        /* when input character is enter, fill '\0' to end the string and handle */
                    if (!SHELL_BUFFER_IS_FULL(shell_dev))
                    {                     
                        const char buffer[] = "\r\n"; 
                      
                        SHELL_BUFFER_INSERT_CH(shell_dev, '\0');
                        /* find the shell command */
                        if ((shell_cmd = shell_cmd_find(shell_dev)) != NULL)
                        {
                            /* check the owner is high priority to handle it*/
                            if (shell_dev->priority >= shell_cmd->priority)
                            {
                                shell_cmd->func(shell_dev);
                            }
                        }
                        
                        SHELL_DEV_WRITE(shell_dev, buffer, sizeof(buffer));
                        SHELL_DEV_WRITE(shell_dev, usr_log[shell_dev->priority], strlen(usr_log[shell_dev->priority]));                       
                        SHELL_BUFFER_POINT_INC(shell_dev);
                        SHELL_BUFFER_CLEAR_LAST(shell_dev);
                    }
                    break;
                    
                case INPUT_IS_DIRECTION:
                    {
                        const char buffer[] = "\r";
                        int print = 0;
                      
                        if (KEYBOARD_EXPAND_UP == ch)
                        {
                            SHELL_BUFFER_CURRENT_POINT_SUB(shell_dev);
                            print = 1;
                        }
                        else if (KEYBOARD_EXPAND_DOWN == ch)
                        {
                            SHELL_BUFFER_CURRENT_POINT_INC(shell_dev);
                            print = 1;
                        }
                        else if (KEYBOARD_EXPAND_RIGHT == ch)
                        {

                        }
                        else if (KEYBOARD_EXPAND_LEFT == ch)
                        {

                        }
                        if (print)
                        {
                            SHELL_DEV_WRITE(shell_dev, buffer, sizeof(buffer));
                            SHELL_DEV_WRITE(shell_dev, usr_log[shell_dev->priority], strlen(usr_log[shell_dev->priority])); 
                            SHELL_BUFFER_PRINT(shell_dev);
                        }
                    }
                    break;
                  
                default:
                        /* return when inputing is others */
                    break;
            }
        }
    }
}

/**
  * the function will create a shell device for usr device port
  *
  * @param name the name of the device port
  *
  * @return the result
  */
struct shell_dev* shell_dev_create(const char *name, err_t *err, os_u8 ack)
{
    struct shell_dev *shell_dev;
    int file;
    int __err;
    int tid;
    pthread_attr_t attr;
    char buffer[32] = "/dev/";
    const char *dir = "/dev/";
    sched_param_t shell_sched_param =
      SCHED_PARAM_INIT(PTHREAD_TYPE_KERNEL,
                       SHELL_THREAD_TICKS,
                       SHELL_THREAD_PRIORITY);
  
    memcpy(buffer + strlen(dir), name, strlen(name) + 1);
    if ((file = access(buffer, O_RDWR)) <= 0)
        return NULL;
    
    if (shell_dev_find(file))
    {
        *err = -EINVAL;
        return NULL;
    }
    
    shell_dev = calloc(sizeof(struct shell_dev));
    ASSERT_KERNEL(shell_dev != NULL);
    
    shell_dev->priority = 1;
    shell_dev->file     = file;
    shell_dev->ack      = ack;
        
    pthread_attr_setschedparam(&attr, &shell_sched_param);
    pthread_attr_setstacksize(&attr, SHELL_THREAD_STACK_SIZE);
    
    __err = pthread_create(&tid,
                           &attr,
                           shell_thread_entry,
                           shell_dev);
    ASSERT_KERNEL(!__err);

    list_insert_tail(&shell_dev_list, &shell_dev->list);
    
    *err = __err;
    
    return shell_dev;
}

/**
  * the function will set the shell device
  *
  * @param shell_dev the shell device point
  * @param cmd shell setting command
  * @param arg the paramter of the command
  *
  * @return the result
  */
err_t shell_dev_set(struct shell_dev *shell_dev, shell_cmd_t cmd, void *arg)
{
    /* check the command */
    switch (cmd)
    {
        /* set the shell device ack */
        case SHELL_DEVICE_SET_ACK:
            {
                shell_dev->ack = *(os_u8 *)arg;               
                break;
            }
        
        default:
            break;
    }
    
    return 0;
}

/**
  * the function will init the shell function
  *
  * @return the result
  */
err_t shell_init(void)
{  
    list_init(&shell_dev_list);
  
    return 0;  
}

/**
 * This function will print the information of kernel
 *
 * @param fmt the string should be printed
 * @param shell_dev the shell device point
 *
 * @return the number of printed character
 */
ssize_t shell_printk( struct shell_dev *shell_dev, const char *fmt, ... )
{
    va_list args;
    ssize_t send_bytes;
    char shell_print_buf[SHELL_PRINT_BUF_MAX];
    
#if _ATOM_
    phys_reg_t temp;  
    
    temp = hw_interrupt_suspend();
#endif
    va_start( args, fmt );
    send_bytes = vsprintf( shell_print_buf, fmt, args );
    va_end( args );
    
    SHELL_DEV_WRITE(shell_dev, shell_print_buf, send_bytes);
#if _ATOM_    
    hw_interrupt_recover( temp );
#endif
    
    return send_bytes;
}



/**
  * the function will init the shell function
  *
  * @param shell_dev the shell device point
  */
static void help(struct shell_dev *shell_dev)
{
    struct shell_cmd *shell_cmd = SHELL_CMD_START_ADDR;
    os_u32 shell_cmd_num = SHELL_CMD_NUM;
    os_u32 i;
    
    shell_printk(shell_dev, "\r\n");
  
    for (i = 0; i < shell_cmd_num; i++)
    {
        if (shell_dev->priority >= shell_cmd->priority)
        {
            shell_printk(shell_dev, "\r\n%-20s -- %s", shell_cmd->name, shell_cmd->desc);
        }
        shell_cmd++;
    }
    
    shell_printk(shell_dev, "\r\n");
}
SHELL_CMD_EXPORT(help, show all shell command and its description, 1);

#else

struct shell_dev* shell_dev_find(int file)
{
    return NULL;
}

err_t shell_init(void)
{
    return 0;
}

struct shell_dev* shell_dev_create(const char *name, err_t *err, os_u8 ack)
{
    return 0;
}

ssize_t shell_printk(struct shell_dev *shell_dev, const char *fmt, ...)
{
    return 0;
}

err_t shell_dev_set(struct shell_dev *shell_dev, shell_cmd_t cmd, void *arg)
{
    return 0;
}

#endif

/*@}*/
