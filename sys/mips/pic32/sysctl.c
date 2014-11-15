/*
 * Copyright (c) 2014 Serge Vakulenko
 *
 * This code is derived from software contributed to Berkeley by
 * the Systems Programming Group of the University of Utah Computer
 * Science Department, The Mach Operating System project at
 * Carnegie-Mellon University and Ralph Campbell.
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
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/file.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/msgbuf.h>
#include <sys/tty.h>
#include <sys/user.h>
#include <sys/sysctl.h>
#include <sys/mount.h>
#include <sys/dkstat.h>
#include <sys/dmap.h>
#include <sys/gmon.h>
#include <sys/socket.h>
#include <sys/namei.h>
#include <sys/map.h>
#include <sys/protosw.h>

#include <machine/cpu.h>
#include <machine/pte.h>
#include <mips/dev/device.h>

#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/ip_var.h>
#include <netinet/ip_mroute.h>
#include <netinet/icmp_var.h>
#include <netinet/igmp_var.h>
#include <netinet/in_pcb.h>
#include <netinet/tcp.h>
#include <netinet/tcp_timer.h>
#include <netinet/tcp_var.h>
#include <netinet/udp.h>
#include <netinet/udp_var.h>

#include <nfs/rpcv2.h>
#include <nfs/nfsproto.h>
#include <nfs/nfs.h>

/*
 * Names for intrcnt data.
 */
static char intrnames[] =
    "softclock\0"
    "softnet\0"
    "uart\0"
    "ether\0"
    "disk\0"
    "memory\0"
    "clock\0"
    "fp\0";

/*
 * List of kernel symbols, exported via sysctl() call.
 */
static const struct {
    const char *name;
    int addr;
} nlist[] = {
    { "_allproc",       (int) &allproc      },
    { "_avail_end",     (int) &avail_end    },
    { "_avail_start",   (int) &avail_start  },
    { "_averunnable",   (int) &averunnable  },
    { "_boottime",      (int) &boottime     },
    { "_bucket",        (int) &bucket       },
    { "_ccpu",          (int) &ccpu         },
    { "_cnt",           (int) &cnt          },
    { "_cp_time",       (int) &cp_time      },
    { "_dk_busy",       (int) &dk_busy      },
    { "_dk_ndrive",     (int) &dk_ndrive    },
    { "_dk_seek",       (int) &dk_seek      },
    { "_dk_time",       (int) &dk_time      },
    { "_dk_wds",        (int) &dk_wds       },
    { "_dk_wpms",       (int) &dk_wpms      },
    { "_dk_xfer",       (int) &dk_xfer      },
    { "_dmmax",         (int) &dmmax        },
    { "_eintrcnt",      (int) &intrcnt + sizeof(intrcnt) },
    { "_eintrnames",    (int) &intrnames + sizeof(intrnames) },
    { "_filehead",      (int) &filehead     },
    { "_fscale",        (int) &fscale       },
#ifdef GPROF
    { "__gmonparam",    (int) &_gmonparam   },
#endif
    { "_hz",            (int) &hz           },
    { "_icmpstat",      (int) &icmpstat     },
    { "_ifnet",         (int) &ifnet        },
    { "_igmpstat",      (int) &igmpstat     },
    { "_intrcnt",       (int) &intrcnt      },
    { "_intrnames",     (int) &intrnames    },
    { "_ip_mrtproto",   (int) &ip_mrtproto  },
    { "_ipstat",        (int) &ipstat       },
    { "_kmemstats",     (int) &kmemstats    },
    { "_maxfiles",      (int) &maxfiles     },
    { "_mbstat",        (int) &mbstat       },
    { "_mountlist",     (int) &mountlist    },
    { "_mrtstat",       (int) &mrtstat      },
    { "_mrttable",      (int) &mrttable     },
    { "_msgbufp",       (int) &msgbufp      },
    { "_nchstats",      (int) &nchstats     },
    { "_nfiles",        (int) &nfiles       },
    { "_nfsstats",      (int) &nfsstats     },
#ifdef SEQSWAP
    { "_niswap",        (int) &niswap       },
    { "_niswdev",       (int) &niswdev      },
#endif
    { "_nprocs",        (int) &nprocs       },
    { "_npty",          (int) &npty         },
    { "_nswap",         (int) &nswap        },
    { "_nswapmap",      (int) &nswapmap     },
    { "_nswdev",        (int) &nswdev       },
    { "_numvnodes",     (int) &numvnodes    },
    { "_profhz",        (int) &profhz       },
    { "_pt_tty",        (int) &pt_tty       },
    { "_rtstat",        (int) &rtstat       },
    { "_rt_tables",     (int) &rt_tables    },
    { "_scsi_dinit",    (int) &scsi_dinit   },
    { "_stathz",        (int) &stathz       },
    { "_swapmap",       (int) &swapmap      },
    { "_swdevt",        (int) &swdevt       },
    { "Sysmap",         (int) &Sysmap       },
    { "Sysmapsize",     (int) &Sysmapsize   },
    { "_tcb",           (int) &tcb          },
    { "_tcpstat",       (int) &tcpstat      },
    { "_tk_nin",        (int) &tk_nin       },
    { "_tk_nout",       (int) &tk_nout      },
    { "_total",         (int) &total        },
    { "_uart_cnt",      (int) &uart_cnt     },
    { "_uart_tty",      (int) &uart_tty     },
    { "_udb",           (int) &udb          },
    { "_udpstat",       (int) &udpstat      },
    { "_unixsw",        (int) &unixsw       },
    { "_viftable",      (int) &viftable     },
    { "_zombproc",      (int) &zombproc     },
    { 0, 0 },
};

/*
 * Machine dependent system variables.
 */
int
cpu_sysctl(name, namelen, oldp, oldlenp, newp, newlen, p)
    int *name;
    u_int namelen;
    void *oldp;
    size_t *oldlenp;
    void *newp;
    size_t newlen;
    struct proc *p;
{
    int i;

    /* all sysctl names at this level are terminal */
    if (namelen != 1)
        return (ENOTDIR);               /* overloaded */

    switch (name[0]) {
    case CPU_CONSDEV:
        return sysctl_rdstruct(oldp, oldlenp, newp, &cn_dev, sizeof cn_dev);

    case CPU_NLIST:
        /* Get address of a kernel symbol. */
        for (i=0; nlist[i].name; i++) {
            if (bcmp(newp, nlist[i].name, newlen) == 0) {
                int addr = nlist[i].addr;
                if (! oldp)
                    return 0;
                if (*oldlenp < sizeof(int))
                    return ENOMEM;
                *oldlenp = sizeof(int);
                return copyout((caddr_t) &addr, (caddr_t) oldp, sizeof(int));
            }
        }
        return EOPNOTSUPP;
    }
    return EOPNOTSUPP;
}
