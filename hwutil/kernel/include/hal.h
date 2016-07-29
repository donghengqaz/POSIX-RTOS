#ifndef _HAL_H_
#define _HAL_H_

#include "init.h"

err_t hal_init(void);

struct hal_func
{
#define HAL_ORDER_MAX       16
  
    const char        *name;
    const char        *desc;
    err_t             (*init) (void);
    size_t            order;
};
typedef struct hal_func hal_func_t;

#define HAL_FUNC_EXPORT(func, desc, order)  \
    static const char __hal##func##_func[] = #func;  \
    static const char __hal##func##_desc[] = #desc;  \
    const struct hal_func __hal##func __HAL_SECTION = \
    { \
        __hal##func##_func, \
        __hal##func##_desc, \
        func, \
        order \
    };

#endif
