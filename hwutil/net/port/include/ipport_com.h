#ifndef _IPPORT_COM_H_
#define _IPPORT_COM_H_

#include "rtos.h"
#include "shell.h"

struct ipport_com
{
    struct shell_dev *shell_dev;
    
    void             *arg;
};

err_t ipport_com_configuration(void);

#endif
