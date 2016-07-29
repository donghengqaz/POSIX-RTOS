#ifndef _SPINLOCK_H_
#define _SPINLOCK_H_

#include "types.h"

INLINE int arch_try_spin_lock(void* x)
{
  asm ( "ldrex r1, [r0] \n"
        "cbnz  r1, ret  \n"
        "mov   r2, #1   \n"
        "strex r0, r2, [r1] \n"
        "ret:    ");
}

INLINE int arch_spin_unlock(void* x)
{
  asm ( "ldrex r1, [r0] \n"
        "cbz   r1, ret  \n"
        "mov   r2, #0   \n"
        "strex r0, r2, [r1] \n"
        "ret:    ");
}

#endif
