#ifndef _INIT_H_
#define _INIT_H_

#include "rtos.h"

#if defined (__ICCARM__)
	/*shell command tab section information*/
     #define SHELL_CMD_SECTION_NAME  "shell_tab"
     #pragma section = SHELL_CMD_SECTION_NAME
     #define SHELL_CMD_START_ADDR    __section_begin(SHELL_CMD_SECTION_NAME)
     #define SHELL_CMD_NUM           (((os_u32)__section_end(SHELL_CMD_SECTION_NAME) - (os_u32)__section_begin(SHELL_CMD_SECTION_NAME)) / sizeof(struct shell_cmd))
     #define __SHELL_CMD_SECTION     SECTION(SHELL_CMD_SECTION_NAME)

     /* hal function command tab section information*/
     #define HAL_SECTION_NAME        "hal_tab"
     #pragma section = HAL_SECTION_NAME
     #define HAL_FUNC_START_ADDR     __section_begin(HAL_SECTION_NAME)
     #define HAL_FUNC_NUM            (((os_u32)__section_end(HAL_SECTION_NAME) - (os_u32)__section_begin(HAL_SECTION_NAME)) / sizeof(struct hal_func))
     #define __HAL_SECTION           SECTION(HAL_SECTION_NAME)
#elif defined (__GNUC__)
	/*shell command tab section information*/
     #define SHELL_CMD_SECTION_NAME  "shell_tab"
     #define SHELL_CMD_START_ADDR    0
     #define SHELL_CMD_NUM           0
     #define __SHELL_CMD_SECTION     0

     /* hal function command tab section information*/
     #define HAL_SECTION_NAME        "hal_tab"
     #define HAL_FUNC_START_ADDR     0
     #define HAL_FUNC_NUM            0
     #define __HAL_SECTION           0
#endif

/*  */
void rtos_run(void);

/*  */
#define RTOS_RUN \
int main()  \
{ \
    rtos_run(); \
}

#endif
