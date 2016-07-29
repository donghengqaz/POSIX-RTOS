#ifndef _TYPES_H_
#define _TYPES_H_

/* define the "C language advanced keyword" of the target IDE */
#if defined (__IAR_SYSTEMS_ICC__)
    #define NO_INIT                         __no_init
    #define SECTION(x)                      @ x
    #define UNUSED(x)                       ((void)(x))
    #define USED
    #define PRAGMA(x)                       _Pragma(#x)
    #define ALIGNMENT(n)                    PRAGMA(data_alignment=n)
    #define WEAK                            __weak
    #define STATIC                          static
    #define INLINE                          STATIC inline
    #define RTT_API
    #define RESTRICT                        restrict
#else
    #define NO_INIT
    #define SECTION(x)
    #define UNUSED(x)
    #define USED
    #define PRAGMA(x)
    #define ALIGNMENT(n)
    #define WEAK
    #define STATIC
    #define INLINE
    #define RTT_API
    #define RESTRICT
#endif    
/******************************************************************************/

/* define the NULL */
#ifndef NULL
    #define NULL                            0
#endif
/******************************************************************************/

/* define the os data type */
typedef unsigned char                       os_u8;
typedef unsigned short                      os_u16;
typedef unsigned int                        os_u32;
typedef signed char                         os_s8;
typedef signed short                        os_s16;
typedef signed int                          os_s32;
typedef signed int                          err_t;
typedef signed int                          loff_t;

typedef unsigned int                        size_t;
typedef signed int                          ssize_t;
#ifndef __cplusplus
    typedef enum {false = 0, true = 1}      bool;
    typedef bool                            BOOL;
#endif
typedef unsigned long long                  os_u64;
typedef signed long long                    os_s64;
/******************************************************************************/

/* define the hardware data type */
typedef unsigned int                        phys_addr_t;
typedef unsigned int                        phys_reg_t;

/* define the handle of  the thread */
typedef int                                 pthread_t;
typedef int                                 pid_t;

/* define the handle of  the timer */
typedef int                                 timer_t;

/* define the date type of the time */
typedef unsigned int                        time_t;

/* define the handle of the message queue */
typedef int                                 mqd_t ;

typedef int                                 mode_t;
/******************************************************************************/
    
/* define the data type attribute */
#define OS_U8_MAX                           (255)
#define OS_U8_MIN                           (0)

#define OS_S8_MAX                           (127)
#define OS_S8_MIN                           (-127)

#define OS_U16_MAX                          (65535)
#define OS_U16_MIN                          (0)

#define OS_S16_MAX                          (32767)
#define OS_S16_MIN                          (-32767)

#define OS_U32_MAX                          (4294967295)
#define OS_U32_MIN                          (0)

#define OS_S32_MAX                          (2147483647)
#define OS_S32_MIN                          (-2147483647)
/******************************************************************************/
    
/* define the IO handle declare */
#ifndef __IO
#define __IO                                volatile
#endif

#ifndef __I  
#define __I                                 const
#endif

#ifndef __O
#define __O                                 volatile
#endif
    
#define OS_RW                               __IO
#define OS_RO                               __I
#define OS_WO                               __O
/******************************************************************************/
    
/* define the memory address align */  
#define __ALIGN(size, align)                (((size) + (align) - 1) & ~((align) - 1))

#ifndef PHY_ADDR_REMAP
    #define PHY_ADDR_REMAP(x)               x
#endif

#define ARRAY_SIZE(x)                       sizeof(x) / sizeof(x[0])
/******************************************************************************/

/*
 *  the definition of pthread attribute and data type  
 */
typedef os_u8                               pthread_type_t;
typedef os_u8                               pthread_flag_t;

struct pthread_attr
{
    pthread_type_t          type;
    
    /* thread tick slice */
    os_u8                   init_ticks;

    /* thread priority */
    os_u8                   init_prio;
    
    /* thread stack info */
    char                    *stk_addr;
    size_t                  stk_size;
};
typedef struct pthread_attr pthread_attr_t;


/* type of timer source clock */
enum clockid
{
    CLOCK_REALTIME = 0,
  
};
typedef enum clockid clockid_t;

#endif
