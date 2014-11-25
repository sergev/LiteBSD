/*
 * Copyright (c) 2002 Networks Associates Technology, Inc.
 * All rights reserved.
 *
 * This software was developed for the FreeBSD Project by Marshall
 * Kirk McKusick and Network Associates Laboratories, the Security
 * Research Division of Network Associates, Inc. under DARPA/SPAWAR
 * contract N66001-01-C-8035 ("CBOSS"), as part of the DARPA CHATS
 * research program.
 *
 * Copyright (c) 1983, 1989, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
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
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/mount.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/wait.h>

#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ffs/fs.h>

#include <ctype.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <signal.h>

#include "mntopts.h"
#include "pathnames.h"

#ifndef ECANCELED
#define ECANCELED 1
#endif

struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_ASYNC,
	MOPT_UPDATE,
	MOPT_FORCE,
	{ NULL },
};

void	fatal(const char *fmt, ...)
	    __attribute__((__format__ (printf, 1, 2)))
	    __attribute__((__nonnull__ (1)));
__dead void	usage(void);
void	mkfs(char *, int, int, mode_t, uid_t, gid_t);

/*
 * The following two constants set the default block and fragment sizes.
 * Both constants must be a power of 2 and meet the following constraints:
 *	MINBSIZE <= DESBLKSIZE <= MAXBSIZE
 *	sectorsize <= DESFRAGSIZE <= DESBLKSIZE
 *	DESBLKSIZE / DESFRAGSIZE <= 8
 */
#define	DFL_FRAGSIZE	4096
#define	DFL_BLKSIZE	4096

/*
 * MAXBLKPG determines the maximum number of data blocks which are
 * placed in a single cylinder group. The default is one indirect
 * block worth of data blocks.
 */
#define MAXBLKPG_FFS1(bsize)	((bsize) / sizeof(int32_t))

/*
 * Each file system has a number of inodes statically allocated.
 * We allocate one inode slot per NFPI fragments, expecting this
 * to be far more than we will ever need.
 */
#define	NFPI		4

int	mfs;			/* run as the memory based filesystem */
int	Nflag;			/* run without writing file system */
daddr_t	fssize;			/* file system size in 512-byte blocks */
long 	sectorsize;		/* bytes/sector */
int	fsize = 0;		/* fragment size */
int	bsize = 0;		/* block size */
int	maxfrgspercg = INT_MAX;	/* maximum fragments per cylinder group */
int	minfree = MINFREE;	/* free space threshold */
int	opt = DEFAULTOPT;	/* optimization preference (space or time) */
int	reqopt = -1;		/* opt preference has not been specified */
int	density;		/* number of bytes per inode */
int	maxbpg;			/* maximum blocks per file in a cyl group */
int	avgfilesize = AVFILESIZ;/* expected average file size */
int	avgfilesperdir = AFPDIR;/* expected number of files per directory */
int	mntflags = MNT_ASYNC;	/* flags to be passed to mount */
int	quiet = 0;		/* quiet flag */
caddr_t	membase;		/* start address of memory based filesystem */
char	*disktype;
int	unlabeled;

extern	char *__progname;

#ifdef MFS
static int do_exec(const char *, const char *, char *const[]);
static int isdir(const char *);
static void copy(char *, char *, struct mfs_args *);
static int gettmpmnt(char *, size_t);
#endif

int scan_scaled(char *scaled, long *result);

static struct diskpart *
getdiskpart(s, fd)
	char *s;
	int fd;
{
	static struct diskpart lab;

	if (ioctl(fd, DIOCGETPART, (char *)&lab) < 0) {
		warn("ioctl (DIOCGETPART)");
		fatal("%s: can't read disk label", s);
	}
	return (&lab);
}

static long
strtonum(const char *numstr, long minval, long maxval, const char **errstrp)
{
	long ll = 0;
	int saved_errno = errno;
	char *ep;

	errno = 0;
	if (minval > maxval)
		goto invalid;

        ll = strtol(numstr, &ep, 10);
        if (numstr == ep || *ep != '\0')
		goto invalid;

        if ((ll == LONG_MIN && errno == ERANGE) || ll < minval)
                goto too_small;

        else if ((ll == LONG_MAX && errno == ERANGE) || ll > maxval)
                goto too_large;

	*errstrp = 0;
	errno = saved_errno;
	return ll;

invalid:
	*errstrp = "invalid";
	errno = EINVAL;
	return 0;

too_small:
	*errstrp = "too small";
	errno = ERANGE;
	return 0;

too_large:
	*errstrp = "too large";
	errno = ERANGE;
	return 0;
}

