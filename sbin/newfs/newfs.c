/*
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
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
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

#ifndef lint
static char sccsid[] = "@(#)newfs.c	8.13 (Berkeley) 5/1/95";
#endif /* not lint */

#ifndef lint
static char copyright[] =
"@(#) Copyright (c) 1983, 1989, 1993, 1994\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif /* not lint */

/*
 * newfs: friendly front end to mkfs
 */
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/disklabel.h>
#include <sys/file.h>
#include <sys/mount.h>

#include <ufs/ufs/dir.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ffs/fs.h>
#include <ufs/ufs/ufsmount.h>

#include <ctype.h>
#include <errno.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "mntopts.h"

struct mntopt mopts[] = {
	MOPT_STDOPTS,
	MOPT_ASYNC,
	{ NULL },
};

#if __STDC__
void	fatal(const char *fmt, ...);
#else
void	fatal();
#endif

/*
 * The following two constants set the default block and fragment sizes.
 * Both constants must be a power of 2 and meet the following constraints:
 *	MINBSIZE <= DESBLKSIZE <= MAXBSIZE
 *	sectorsize <= DESFRAGSIZE <= DESBLKSIZE
 *	DESBLKSIZE / DESFRAGSIZE <= 8
 */
#define	DFL_FRAGSIZE	1024
#define	DFL_BLKSIZE	8192

/*
 * Cylinder groups may have up to many cylinders. The actual
 * number used depends upon how much information can be stored
 * on a single cylinder. The default is to use 16 cylinders
 * per group.
 */
#define	DESCPG		16	/* desired fs_cpg */

/*
 * ROTDELAY gives the minimum number of milliseconds to initiate
 * another disk transfer on the same cylinder. It is used in
 * determining the rotationally optimal layout for disk blocks
 * within a file; the default of fs_rotdelay is 4ms.
 */
#define ROTDELAY	4

/*
 * MAXBLKPG determines the maximum number of data blocks which are
 * placed in a single cylinder group. The default is one indirect
 * block worth of data blocks.
 */
#define MAXBLKPG(bsize)	((bsize) / sizeof(daddr_t))

/*
 * Each file system has a number of inodes statically allocated.
 * We allocate one inode slot per NFPI fragments, expecting this
 * to be far more than we will ever need.
 */
#define	NFPI		4

/*
 * For each cylinder we keep track of the availability of blocks at different
 * rotational positions, so that we can lay out the data to be picked
 * up with minimum rotational latency.  NRPOS is the default number of
 * rotational positions that we distinguish.  With NRPOS of 8 the resolution
 * of our summary information is 2ms for a typical 3600 rpm drive.
 */
#define	NRPOS		8	/* number distinct rotational positions */


int	mfs;			/* run as the memory based filesystem */
int	Nflag;			/* run without writing file system */
int	Oflag;			/* format as an 4.3BSD file system */
int	fssize;			/* file system size */
int	ntracks;		/* # tracks/cylinder */
int	nsectors;		/* # sectors/track */
int	nphyssectors;		/* # sectors/track including spares */
int	secpercyl;		/* sectors per cylinder */
int	trackspares = -1;	/* spare sectors per track */
int	cylspares = -1;		/* spare sectors per cylinder */
int	sectorsize;		/* bytes/sector */
#ifdef tahoe
int	realsectorsize;		/* bytes/sector in hardware */
#endif
int	rpm;			/* revolutions/minute of drive */
int	interleave;		/* hardware sector interleave */
int	trackskew = -1;		/* sector 0 skew, per track */
int	headswitch;		/* head switch time, usec */
int	trackseek;		/* track-to-track seek, usec */
int	fsize = 0;		/* fragment size */
int	bsize = 0;		/* block size */
int	cpg = DESCPG;		/* cylinders/cylinder group */
int	cpgflg;			/* cylinders/cylinder group flag was given */
int	minfree = MINFREE;	/* free space threshold */
int	opt = DEFAULTOPT;	/* optimization preference (space or time) */
int	density;		/* number of bytes per inode */
int	maxcontig = 0;		/* max contiguous blocks to allocate */
int	rotdelay = ROTDELAY;	/* rotational delay between blocks */
int	maxbpg;			/* maximum blocks per file in a cyl group */
int	nrpos = NRPOS;		/* # of distinguished rotational positions */
int	bbsize = BBSIZE;	/* boot block size */
int	sbsize = SBSIZE;	/* superblock size */
int	mntflags = MNT_ASYNC;	/* flags to be passed to mount */
u_long	memleft;		/* virtual memory available */
caddr_t	membase;		/* start address of memory based filesystem */

