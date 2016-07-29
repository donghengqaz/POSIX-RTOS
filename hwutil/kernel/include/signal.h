#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "siginfo.h"

void signal_init(void);

sighandler_t signal(int signum, sighandler_t handler);

int sigqueue(pid_t pid, int signo, const union sigval value);

#endif
