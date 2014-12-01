/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 * Copyright (c) 2014 Serge Vakulenko
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department and Ralph Campbell.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * from: Utah $Hdr: clock.c 1.18 91/01/21$
 *
 *      @(#)clock.c     8.2 (Berkeley) 10/9/94
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>

#include <machine/machConst.h>
#include <machine/pic32mz.h>

#define SECMIN  60U                     /* seconds per minute */
#define SECHOUR (60U * SECMIN)          /* seconds per hour */
#define SECDAY  (24U * SECHOUR)         /* seconds per day */
#define SECYR   (365U * SECDAY)         /* seconds per common year */

#define LEAPYEAR(year)  (((year) % 4) == 0)

/*
 * Machine-dependent clock routines.
 *
 * Startrtclock restarts the real-time clock, which provides
 * hardclock interrupts to kern_clock.c.
 *
 * Inittodr initializes the time of day hardware which provides
 * date functions.  Its primary function is to use some file
 * system information in case the hardare clock lost state.
 *
 * Resettodr restores the time of day hardware after a time change.
 */

/*
 * Start the real-time and statistics clocks. Leave stathz 0 since there
 * are no other timers available.
 */
void
cpu_initclocks()
{
    extern int tickadj;

    hz = HZ;                    /* 100 Hz */
    tick = 1000000 / HZ;        /* number of micro-seconds between interrupts */
    tickadj = 240000 / (60 * HZ);

    unsigned count = mfc0_Count();
    count += (CPU_KHZ * 1000 / HZ + 1) / 2;
    mtc0_Compare (count);
    IECSET(0) = 1 << PIC32_IRQ_CT;
}

/*
 * We assume newhz is either stathz or profhz, and that neither will
 * change after being set up above.  Could recalculate intervals here
 * but that would be a drag.
 */
void
setstatclockrate(newhz)
    int newhz;
{
}

/*
 * This is the amount to add to the value stored in the clock chip
 * to get the current year.
 */
#define YR_OFFSET       22

/*
 * This code is defunct after 2099.
 * Will Unix still be here then??
 */
static short dayyr[12] = {
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334
};

/*
 * Initialze the time of day register, based on the time base which is, e.g.
 * from a filesystem.  Base provides the time to within six months,
 * and the time of year clock (if any) provides the rest.
 */
void
inittodr(base)
    time_t base;
{
    register int days, yr;
    int sec, min, hour, day, mon, year;
    long deltat;
    int badbase;

    if (base < 5*SECYR) {
        printf("WARNING: preposterous time in file system");
        /* read the system clock anyway */
        base = 6*SECYR + 186*SECDAY + SECDAY/2;
        badbase = 1;
    } else
        badbase = 0;

#if 0
    // TODO: pic32
    register volatile struct chiptime *c = Mach_clock_addr;
    /* don't read clock registers while they are being updated */
    int s = splclock();
    while ((c->rega & REGA_UIP) == 1)
        ;
    sec = c->sec;
    min = c->min;
    hour = c->hour;
    day = c->day;
    mon = c->mon;
    year = c->year + YR_OFFSET;
    splx(s);
#else
    sec = 0;
    min = 0;
    hour = 0;
    day = 0;
    mon = 0;
    year = 0 + YR_OFFSET;
#endif

    /* simple sanity checks */
    if (year < 70 || mon < 1 || mon > 12 || day < 1 || day > 31 ||
        hour > 23 || min > 59 || sec > 59) {
        /*
         * Believe the time in the file system for lack of
         * anything better, resetting the TODR.
         */
        time.tv_sec = base;
        if (!badbase) {
            printf("WARNING: preposterous clock chip time");
            resettodr();
        }
        goto bad;
    }
    days = 0;
    for (yr = 70; yr < year; yr++)
        days += LEAPYEAR(yr) ? 366 : 365;
    days += dayyr[mon - 1] + day - 1;
    if (LEAPYEAR(yr) && mon > 2)
        days++;
    /* now have days since Jan 1, 1970; the rest is easy... */
    time.tv_sec = days * SECDAY + hour * 3600 + min * 60 + sec;

    if (!badbase) {
        /*
         * See if we gained/lost two or more days;
         * if so, assume something is amiss.
         */
        deltat = time.tv_sec - base;
        if (deltat < 0)
            deltat = -deltat;
        if (deltat < 2 * SECDAY)
            return;
        printf("WARNING: clock %s %d days",
            time.tv_sec < base ? "lost" : "gained", deltat / SECDAY);
    }
bad:
    printf(" -- CHECK AND RESET THE DATE!\n");
}

/*
 * Reset the TODR based on the time value; used when the TODR
 * has a preposterous value and also when the time is reset
 * by the stime system call.  Also called when the TODR goes past
 * TODRZERO + 100*(SECYEAR+2*SECDAY) (e.g. on Jan 2 just after midnight)
 * to wrap the TODR around.
 */
void
resettodr()
{
#if 0
    register int t, t2;
    int sec, min, hour, day, mon, year;

    /* compute the year */
    t2 = time.tv_sec / SECDAY;
    year = 69;
    while (t2 >= 0) {       /* whittle off years */
        t = t2;
        year++;
        t2 -= LEAPYEAR(year) ? 366 : 365;
    }

    /* t = month + day; separate */
    t2 = LEAPYEAR(year);
    for (mon = 1; mon < 12; mon++)
        if (t < dayyr[mon] + (t2 && mon > 1))
            break;

    day = t - dayyr[mon - 1] + 1;
    if (t2 && mon > 2)
        day--;

    /* the rest is easy */
    t = time.tv_sec % SECDAY;
    hour = t / 3600;
    t %= 3600;
    min = t / 60;
    sec = t % 60;

    // TODO: pic32
    register volatile struct chiptime *c = Mach_clock_addr;
    int s = splclock();
    t = c->regb;
    c->regb = t | REGB_SET_TIME;
    mips_sync();
    c->sec = sec;
    c->min = min;
    c->hour = hour;
    c->day = day;
    c->mon = mon;
    c->year = year - YR_OFFSET;
    c->regb = t;
    mips_sync();
    splx(s);
#endif
}
