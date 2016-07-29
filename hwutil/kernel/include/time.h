#ifndef _TIME_H_
#define _TIME_H_

#include "signal.h"

struct timespec
{
    /* second of time */
    time_t tv_sec;

    /* nano second of time */
    long tv_nsec;
};

struct itimerspect
{
    /* the reload time of timer set */
    struct timespec it_interval; 

    /* the time of timer set */
    struct timespec it_value;
};

struct tm
{
    int     tm_ms;
    int     tm_sec;
    int     tm_min;
    int     tm_hour;
    int     tm_mday;
    int     tm_mon;
    int     tm_year;
    int     tm_wday;
    int     tm_yday;
    int     tm_isdst;
};

int timer_init(void);
int timer_create (clockid_t clockid, struct sigevent *RESTRICT evp, timer_t *RESTRICT timerid);
int timer_settime (timer_t timerid, int flags, const struct itimerspect *value, struct itimerspect *ovalue);
struct tm *localtime_r(const time_t *time, struct tm *RESTRICT result);

extern void udelay(int us);
extern void mdelay(int ms);

#endif