/*
 * This routine is a generic rewrite of the original code found in
 * disklabel(8).
 */
static int
opendev(const char *path, int oflags, int dflags, char **realpath)
{
	static char namebuf[PATH_MAX];
	char *slash, *prefix;
	int fd;

	/* Initial state */
	fd = -1;
	errno = ENOENT;

	if (dflags & 4)                         /* OPENDEV_BLCK */
		prefix = "";			/* block device */
	else
		prefix = "r";			/* character device */

        slash = strchr(path, '/');
	if (slash) {
		strlcpy(namebuf, path, sizeof(namebuf));
		fd = open(namebuf, oflags);
	} else {
                if (snprintf(namebuf, sizeof(namebuf), "%s%s%s",
                    _PATH_DEV, prefix, path) < sizeof(namebuf)) {
                        fd = open(namebuf, oflags);
                } else
                        errno = ENAMETOOLONG;
	}
	if (realpath)
		*realpath = namebuf;

	return (fd);
}

int
main(int argc, char *argv[])
{
	int ch;
	struct diskpart *pp;
	struct stat st;
	struct statfs *mp;
	struct rlimit rl;
	int fsi = -1, oflagset = 0, fso = -1, len, n;
	char *cp = NULL, *s1, *s2, *special, *opstring, *realdev;
#ifdef MFS
	char mountfromname[BUFSIZ];
	char *pop = NULL, node[MAXPATHLEN];
	pid_t pid, res;
	struct statfs sf;
	struct stat mountpoint;
	int status;
#endif
	uid_t mfsuid = 0;
	gid_t mfsgid = 0;
	mode_t mfsmode = 0;
	char *fstype = NULL;
	char **saveargv = argv;
	int ffsflag = 1;
	const char *errstr;
	long fssize_input = 0;
	int fssize_usebytes = 0;
	u_int64_t nsecs;

	if (strstr(__progname, "mfs"))
		mfs = Nflag = quiet = 1;

	opstring = mfs ?
	    "P:T:b:c:e:f:i:m:o:s:" :
	    "NS:T:b:c:e:f:g:h:i:m:o:qs:t:";
	while ((ch = getopt(argc, argv, opstring)) != -1) {
		switch (ch) {
		case 'N':
			Nflag = 1;
			break;
		case 'S':
			if (scan_scaled(optarg, &sectorsize) == -1 ||
			    sectorsize <= 0 || (sectorsize % DEV_BSIZE))
				fatal("sector size invalid: %s", optarg);
			break;
		case 'T':
			disktype = optarg;
			break;
		case 'b':
			bsize = strtonum(optarg, MINBSIZE, MAXBSIZE, &errstr);
			if (errstr)
				fatal("block size is %s: %s", errstr, optarg);
			break;
		case 'c':
			maxfrgspercg = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr)
				fatal("fragments per cylinder group is %s: %s",
				    errstr, optarg);
			break;
		case 'e':
			maxbpg = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr)
				fatal("blocks per file in a cylinder group is"
				    " %s: %s", errstr, optarg);
			break;
		case 'f':
			fsize = strtonum(optarg, MINBSIZE / MAXFRAG, MAXBSIZE,
			    &errstr);
			if (errstr)
				fatal("fragment size is %s: %s",
				    errstr, optarg);
			break;
		case 'g':
			avgfilesize = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr)
				fatal("average file size is %s: %s",
				    errstr, optarg);
			break;
		case 'h':
			avgfilesperdir = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr)
				fatal("average files per dir is %s: %s",
				    errstr, optarg);
			break;
		case 'i':
			density = strtonum(optarg, 1, INT_MAX, &errstr);
			if (errstr)
				fatal("bytes per inode is %s: %s",
				    errstr, optarg);
			break;
		case 'm':
			minfree = strtonum(optarg, 0, 99, &errstr);
			if (errstr)
				fatal("free space %% is %s: %s",
				    errstr, optarg);
			break;
		case 'o':
			if (mfs)
				getmntopts(optarg, mopts, &mntflags, 0);
			else {
				if (strcmp(optarg, "space") == 0)
					reqopt = opt = FS_OPTSPACE;
				else if (strcmp(optarg, "time") == 0)
					reqopt = opt = FS_OPTTIME;
				else
					fatal("%s: unknown optimization "
					    "preference: use `space' or `time'.",
					    optarg);
			}
			break;
		case 'q':
			quiet = 1;
			break;
		case 's':
			if (scan_scaled(optarg, &fssize_input) == -1 ||
			    fssize_input <= 0)
				fatal("file system size invalid: %s", optarg);
			fssize_usebytes = 0;    /* in case of multiple -s */
			for (s1 = optarg; *s1 != '\0'; s1++)
				if (isalpha((unsigned char)*s1)) {
					fssize_usebytes = 1;
					break;
				}
			break;
		case 't':
			fstype = optarg;
			if (strcmp(fstype, "ffs"))
				ffsflag = 0;
			break;