char	device[MAXPATHLEN];
char	*progname;

static struct diskpart *
getdiskpart(s, fd)
	char *s;
	int fd;
{
	static struct diskpart lab;

	if (ioctl(fd, DIOCGETPART, (char *)&lab) < 0) {
		warn("ioctl (GDINFO)");
		fatal("%s: can't read disk label", s);
	}
	return (&lab);
}

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	register int ch;
	register struct diskpart *pp;
	struct stat st;
	struct statfs *mp;
	int fsi, fso, len, n;
	char *cp, *s1, *s2, *special, *opstring, buf[BUFSIZ];

	if (progname = strrchr(*argv, '/'))
		++progname;
	else
		progname = *argv;

	if (strstr(progname, "mfs")) {
		mfs = 1;
		Nflag++;
	}

	opstring = mfs ?
	    "Na:b:c:d:e:f:i:m:o:s:" :
	    "NOS:a:b:c:d:e:f:i:k:l:m:n:o:p:r:s:t:u:x:";
	while ((ch = getopt(argc, argv, opstring)) != EOF)
		switch (ch) {
		case 'N':
			Nflag = 1;
			break;
		case 'O':
			Oflag = 1;
			break;
		case 'S':
			if ((sectorsize = atoi(optarg)) <= 0)
				fatal("%s: bad sector size", optarg);
			break;
		case 'a':
			if ((maxcontig = atoi(optarg)) <= 0)
				fatal("%s: bad maximum contiguous blocks\n",
				    optarg);
			break;
		case 'b':
			if ((bsize = atoi(optarg)) < MINBSIZE)
				fatal("%s: bad block size", optarg);
			break;
		case 'c':
			if ((cpg = atoi(optarg)) <= 0)
				fatal("%s: bad cylinders/group", optarg);
			cpgflg++;
			break;
		case 'd':
			if ((rotdelay = atoi(optarg)) < 0)
				fatal("%s: bad rotational delay\n", optarg);
			break;
		case 'e':
			if ((maxbpg = atoi(optarg)) <= 0)
		fatal("%s: bad blocks per file in a cylinder group\n",
				    optarg);
			break;
		case 'f':
			if ((fsize = atoi(optarg)) <= 0)
				fatal("%s: bad fragment size", optarg);
			break;
		case 'i':
			if ((density = atoi(optarg)) <= 0)
				fatal("%s: bad bytes per inode\n", optarg);
			break;
		case 'k':
			if ((trackskew = atoi(optarg)) < 0)
				fatal("%s: bad track skew", optarg);
			break;
		case 'l':
			if ((interleave = atoi(optarg)) <= 0)
				fatal("%s: bad interleave", optarg);
			break;
		case 'm':
			if ((minfree = atoi(optarg)) < 0 || minfree > 99)
				fatal("%s: bad free space %%\n", optarg);
			break;
		case 'n':
			if ((nrpos = atoi(optarg)) <= 0)
				fatal("%s: bad rotational layout count\n",
				    optarg);
			break;
		case 'o':
			if (mfs)
				getmntopts(optarg, mopts, &mntflags, 0);
			else {
				if (strcmp(optarg, "space") == 0)
					opt = FS_OPTSPACE;
				else if (strcmp(optarg, "time") == 0)
					opt = FS_OPTTIME;
				else
	fatal("%s: unknown optimization preference: use `space' or `time'.");
			}
			break;
		case 'p':
			if ((trackspares = atoi(optarg)) < 0)
				fatal("%s: bad spare sectors per track",
				    optarg);
			break;
		case 'r':
			if ((rpm = atoi(optarg)) <= 0)
				fatal("%s: bad revolutions/minute\n", optarg);
			break;
		case 's':
			if ((fssize = atoi(optarg)) <= 0)
				fatal("%s: bad file system size", optarg);
			break;
		case 't':
			if ((ntracks = atoi(optarg)) <= 0)
				fatal("%s: bad total tracks", optarg);
			break;
		case 'u':
			if ((nsectors = atoi(optarg)) <= 0)
				fatal("%s: bad sectors/track", optarg);
			break;
		case 'x':
			if ((cylspares = atoi(optarg)) < 0)
				fatal("%s: bad spare sectors per cylinder",
				    optarg);
			break;
		case '?':
		default:
			usage();
		}
	argc -= optind;
	argv += optind;

	if (argc != 2 && (mfs || argc != 1))
		usage();

	special = argv[0];
	cp = strrchr(special, '/');
	if (cp == 0) {
		/*
		 * No path prefix; try /dev/r%s then /dev/%s.
		 */
		(void)sprintf(device, "%sr%s", _PATH_DEV, special);
		if (stat(device, &st) == -1)
			(void)sprintf(device, "%s%s", _PATH_DEV, special);
		special = device;
	}
	if (Nflag) {
		fso = -1;
	} else {
		fso = open(special, O_WRONLY);
		if (fso < 0)
			fatal("%s: %s", special, strerror(errno));

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
        fsi = open(special, O_RDONLY);
        if (fsi < 0)
                fatal("%s: %s", special, strerror(errno));
        if (fstat(fsi, &st) < 0)
                fatal("%s: %s", special, strerror(errno));
        if ((st.st_mode & S_IFMT) != S_IFCHR && !mfs)
                printf("%s: %s: not a character-special device\n",
                    progname, special);
        cp = strchr(argv[0], '\0') - 1;
        if (cp == (char *)-1 ||
            (*cp < 'a' || *cp > 'h') && !isdigit(*cp))
                fatal("%s: can't figure out file system partition",
                    argv[0]);
        pp = getdiskpart(special, fsi);
        if (pp->dp_size == 0)
                fatal("%s: `%c' partition is unavailable",
                    argv[0], *cp);
	if (fssize == 0)
		fssize = pp->dp_size;
	if (fssize > pp->dp_size && !mfs)
	       fatal("%s: maximum file system size on the `%c' partition is %d",
			argv[0], *cp, pp->dp_size);
	if (rpm == 0) {
		rpm = 3600;
	}
	if (ntracks == 0) {
		ntracks = 32;
//		if (ntracks <= 0)
//			fatal("%s: no default #tracks", argv[0]);
	}
	if (nsectors == 0) {
		nsectors = 32;
//		if (nsectors <= 0)
//			fatal("%s: no default #sectors/track", argv[0]);
	}
	if (sectorsize == 0) {
		sectorsize = DEV_BSIZE;
//		if (sectorsize <= 0)
//			fatal("%s: no default sector size", argv[0]);
	}
	if (trackskew == -1) {
		trackskew = 0;
	}
	if (interleave == 0) {
		interleave = 1;
	}
	if (fsize == 0) {
		fsize = MAX(DFL_FRAGSIZE, DEV_BSIZE);
	}
	if (bsize == 0) {
		bsize = MIN(DFL_BLKSIZE, 8 * fsize);
	}
	/*
	 * Maxcontig sets the default for the maximum number of blocks
	 * that may be allocated sequentially. With filesystem clustering
	 * it is possible to allocate contiguous blocks up to the maximum
	 * transfer size permitted by the controller or buffering.
	 */
	if (maxcontig == 0)
		maxcontig = MAX(1, MIN(MAXPHYS, MAXBSIZE) / bsize);
	if (density == 0)
		density = NFPI * fsize;
	if (minfree < MINFREE && opt != FS_OPTSPACE) {
		fprintf(stderr, "Warning: changing optimization to space ");
		fprintf(stderr, "because minfree is less than %d%%\n", MINFREE);
		opt = FS_OPTSPACE;
	}
	if (trackspares == -1) {
		trackspares = 0;
	}
	nphyssectors = nsectors + trackspares;
	if (cylspares == -1) {
		cylspares = 0;
	}
	secpercyl = nsectors * ntracks - cylspares;
	if (maxbpg == 0)
		maxbpg = MAXBLKPG(bsize);
	headswitch = 0;
	trackseek = 0;
#ifdef tahoe
	realsectorsize = sectorsize;
	if (sectorsize != DEV_BSIZE) {		/* XXX */
		int secperblk = DEV_BSIZE / sectorsize;

		sectorsize = DEV_BSIZE;
		nsectors /= secperblk;
		nphyssectors /= secperblk;
		secpercyl /= secperblk;
		fssize /= secperblk;
		pp->dp_size /= secperblk;
	}
#endif
	mkfs(pp, special, fsi, fso);
#ifdef tahoe
	if (realsectorsize != DEV_BSIZE)
		pp->dp_size *= DEV_BSIZE / realsectorsize;
#endif
	if (!Nflag)
		close(fso);
	close(fsi);
#ifdef MFS
	if (mfs) {
		struct mfs_args args;

		sprintf(buf, "mfs:%d", getpid());
		args.fspec = buf;
		args.export.ex_root = -2;
		if (mntflags & MNT_RDONLY)
			args.export.ex_flags = MNT_EXRDONLY;
		else
			args.export.ex_flags = 0;
		args.base = membase;
		args.size = fssize * sectorsize;
		if (mount("mfs", argv[1], mntflags, &args) < 0)
			fatal("%s: %s", argv[1], strerror(errno));
	}
#endif
	exit(0);
}

