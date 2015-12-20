/*
 *		bsd (4.2, others?), Sun (3.2, ?) and Ultrix-32 (?) file I/O.
 */
#include	"def.h"

static	FILE	*ffp;
extern	char	*getenv(), *strncpy();
char	*adjustname();

/*
 * Open a file for reading.
 */
ffropen(fn) char *fn; {
	if ((ffp=fopen(fn, "r")) == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */
ffwopen(fn) char *fn; {
	if ((ffp=fopen(fn, "w")) == NULL) {
		ewprintf("Cannot open file for writing");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Close a file.
 * Should look at the status.
 */
ffclose() {
	(VOID) fclose(ffp);
	return (FIOSUC);
}

/*
 * Write a buffer to the already
 * opened file. bp points to the
 * buffer. Return the status.
 * Check only at the newline and
 * end of buffer.
 */
ffputbuf(bp)
BUFFER *bp;
{
    register char *cp;
    register char *cpend;
    register LINE *lp;
    register LINE *lpend;

    lpend = bp->b_linep;
    lp = lforw(lpend);
    do {
	cp = &ltext(lp)[0];		/* begining of line	*/
	cpend = &cp[llength(lp)];	/* end of line		*/
	while(cp != cpend) {
	    putc(*cp, ffp);
	    cp++;	/* putc may evalualte arguments more than once */
	}
	lp = lforw(lp);
	if(lp == lpend) break;		/* no implied newline on last line */
	putc('\n', ffp);
    } while(!ferror(ffp));
    if(ferror(ffp)) {
	ewprintf("Write I/O error");
	return FIOERR;
    }
    return FIOSUC;
}

/*
 * Read a line from a file, and store the bytes
 * in the supplied buffer. Stop on end of file or end of
 * line.  When FIOEOF is returned, there is a valid line
 * of data without the normally implied \n.
 */
ffgetline(buf, nbuf, nbytes)
register char	*buf;
register int	nbuf;
register int	*nbytes;
{
	register int	c;
	register int	i;

	i = 0;
	while((c = getc(ffp))!=EOF && c!='\n') {
		buf[i++] = c;
		if (i >= nbuf) return FIOLONG;
	}
	if (c == EOF  && ferror(ffp) != FALSE) {
		ewprintf("File read error");
		return FIOERR;
	}
	*nbytes = i;
	return c==EOF ? FIOEOF : FIOSUC;
}

#ifndef NO_BACKUP
/*
 * Rename the file "fname" into a backup
 * copy. On Unix the backup has the same name as the
 * original file, with a "~" on the end; this seems to
 * be newest of the new-speak. The error handling is
 * all in "file.c". The "unlink" is perhaps not the
 * right thing here; I don't care that much as
 * I don't enable backups myself.
 */
fbackupfile(fn) char *fn; {
	register char	*nname;

	if ((nname=malloc((unsigned)(strlen(fn)+1+1))) == NULL) {
		ewprintf("Can't get %d bytes", strlen(fn) + 1);
		return (ABORT);
	}
	(void) strcpy(nname, fn);
	(void) strcat(nname, "~");
	(void) unlink(nname);			/* Ignore errors.	*/
	if (rename(fn, nname) < 0) {
		free(nname);
		return (FALSE);
	}
	free(nname);
	return (TRUE);
}
#endif

/*
 * The string "fn" is a file name.
 * Perform any required appending of directory name or case adjustments.
 * If NO_DIR is not defined, the same file should be refered to even if the
 * working directory changes.
 */
#ifdef SYMBLINK
#include <sys/types.h>
#include <sys/stat.h>
#ifndef MAXSYMLINKS
#define MAXSYMLINKS 8		/* maximum symbolic links to follow */
#endif
#endif
#include <pwd.h>
#ifndef NO_DIR
extern char *wdir;
#endif

char *adjustname(fn)
register char *fn;
{
    register char *cp;
    static char fnb[NFILEN];
    struct passwd *pwent;
#ifdef	SYMBLINK
    struct stat statbuf;
    int i, j;
    char linkbuf[NFILEN];
#endif

    switch(*fn) {
    	case '/':
	    cp = fnb;
	    *cp++ = *fn++;
	    break;
	case '~':
	    fn++;
	    if(*fn == '/' || *fn == '\0') {
		(VOID) strcpy(fnb, getenv("HOME"));
		cp = fnb + strlen(fnb);
	    	if(*fn) fn++;
		break;
	    } else {
		cp = fnb;
		while(*fn && *fn != '/') *cp++ = *fn++;
		*cp = '\0';
		if((pwent = getpwnam(fnb)) != NULL) {
		    (VOID) strcpy(fnb, pwent->pw_dir);
		    cp = fnb + strlen(fnb);
		    break;
		} else {
		    fn -= strlen(fnb) + 1;
		    /* can't find ~user, continue to default case */
		}
	    }
	default:
#ifndef	NODIR
	    strcpy(fnb, wdir);
	    cp = fnb + strlen(fnb);
	    break;
#else
	    return fn;				/* punt */
#endif
    }
    if(cp != fnb && cp[-1] != '/') *cp++ = '/';
    while(*fn) {
    	switch(*fn) {
	    case '.':
		switch(fn[1]) {
	            case '\0':
		    	*--cp = '\0';
		    	return fnb;
	    	    case '/':
	    	    	fn += 2;
		    	continue;
		    case '.':
		    	if(fn[2]=='/' || fn[2] == '\0') {
#ifdef SYMBLINK
			    cp[-1] = '\0';
			    for(j = MAXSYMLINKS; j-- && 
			    	    lstat(fnb, &statbuf) != -1 && 
			    	    (statbuf.st_mode&S_IFMT) == S_IFLNK &&
			    	    (i = readlink(fnb, linkbuf, sizeof linkbuf))
				    != -1 ;) {
				if(linkbuf[0] != '/') {
				    --cp;
				    while(cp > fnb && *--cp != '/') {}
				    ++cp;
				    (VOID) strncpy(cp, linkbuf, i);
				    cp += i;
				} else {
				    (VOID) strncpy(fnb, linkbuf, i);
				    cp = fnb + i;
				}
				if(cp[-1]!='/') *cp++ = '\0';
				else cp[-1] = '\0';
			    }
			    cp[-1] = '/';
#endif
			    --cp;
			    while(cp > fnb && *--cp != '/') {}
			    ++cp;
			    if(fn[2]=='\0') {
			        *--cp = '\0';
			        return fnb;
			    }
		            fn += 3;
		            continue;
		        }
		        break;
		    default:
		    	break;
	        }
		break;
	    case '/':
	    	fn++;
	    	continue;
	    default:
	    	break;
	}
	while(*fn && (*cp++ = *fn++) != '/') {}
    }
    if(cp[-1]=='/') --cp;
    *cp = '\0';
    return fnb;
}

#ifndef NO_STARTUP
#include <sys/file.h>
/*
 * Find a startup file for the user and return its name. As a service
 * to other pieces of code that may want to find a startup file (like
 * the terminal driver in particular), accepts a suffix to be appended
 * to the startup file name.
 */
char *
startupfile(suffix)
char *suffix;
{
	register char	*file;
	static char	home[NFILEN];
	char		*getenv();

	if ((file = getenv("HOME")) == NULL) goto notfound;
	if (strlen(file)+7 >= NFILEN - 1) goto notfound;
	(VOID) strcpy(home, file);
	(VOID) strcat(home, "/.mg");
	if (suffix != NULL) {
		(VOID) strcat(home, "-");
		(VOID) strcat(home, suffix);
	}
	if (access(home, F_OK ) == 0) return home;

notfound:
#ifdef	STARTUPFILE
	file = STARTUPFILE;
	if (suffix != NULL) {
		(VOID) strcpy(home, file);
		(VOID) strcat(home, "-");
		(VOID) strcat(home, suffix);
		file = home;
	}
	if (access(file, F_OK ) == 0) return file;
#endif

	return NULL;
}
#endif

#ifndef NO_DIRED
#include <sys/wait.h>
#include <errno.h>
#include "kbd.h"

/* From OpenBSD mg */

int
copy(char *frname, char *toname)
{
    int ifd, ofd;
    char buf[BUFSIZ];
    mode_t fmode = DEFFILEMODE;		/* XXX?? */
    struct stat orig;
    ssize_t sr;

    if ((ifd = open(frname, O_RDONLY)) == -1)
	return (FALSE);
    if (fstat(ifd, &orig) == -1) {
	ewprintf("fstat: %s", strerror(errno));
	close(ifd);
	return (FALSE);
    }

    if ((ofd = open(toname, O_WRONLY|O_CREAT|O_TRUNC, fmode)) == -1) {
	close(ifd);
	return (FALSE);
    }
    while ((sr = read(ifd, buf, sizeof(buf))) > 0) {
	if (write(ofd, buf, (size_t)sr) != sr) {
	    ewprintf("write error: %s", strerror(errno));
	    break;
	}
    }
    if (fchmod(ofd, orig.st_mode) == -1)
	ewprintf("Cannot set original mode: %s", strerror(errno));

    if (sr == -1) {
	ewprintf("Read error: %s", strerror(errno));
	close(ifd);
	close(ofd);
	return (FALSE);
    }
    /*
     * It is "normal" for this to fail since we can't guarantee that
     * we will be running as root.
     */
    if (fchown(ofd, orig.st_uid, orig.st_gid) && errno != EPERM)
	ewprintf("Cannot set owner: %s", strerror(errno));

    (void) close(ifd);
    (void) close(ofd);

    return (TRUE);
}

BUFFER *dired_(dirname)
char *dirname;
{
    register BUFFER *bp;
    char line[256];
    BUFFER *findbuffer();
    FILE *dirpipe;
    FILE *popen();

    if((dirname = adjustname(dirname)) == NULL) {
	ewprintf("Bad directory name");
	return NULL;
    }
    if(dirname[strlen(dirname)-1] != '/') (VOID) strcat(dirname, "/");
    if((bp = findbuffer(dirname)) == NULL) {
	ewprintf("Could not create buffer");
	return NULL;
    }
    if(bclear(bp) != TRUE) return FALSE;
    (VOID) strcpy(line, "ls -al ");
    (VOID) strcpy(&line[7], dirname);
    if((dirpipe = popen(line, "r")) == NULL) {
	ewprintf("Problem opening pipe to ls");
	return NULL;
    }
    line[0] = line[1] = ' ';
    while(fgets(&line[2], 254, dirpipe) != NULL) {
	line[strlen(line) - 1] = '\0';		/* remove ^J	*/
	(VOID) addline(bp, line);
    }
    if(pclose(dirpipe) == -1) {
	ewprintf("Problem closing pipe to ls");
	return NULL;
    }
    bp->b_dotp = lforw(bp->b_linep);		/* go to first line */
    (VOID) strncpy(bp->b_fname, dirname, NFILEN);
    if((bp->b_modes[0] = name_mode("dired")) == NULL) {
	bp->b_modes[0] = &map_table[0];
	ewprintf("Could not find mode dired");
	return NULL;
    }
    bp->b_nmodes = 0;
    return bp;
}

d_makename(lp, fn)
register LINE *lp;
register char *fn;
{
    register char *cp;

    if(llength(lp) <= 47) return ABORT;
    (VOID) strcpy(fn, curbp->b_fname);
    cp = fn + strlen(fn);
    bcopy(&lp->l_text[47], cp, llength(lp) - 47);
    cp[llength(lp) - 47] = '\0';
    return lgetc(lp, 2) == 'd';
}
#endif
