#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "stdio.h"

/* assert signal result */
#define ASSERT_KERNEL(signal)                                                         \
{                                                                                     \
    if (!(signal))                                                                    \
    {                                                                                 \
        volatile bool dummy = true;                                                   \
        printk("(%s) assert failed at %s:%d .\r\n", #signal, __FUNCTION__, __LINE__); \
        while(true == dummy);                                                         \
    }                                                                                 \
}

#endif