/*VARARGS*/
void
#if __STDC__
fatal(const char *fmt, ...)
#else
fatal(fmt, va_alist)
	char *fmt;
	va_dcl
#endif
{
	va_list ap;

#if __STDC__
	va_start(ap, fmt);
#else
	va_start(ap);
#endif
	if (fcntl(STDERR_FILENO, F_GETFL) < 0) {
		openlog(progname, LOG_CONS, LOG_DAEMON);
		vsyslog(LOG_ERR, fmt, ap);
		closelog();
	} else {
		vwarnx(fmt, ap);
	}
	va_end(ap);
	exit(1);
	/*NOTREACHED*/
}

usage()
{
	if (mfs) {
		fprintf(stderr,
		    "usage: %s [ -fsoptions ] special-device mount-point\n",
			progname);
	} else
		fprintf(stderr,
                    "usage: %s [ -fsoptions ] special-device\n",
                        progname);

	fprintf(stderr, "where fsoptions are:\n");
	fprintf(stderr,
	    "\t-N do not create file system, just print out parameters\n");
	fprintf(stderr, "\t-O create a 4.3BSD format filesystem\n");
	fprintf(stderr, "\t-S sector size\n");
	fprintf(stderr, "\t-a maximum contiguous blocks\n");
	fprintf(stderr, "\t-b block size\n");
	fprintf(stderr, "\t-c cylinders/group\n");
	fprintf(stderr, "\t-d rotational delay between contiguous blocks\n");
	fprintf(stderr, "\t-e maximum blocks per file in a cylinder group\n");
	fprintf(stderr, "\t-f frag size\n");
	fprintf(stderr, "\t-i number of bytes per inode\n");
	fprintf(stderr, "\t-k sector 0 skew, per track\n");
	fprintf(stderr, "\t-l hardware sector interleave\n");
	fprintf(stderr, "\t-m minimum free space %%\n");
	fprintf(stderr, "\t-n number of distinguished rotational positions\n");
	fprintf(stderr, "\t-o optimization preference (`space' or `time')\n");
	fprintf(stderr, "\t-p spare sectors per track\n");
	fprintf(stderr, "\t-s file system size (sectors)\n");
	fprintf(stderr, "\t-r revolutions/minute\n");
	fprintf(stderr, "\t-t tracks/cylinder\n");
	fprintf(stderr, "\t-u sectors/track\n");
	fprintf(stderr, "\t-x spare sectors per cylinder\n");
	exit(1);
}
