#ifndef _INIT_H_
#define _INIT_H_

#include "rtos.h"

#ifndef SHELL_CMD_SECTION_NAME
    #define SHELL_CMD_SECTION_NAME  "shell_tab"
#endif
#ifndef CHIP_SECTION_NAME
    #define CHIP_SECTION_NAME       "chip_tab"
#endif
#ifndef HAL_SECTION_NAME
    #define HAL_SECTION_NAME        "hal_tab"
#endif

#if defined (__ICCARM__)
    /*shell command tab section information*/
    #pragma section = SHELL_CMD_SECTION_NAME
    #define SHELL_CMD_START_ADDR    __section_begin(SHELL_CMD_SECTION_NAME)
    #define SHELL_CMD_NUM           (((os_u32)__section_end(SHELL_CMD_SECTION_NAME) - (os_u32)__section_begin(SHELL_CMD_SECTION_NAME)) / sizeof(struct shell_cmd))
    #define __SHELL_CMD_SECTION     SECTION(SHELL_CMD_SECTION_NAME)

    /* HAL function command tab section information*/
    #pragma section = HAL_SECTION_NAME
    #define HAL_FUNC_START_ADDR     __section_begin(HAL_SECTION_NAME)
    #define HAL_FUNC_NUM            (((os_u32)__section_end(HAL_SECTION_NAME) - (os_u32)__section_begin(HAL_SECTION_NAME)) / sizeof(struct hal_func))
    #define __HAL_SECTION           SECTION(HAL_SECTION_NAME)
#elif defined (__GNUC__)
    extern os_u32 __shell_tab_start, __shell_tab_end;
    extern os_u32 __chip_tab_start, __chip_tab_end;
    extern os_u32 __hal_tab_start, __hal_tab_end;

	/*shell command tab section information*/
    #define SHELL_CMD_START_ADDR    __shell_tab_start
    #define SHELL_CMD_END_ADDR      __shell_tab_end
    #define SHELL_CMD_NUM           ((SHELL_CMD_START_ADDR - SHELL_CMD_END_ADDR) / sizeof(struct shell_cmd))
    #define __SHELL_CMD_SECTION     SECTION(SHELL_CMD_SECTION_NAME)

    /* chip function command tab section information*/
    #define CHIP_FUNC_START_ADDR    __chip_tab_start
    #define CHIP_FUNC_END_ADDR      __chip_tab_end
    #define CHIP_FUNC_NUM            ((CHIP_FUNC_START_ADDR - CHIP_FUNC_END_ADDR) / sizeof(struct chip_desc))
    #define __CHIP_SECTION           SECTION(CHIP_SECTION_NAME)

    /* HAL function command tab section information*/
    #define HAL_FUNC_START_ADDR     __hal_tab_start
    #define HAL_FUNC_END_ADDR       __hal_tab_end
    #define HAL_FUNC_NUM            ((HAL_FUNC_START_ADDR - HAL_FUNC_END_ADDR) / sizeof(struct hal_func))
    #define __HAL_SECTION           SECTION(HAL_SECTION_NAME)
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