#ifdef MFS
		case 'P':
			pop = optarg;
			break;
#endif
		case '?':
		default:
			usage();
		}
		if (!ffsflag)
			break;
	}
	argc -= optind;
	argv += optind;

	if (ffsflag && argc - mfs != 1)
		usage();

	if (mfs) {
		/* Increase our data size to the max */
		if (getrlimit(RLIMIT_DATA, &rl) == 0) {
			rl.rlim_cur = rl.rlim_max;
			(void)setrlimit(RLIMIT_DATA, &rl);
		}
	}

	special = argv[0];

	if (mfs && strcmp(special, "swap") == 0) {
		/*
		 * it's an MFS, mounted on "swap."  fake up a label.
		 * XXX XXX XXX
		 */
                struct diskpart mfsfakepart;

		memset(&mfsfakepart, 0, sizeof(mfsfakepart));
		pp = &mfsfakepart;
		pp->dp_size = 16384;
		goto havelabel;
	}
	if (! Nflag) {
		fso = opendev(special, O_WRONLY, 0, &realdev);
		if (fso < 0)
			fatal("%s: %s", special, strerror(errno));
		special = realdev;

		/* Bail if target special is mounted */
		n = getmntinfo(&mp, MNT_NOWAIT);
		if (n == 0)
			fatal("%s: getmntinfo: %s", special, strerror(errno));

		len = sizeof(_PATH_DEV) - 1;
		s1 = special;
		if (strncmp(_PATH_DEV, s1, len) == 0)
			s1 += len;

		while (--n >= 0) {
			s2 = mp->f_mntfromname;
			if (strncmp(_PATH_DEV, s2, len) == 0) {
				s2 += len - 1;
				*s2 = 'r';
			}
			if (strcmp(s1, s2) == 0 || strcmp(s1, &s2[1]) == 0)
				fatal("%s is mounted on %s",
				    special, mp->f_mntonname);
			++mp;
		}
	}
        fsi = opendev(special, O_RDONLY, 0, NULL);
        if (fsi < 0)
                fatal("%s: %s", special, strerror(errno));
        if (fstat(fsi, &st) < 0)
                fatal("%s: %s", special, strerror(errno));
        if (!mfs) {
                if (S_ISBLK(st.st_mode))
                        fatal("%s: block device", special);
                if (!S_ISCHR(st.st_mode))
                        warnx("%s: not a character-special device",
                            special);
        }
        pp = getdiskpart(special, fsi);
        if (pp->dp_size == 0)
                fatal("%s: `%c' partition is unavailable",
                    argv[0], *cp);
havelabel:
	if (sectorsize == 0) {
		sectorsize = DEV_BSIZE;
		if (sectorsize <= 0)
			fatal("%s: no default sector size", argv[0]);
	}

	if (fssize_usebytes) {
		nsecs = fssize_input / sectorsize;
		if (fssize_input % sectorsize != 0)
			nsecs++;
	} else if (fssize_input == 0)
		nsecs = pp->dp_size;
	else
		nsecs = fssize_input;

	if (nsecs > pp->dp_size && !mfs)
	       fatal("%s: maximum file system size on the `%c' partition is "
		   "%llu sectors", argv[0], *cp, pp->dp_size);

	/* Can't use DL_SECTOBLK() because sectorsize may not be from label! */
	fssize = nsecs * (sectorsize / DEV_BSIZE);
	if (fssize == 0)
		fssize = pp->dp_size;
	if (fsize == 0)
		fsize = MAX(DFL_FRAGSIZE, DEV_BSIZE);
	if (bsize == 0) {
		bsize = MIN(DFL_BLKSIZE, 8 * fsize);
	}
	if (density == 0)
		density = NFPI * fsize;
	if (minfree < MINFREE && opt != FS_OPTSPACE && reqopt == -1) {
		warnx("warning: changing optimization to space "
		    "because minfree is less than %d%%\n", MINFREE);
		opt = FS_OPTSPACE;
	}
	if (maxbpg == 0) {
		maxbpg = MAXBLKPG_FFS1(bsize);
	}
