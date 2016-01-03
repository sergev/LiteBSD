/*
 * CPU-dependent part of sysctl() system call.
 *
 * Copyright (c) 2014 Serge Vakulenko
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
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
#include "pty.h"

/*
 * Names for intrcnt data.
 */
static char intrnames[] =
    "clock\0"
    "softclock\0"
    "softnet\0"
    "uart1\0"
    "uart2\0"
    "uart3\0"
    "uart4\0"
    "uart5\0"
    "uart6\0"
    "ether\0";

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
#if NPTY > 0
    { "_npty",          (int) &npty         },
#endif
    { "_nswap",         (int) &nswap        },
    { "_nswapmap",      (int) &nswapmap     },
    { "_nswdev",        (int) &nswdev       },
    { "_numvnodes",     (int) &numvnodes    },
    { "_profhz",        (int) &profhz       },
#if NPTY > 0
    { "_pt_tty",        (int) &pt_tty       },
#endif
    { "_rtstat",        (int) &rtstat       },
    { "_rt_tables",     (int) &rt_tables    },
    { "_conf_dinit",    (int) &conf_dinit   },
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
    int i, error;
    char buf[16];

    /* all sysctl names at this level are terminal */
    if (namelen != 1)
        return (ENOTDIR);               /* overloaded */

    switch (name[0]) {
    case CPU_CONSDEV:
        /* Get major/minor of a console device. */
        return sysctl_rdstruct(oldp, oldlenp, newp, &cn_dev, sizeof cn_dev);

    case CPU_NLIST:
        /* Get address of a kernel symbol. */
        for (i=0; nlist[i].name; i++) {
            error = copyinstr(newp, buf, sizeof(buf), 0);
            if (error)
                return error;
            if (bcmp(buf, nlist[i].name, newlen) == 0) {
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

    case CPU_WIFI_SCAN:
        /* Scan the Wi-Fi network. */
#ifdef WF_INT
        if (newp) {
            wifi_scan();
            return 0;
        }
#endif
        break;
    }
    return EOPNOTSUPP;
}
