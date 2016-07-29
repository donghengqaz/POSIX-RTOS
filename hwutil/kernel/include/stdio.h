#ifndef _STDIO_H_
#define _STDIO_H_

#include "rtos.h"

typedef char* va_list;
#ifndef __va_rounded_size
    #define __va_rounded_size(TYPE) (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#endif
#ifndef va_start
    #define va_start(AP, LASTARG)   (AP = ((char *)& (LASTARG) + __va_rounded_size(LASTARG)))
#endif
#ifndef va_arg
    #define va_arg(AP, TYPE)        (AP += __va_rounded_size(TYPE), *((TYPE *)(AP - __va_rounded_size(TYPE))))
#endif
#ifndef va_end
    #define va_end(AP)              (AP = (va_list)0)
#endif

/*
 *
 */
#define MAX_DIGITS     15
/******************************************************************************/

/*
 *
 */
#define ZEROPAD     (1)               // Pad with zero
#define SIGN        (2)               // Unsigned/signed long
#define PLUS        (4)               // Show plus
#define SPACE       (8)               // Space if plus
#define LEFT        (16)              // Left justified
#define SPECIAL     (32)              // 0x
#define LARGE       (64)              // Use 'ABCDEF' instead of 'abcdef'
#define LOW_DIGITS   "0123456789abcdefghijklmnopqrstuvwxyz"
#define UPPER_DIGITS "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
/******************************************************************************/

/*
 *
 */
#define is_digit(c) ((c) >= '0' && (c) <= '9')
#define is_space(a) ( (unsigned long)(a - 9) < 5u || ' ' == a )
#define DOUBLE_ZERO ((double)0.1)
#define IS_DOUBLE_ZERO(D) (D <= DOUBLE_ZERO && D >= -DOUBLE_ZERO)
/******************************************************************************/

int vsprintf(char *buf, const char *fmt, va_list args);
int sprintf(char *buf, const char *fmt, ...);
ssize_t printk(const char *fmt, ...);
ssize_t printf(const char *fmt, ...);

#endif