#ifdef MFS
	if (mfs) {
		if (realpath(argv[1], node) == NULL)
			err(1, "realpath %s", argv[1]);
		if (stat(node, &mountpoint) < 0)
			err(ECANCELED, "stat %s", node);
		mfsuid = mountpoint.st_uid;
		mfsgid = mountpoint.st_gid;
		mfsmode = mountpoint.st_mode & ALLPERMS;
	}
#endif

	mkfs(special, fsi, fso, mfsmode, mfsuid, mfsgid);
	if (!Nflag)
		close(fso);
	close(fsi);
#ifdef MFS
	if (mfs) {
		struct mfs_args args;
		memset(&args, 0, sizeof(args));
		args.base = membase;
		args.size = fssize * DEV_BSIZE;
		args.export.ex_root = -2;
		if (mntflags & MNT_RDONLY)
			args.export.ex_flags = MNT_EXRDONLY;

		switch (pid = fork()) {
		case -1:
			err(10, "mfs");
		case 0:
			snprintf(mountfromname, sizeof(mountfromname),
			    "mfs:%d", getpid());
			break;
		default:
			snprintf(mountfromname, sizeof(mountfromname),
			    "mfs:%d", pid);
			for (;;) {
				/*
				 * spin until the mount succeeds
				 * or the child exits
				 */
				usleep(1);

				/*
				 * XXX Here is a race condition: another process
				 * can mount a filesystem which hides our
				 * ramdisk before we see the success.
				 */
				if (statfs(node, &sf) < 0)
					err(ECANCELED, "statfs %s", node);
				if (!strcmp(sf.f_mntfromname, mountfromname) &&
				    !strncmp(sf.f_mntonname, node,
					     MNAMELEN) &&
				    !strcmp(sf.f_fstypename, "mfs")) {
					if (pop != NULL)
						copy(pop, node, &args);
					exit(0);
				}
				res = waitpid(pid, &status, WNOHANG);
				if (res == -1)
					err(EDEADLK, "waitpid");
				if (res != pid)
					continue;
				if (WIFEXITED(status)) {
					if (WEXITSTATUS(status) == 0)
						exit(0);
					errx(1, "%s: mount: %s", node,
					     strerror(WEXITSTATUS(status)));
				} else
					errx(EDEADLK, "abnormal termination");
			}
			/* NOTREACHED */
		}

		(void) setsid();
		(void) close(0);
		(void) close(1);
		(void) close(2);
		(void) chdir("/");

		args.fspec = mountfromname;
		if (mntflags & MNT_RDONLY && pop != NULL)
			mntflags &= ~MNT_RDONLY;
		if (mount("mfs", node, mntflags, &args) < 0)
			exit(errno); /* parent prints message */
	}
#endif
	exit(0);
}

/*VARARGS*/
void
fatal(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (fcntl(STDERR_FILENO, F_GETFL) < 0) {
		openlog(__progname, LOG_CONS, LOG_DAEMON);
		vsyslog(LOG_ERR, fmt, ap);
		closelog();
	} else {
		vwarnx(fmt, ap);
	}
	va_end(ap);
	exit(1);
	/*NOTREACHED*/
}

__dead void
usage(void)
{
	extern char *__progname;

	if (mfs) {
	    fprintf(stderr,
	        "usage: %s [-b block-size] [-c fragments-per-cylinder-group] "
		"[-e maxbpg]\n"
		"\t[-f frag-size] [-i bytes] [-m free-space] [-o options] "
		"[-P file]\n"
		"\t[-s size] special node\n",
		__progname);
	} else {
	    fprintf(stderr,
	        "usage: %s [-Nq] [-b block-size] "
		"[-c fragments-per-cylinder-group] [-e maxbpg]\n"
		"\t[-f frag-size] [-g avgfilesize] [-h avgfpdir] [-i bytes]\n"
		"\t[-m free-space] [-O filesystem-format] [-o optimization]\n"
		"\t[-S sector-size] [-s size] [-T disktype] [-t fstype] "
		"special\n",
		__progname);
	}

	exit(1);
}

#ifdef MFS

