#include "hal.h"
#include "sys_tick.h"
#include "eth.h"
#include "chip_test.h"
#include "debug.h"

err_t hal_init(void)
{
    ASSERT_KERNEL( tick_confugration() == 0 );
    
    chip_test();
  
    return 0;
}
