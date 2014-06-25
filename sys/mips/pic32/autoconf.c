/*
 * Copyright (c) 1988 University of Utah.
 * Copyright (c) 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
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
 * from: Utah $Hdr: autoconf.c 1.31 91/01/21$
 *
 *      @(#)autoconf.c  8.1 (Berkeley) 6/10/93
 */

/*
 * Setup the system to run on the current machine.
 *
 * Configure() is called at boot time.  Available
 * devices are determined (from possibilities mentioned in ioconf.c),
 * and the drivers are initialized.
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/buf.h>
#include <sys/dkstat.h>
#include <sys/conf.h>
#include <sys/dmap.h>
#include <sys/reboot.h>

#include <machine/cpu.h>
#include <mips/dev/device.h>

/*
 * The following several variables are related to
 * the configuration process, and are used in initializing
 * the machine.
 */
int cold = 1;           /* if 1, still working on cold-start */
int dkn;                /* number of iostat dk numbers assigned so far */
int cpuspeed = 30;      /* approx # instr per usec. */

/*
 * Determine mass storage and memory configuration for a machine.
 * Get cpu type, and then switch out to machine specific procedures
 * which will probe adaptors to see what is out there.
 */
configure()
{
    register struct mips_ctlr *cp;
    register struct scsi_device *dp;
    register struct driver *drp;
    register int i;

    /* probe and initialize controllers */
    for (cp = mips_cinit; drp = cp->mips_driver; cp++) {
        if (cp->mips_addr == (char *)QUES)
            continue;
        if (!(*drp->d_init)(cp))
            continue;

        cp->mips_alive = 1;

        /* probe and initialize devices connected to controller */
        for (dp = scsi_dinit; drp = dp->sd_driver; dp++) {
            /* might want to get fancier later */
            if (dp->sd_cdriver != cp->mips_driver ||
                dp->sd_ctlr != cp->mips_unit)
                continue;       /* not connected */
            if (!(*drp->d_init)(dp))
                continue;
            dp->sd_alive = 1;
            /* if device is a disk, assign number for statistics */
            if (dp->sd_dk && dkn < DK_NDRIVE)
                dp->sd_dk = dkn++;
            else
                dp->sd_dk = -1;
        }
    }

    swapconf();
    cold = 0;
}

/*
 * Configure swap space and related parameters.
 */
swapconf()
{
    register struct swdevt *swp;
    register int nblks;

    for (swp = swdevt; swp->sw_dev != NODEV; swp++)
        if (bdevsw[major(swp->sw_dev)].d_psize) {
            nblks =
              (*bdevsw[major(swp->sw_dev)].d_psize)(swp->sw_dev);
            if (nblks != -1 &&
                (swp->sw_nblks == 0 || swp->sw_nblks > nblks))
                swp->sw_nblks = nblks;
        }
    dumpconf();
}