static int
do_exec(const char *dir, const char *cmd, char *const argv[])
{
	pid_t pid;
	int ret, status;
	sig_t intsave, quitsave;

	switch (pid = fork()) {
	case -1:
		err(1, "fork");
	case 0:
		if (dir != NULL && chdir(dir) != 0)
			err(1, "chdir");
		if (execv(cmd, argv) != 0)
			err(1, "%s", cmd);
		break;
	default:
		intsave = signal(SIGINT, SIG_IGN);
		quitsave = signal(SIGQUIT, SIG_IGN);
		for (;;) {
			ret = waitpid(pid, &status, 0);
			if (ret == -1)
				err(11, "waitpid");
			if (WIFEXITED(status)) {
				status = WEXITSTATUS(status);
				if (status != 0)
					warnx("%s: exited", cmd);
				break;
			} else if (WIFSIGNALED(status)) {
				warnx("%s: terminated by signal %d", cmd,
				    WTERMSIG(status));
				status = 1;
				break;
			}
		}
		signal(SIGINT, intsave);
		signal(SIGQUIT, quitsave);
		return (status);
	}
	/* NOTREACHED */
	return (-1);
}

static int
isdir(const char *path)
{
	struct stat st;

	if (stat(path, &st) != 0)
		err(1, "cannot stat %s", path);
	if (!S_ISDIR(st.st_mode) && !S_ISBLK(st.st_mode))
		errx(1, "%s: not a dir or a block device", path);
	return (S_ISDIR(st.st_mode));
}

static void
copy(char *src, char *dst, struct mfs_args *args)
{
	int ret, dir, created = 0;
	struct ufs_args mount_args;
	char mountpoint[MNAMELEN];
	char *const argv[] = { "pax", "-rw", "-pe", ".", dst, NULL } ;

	dir = isdir(src);
	if (dir)
		strlcpy(mountpoint, src, sizeof(mountpoint));
	else {
		created = gettmpmnt(mountpoint, sizeof(mountpoint));
		memset(&mount_args, 0, sizeof(mount_args));
		mount_args.fspec = src;
		ret = mount("ffs", mountpoint, MNT_RDONLY, &mount_args);
		if (ret != 0) {
			int saved_errno = errno;
			if (created && rmdir(mountpoint) != 0)
				warn("rmdir %s", mountpoint);
			if (unmount(dst, 0) != 0)
				warn("unmount %s", dst);
			errno = saved_errno;
			errx(1, "mount %s %s", src, mountpoint);
		}
	}
	ret = do_exec(mountpoint, "/bin/pax", argv);
	if (!dir && unmount(mountpoint, 0) != 0)
		warn("unmount %s", mountpoint);
	if (created && rmdir(mountpoint) != 0)
		warn("rmdir %s", mountpoint);
	if (ret != 0) {
		if (unmount(dst, 0) != 0)
			warn("unmount %s", dst);
		errx(1, "copy %s to %s failed", mountpoint, dst);
	}

	if (mntflags & MNT_RDONLY) {
		mntflags |= MNT_UPDATE;
		if (mount("mfs", dst, mntflags, args) < 0) {
			warn("%s: mount (update, rdonly)", dst);
			if (unmount(dst, 0) != 0)
				warn("unmount %s", dst);
			exit(1);
		}
	}
}

static int
gettmpmnt(char *mountpoint, size_t len)
{
	const char *tmp;
	const char *mnt = _PATH_MNT;
	struct statfs fs;
	size_t n;

	tmp = getenv("TMPDIR");
	if (tmp == NULL || *tmp == '\0')
		tmp = _PATH_TMP;

	if (statfs(tmp, &fs) != 0)
		err(1, "statfs %s", tmp);
	if (fs.f_flags & MNT_RDONLY) {
		if (statfs(mnt, &fs) != 0)
			err(1, "statfs %s", mnt);
		if (strcmp(fs.f_mntonname, "/") != 0)
			errx(1, "tmp mountpoint %s busy", mnt);
		if (strlcpy(mountpoint, mnt, len) >= len)
			errx(1, "tmp mountpoint %s too long", mnt);
		return (0);
	}
	n = strlcpy(mountpoint, tmp, len);
	if (n >= len)
		errx(1, "tmp mount point too long");
	if (mountpoint[n - 1] != '/')
		strlcat(mountpoint, "/", len);
	n = strlcat(mountpoint, "mntXXXXXXXXXX", len);
	if (n >= len)
		errx(1, "tmp mount point too long");
	if (mkdtemp(mountpoint) == NULL)
		err(1, "mkdtemp %s", mountpoint);
	return (1);
}

#endif /* MFS */
