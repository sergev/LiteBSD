#include <stdlib.h>

#define	EEXIT	8		/* Standard error exit. */

/* Calculate (bytes / DEV_BSIZE) */
#define bytes_to_sectors(x) ((x) >> 9)

#ifndef _SYS_QUEUE_H_
/*
 * Copied from <sys/queue.h>.
 */
#define	TAILQ_HEAD(name, type)						\
struct name {								\
	struct type *tqh_first;	/* first element */			\
	struct type **tqh_last;	/* addr of last element */		\
}

#define	TAILQ_ENTRY(type)						\
struct {								\
	struct type *tqe_next;	/* next element */			\
	struct type **tqe_prev;	/* address of previous element */	\
}

#define	TAILQ_INIT(head) do {						\
	(head)->tqh_first = NULL;					\
	(head)->tqh_last = &(head)->tqh_first;				\
} while (/*CONSTCOND*/0)

#define	TAILQ_INSERT_HEAD(head, elm, field) do {			\
	if (((elm)->field.tqe_next = (head)->tqh_first) != NULL)	\
		(head)->tqh_first->field.tqe_prev =			\
		    &(elm)->field.tqe_next;				\
	else								\
		(head)->tqh_last = &(elm)->field.tqe_next;		\
	(head)->tqh_first = (elm);					\
	(elm)->field.tqe_prev = &(head)->tqh_first;			\
} while (/*CONSTCOND*/0)

#define	TAILQ_INSERT_TAIL(head, elm, field) do {			\
	(elm)->field.tqe_next = NULL;					\
	(elm)->field.tqe_prev = (head)->tqh_last;			\
	*(head)->tqh_last = (elm);					\
	(head)->tqh_last = &(elm)->field.tqe_next;			\
} while (/*CONSTCOND*/0)

#define	TAILQ_REMOVE(head, elm, field) do {				\
	if (((elm)->field.tqe_next) != NULL)				\
		(elm)->field.tqe_next->field.tqe_prev = 		\
		    (elm)->field.tqe_prev;				\
	else								\
		(head)->tqh_last = (elm)->field.tqe_prev;		\
	*(elm)->field.tqe_prev = (elm)->field.tqe_next;			\
} while (/*CONSTCOND*/0)

#define	TAILQ_EMPTY(head)	((head)->tqh_first == NULL)
#define	TAILQ_FIRST(head)	((head)->tqh_first)
#define	TAILQ_NEXT(elm, field)	((elm)->field.tqe_next)

#define	TAILQ_LAST(head, headname)					\
	(*(((struct headname *)((head)->tqh_last))->tqh_last))

#define	TAILQ_PREV(elm, headname, field)				\
	(*(((struct headname *)((elm)->field.tqe_prev))->tqh_last))

#define	TAILQ_FOREACH(var, head, field)					\
	for ((var) = TAILQ_FIRST((head));				\
	    (var);							\
	    (var) = TAILQ_NEXT((var), field))

#define	TAILQ_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = TAILQ_FIRST((head));				\
	    (var) && ((tvar) = TAILQ_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	TAILQ_FOREACH_REVERSE(var, head, headname, field)		\
	for ((var) = (*(((struct headname *)((head)->tqh_last))->tqh_last));	\
		(var);							\
		(var) = (*(((struct headname *)((var)->field.tqe_prev))->tqh_last)))

#define	TAILQ_FOREACH_REVERSE_SAFE(var, head, headname, field, tvar)	\
	for ((var) = TAILQ_LAST((head), headname);			\
	    (var) && ((tvar) = TAILQ_PREV((var), headname, field), 1);	\
	    (var) = (tvar))

#define	LIST_ENTRY(type)						\
struct {								\
	struct type *le_next;	/* next element */			\
	struct type **le_prev;	/* address of previous next element */	\
}

/*
 * List definitions.
 */
#define	LIST_HEAD(name, type)						\
struct name {								\
	struct type *lh_first;	/* first element */			\
}

#define	LIST_HEAD_INITIALIZER(head)					\
	{ NULL }

#define	LIST_FIRST(head)	((head)->lh_first)

#define	LIST_NEXT(elm, field)	((elm)->field.le_next)

#define	LIST_FOREACH(var, head, field)					\
	for ((var) = ((head)->lh_first);				\
		(var);							\
		(var) = ((var)->field.le_next))

#define	LIST_FOREACH_SAFE(var, head, field, tvar)			\
	for ((var) = LIST_FIRST((head));				\
	    (var) && ((tvar) = LIST_NEXT((var), field), 1);		\
	    (var) = (tvar))

#define	LIST_INSERT_HEAD(head, elm, field) do {				\
	if (((elm)->field.le_next = (head)->lh_first) != NULL)		\
		(head)->lh_first->field.le_prev = &(elm)->field.le_next;\
	(head)->lh_first = (elm);					\
	(elm)->field.le_prev = &(head)->lh_first;			\
} while (/*CONSTCOND*/0)

#define	LIST_REMOVE(elm, field) do {					\
	if ((elm)->field.le_next != NULL)				\
		(elm)->field.le_next->field.le_prev = 			\
		    (elm)->field.le_prev;				\
	*(elm)->field.le_prev = (elm)->field.le_next;			\
} while (/*CONSTCOND*/0)

#endif /* _SYS_QUEUE_H_ */

/*
 * buffer cache structure.
 */
struct bufarea {
	TAILQ_ENTRY(bufarea) b_list;		/* buffer list */
	ufs2_daddr_t b_bno;
	int b_size;
	int b_errs;
	int b_flags;
	int b_type;
	union {
		char *b_buf;			/* buffer space */
		ufs1_daddr_t *b_indir1;		/* UFS1 indirect block */
		ufs2_daddr_t *b_indir2;		/* UFS2 indirect block */
		struct fs *b_fs;		/* super block */
		struct cg *b_cg;		/* cylinder group */
		struct ufs1_dinode *b_dinode1;	/* UFS1 inode block */
		struct ufs2_dinode *b_dinode2;	/* UFS2 inode block */
	} b_un;
	char b_dirty;
};

#define	IBLK(bp, i) \
	((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) ? \
	(bp)->b_un.b_indir1[i] : (bp)->b_un.b_indir2[i])

#define IBLK_SET(bp, i, val) do { \
	if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) \
		(bp)->b_un.b_indir1[i] = (val); \
	else \
		(bp)->b_un.b_indir2[i] = (val); \
	} while (0)

/*
 * Buffer flags
 */
#define	B_INUSE 	0x00000001	/* Buffer is in use */
/*
 * Type of data in buffer
 */
#define	BT_UNKNOWN 	 0	/* Buffer holds a superblock */
#define	BT_SUPERBLK 	 1	/* Buffer holds a superblock */
#define	BT_CYLGRP 	 2	/* Buffer holds a cylinder group map */
#define	BT_LEVEL1 	 3	/* Buffer holds single level indirect */
#define	BT_LEVEL2 	 4	/* Buffer holds double level indirect */
#define	BT_LEVEL3 	 5	/* Buffer holds triple level indirect */
#define	BT_EXTATTR 	 6	/* Buffer holds external attribute data */
#define	BT_INODES 	 7	/* Buffer holds external attribute data */
#define	BT_DIRDATA 	 8	/* Buffer holds directory data */
#define	BT_DATA	 	 9	/* Buffer holds user data */
#define BT_NUMBUFTYPES	10
#define BT_NAMES {			\
	"unknown",			\
	"Superblock",			\
	"Cylinder Group",		\
	"Single Level Indirect",	\
	"Double Level Indirect",	\
	"Triple Level Indirect",	\
	"External Attribute",		\
	"Inode Block",			\
	"Directory Contents",		\
	"User Data" }

#define	dirty(bp) do { \
	if (check_fswritefd < 0) \
		check_fatal("SETTING DIRTY FLAG IN READ_ONLY MODE\n"); \
	else \
		(bp)->b_dirty = 1; \
} while (0)
#define	initbarea(bp, type) do { \
	(bp)->b_dirty = 0; \
	(bp)->b_bno = (ufs2_daddr_t)-1; \
	(bp)->b_flags = 0; \
	(bp)->b_type = type; \
} while (0)

enum fixstate {DONTKNOW, NOFIX, FIX, IGNORE};

struct inodesc {
	enum fixstate id_fix;	/* policy on fixing errors */
	int (*id_func)(struct inodesc *);
				/* function to be applied to blocks of inode */
	ufs_ino_t id_number;	/* inode number described */
	ufs_ino_t id_parent;	/* for DATA nodes, their parent */
	ufs_lbn_t id_lbn;	/* logical block number of current block */
	ufs2_daddr_t id_blkno;	/* current block number being examined */
	int id_numfrags;	/* number of frags contained in block */
	int64_t id_filesize;	/* for DATA nodes, the size of the directory */
	ufs2_daddr_t id_entryno;/* for DATA nodes, current entry number */
	int id_loc;		/* for DATA nodes, current location in dir */
	struct direct *id_dirp;	/* for DATA nodes, ptr to current entry */
	char *id_name;		/* for DATA nodes, name to find or enter */
	char id_type;		/* type of descriptor, DATA or ADDR */
};

/*
 * Inode state information is contained on per cylinder group lists
 * which are described by the following structure.
 */
struct inostatlist {
	long	il_numalloced;	/* number of inodes allocated in this cg */
	struct inostat *il_stat;/* inostat info for this cylinder group */
};

/*
 * Each inode on the file system is described by the following structure.
 * The linkcnt is initially set to the value in the inode. Each time it
 * is found during the descent in passes 2, 3, and 4 the count is
 * decremented. Any inodes whose count is non-zero after pass 4 needs to
 * have its link count adjusted by the value remaining in ino_linkcnt.
 */
struct inostat {
	char	ino_state;	/* state of inode, see below */
	char	ino_type;	/* type of inode */
	short	ino_linkcnt;	/* number of links not found */
};

/*
 * Inode states.
 */
#define	USTATE	0x1		/* inode not allocated */
#define	FSTATE	0x2		/* inode is file */
#define	FZLINK	0x3		/* inode is file with a link count of zero */
#define	DSTATE	0x4		/* inode is directory */
#define	DZLINK	0x5		/* inode is directory with a zero link count  */
#define	DFOUND	0x6		/* directory found during descent */
/*     		0x7		   UNUSED - see S_IS_DVALID() definition */
#define	DCLEAR	0x8		/* directory is to be cleared */
#define	FCLEAR	0x9		/* file is to be cleared */
/*     	DUNFOUND === (state == DSTATE || state == DZLINK) */
#define	S_IS_DUNFOUND(state)	(((state) & ~0x1) == DSTATE)
/*     	DVALID   === (state == DSTATE || state == DZLINK || state == DFOUND) */
#define	S_IS_DVALID(state)	(((state) & ~0x3) == DSTATE)
#define	INO_IS_DUNFOUND(ino)	S_IS_DUNFOUND(inoinfo(ino)->ino_state)
#define	INO_IS_DVALID(ino)	S_IS_DVALID(inoinfo(ino)->ino_state)

/* file types */
#define	DATA	1	/* a directory */
#define	SNAP	2	/* a snapshot */
#define	ADDR	3	/* anything but a directory or a snapshot */

union dinode {
	struct ufs1_dinode dp1;
	struct ufs2_dinode dp2;
};
#define	DIP(dp, field) \
	((check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) ? \
	(dp)->dp1.field : (dp)->dp2.field)

#define DIP_SET(dp, field, val) do { \
	if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) \
		(dp)->dp1.field = (val); \
	else \
		(dp)->dp2.field = (val); \
	} while (0)

/*
 * Linked list of duplicate blocks.
 *
 * The list is composed of two parts. The first part of the
 * list (from duplist through the node pointed to by muldup)
 * contains a single copy of each duplicate block that has been
 * found. The second part of the list (from muldup to the end)
 * contains duplicate blocks that have been found more than once.
 * To check if a block has been found as a duplicate it is only
 * necessary to search from duplist through muldup. To find the
 * total number of times that a block has been found as a duplicate
 * the entire list must be searched for occurrences of the block
 * in question. The following diagram shows a sample list where
 * w (found twice), x (found once), y (found three times), and z
 * (found once) are duplicate block numbers:
 *
 *    w -> y -> x -> z -> y -> w -> y
 *    ^		     ^
 *    |		     |
 * duplist	  muldup
 */
struct dups {
	struct dups *next;
	ufs2_daddr_t dup;
};

/*
 * Inode cache data structures.
 */
struct inoinfo {
	struct inoinfo  *i_nexthash;	/* next entry in hash chain */
	ufs_ino_t	i_number;	/* inode number of this entry */
	ufs_ino_t	i_parent;	/* inode number of parent */
	ufs_ino_t	i_dotdot;	/* inode number of `..' */
	size_t          i_isize;	/* size of inode */
	u_int           i_numblks;	/* size of block array in bytes */
	ufs2_daddr_t    i_blks[1];	/* actually longer */
};

#define	setbmap(blkno)	setbit(blockmap, blkno)
#define	testbmap(blkno)	isset(blockmap, blkno)
#define	clrbmap(blkno)	clrbit(blockmap, blkno)

#define	clearinode(dp) \
	if (check_sblk.b_un.b_fs->fs_magic == FS_UFS1_MAGIC) { \
		(dp)->dp1 = ufs1_zino; \
	} else { \
		(dp)->dp2 = ufs2_zino; \
	}

#define	STOP	0x01
#define	SKIP	0x02
#define	KEEPON	0x04
#define	ALTERED	0x08
#define	FOUND	0x10

int		check_readsb(int listerr);
int		check_setup(const char *dev, int part_num);
int             check_blread(int fd, char *buf, ufs2_daddr_t blk, long size);
int             check_changeino(ufs_ino_t dir, const char *name, ufs_ino_t newnum);
int             check_findino(struct inodesc *idesc);
int             check_flushentry(void);
int             check_inode(union dinode *dp, struct inodesc *idesc);
int             check_makeentry(ufs_ino_t parent, ufs_ino_t ino, const char *name);
int             check_reply(const char *question);
int             check_suj(const char *filesys, off_t offset);
struct bufarea *check_cgget(int cg);
struct bufarea *check_getdatablk(ufs2_daddr_t blkno, long size, int type);
union dinode   *check_ginode(ufs_ino_t inumber);
void		check_catch(int);
void		check_catchquit(int);
void		check_pass1(void);
void		check_pass1b(void);
void		check_pass2(void);
void		check_pass3(void);
void		check_pass4(void);
void		check_pass5(void);
void		check_sblock_init(void);
void            check_blwrite(int fd, char *buf, ufs2_daddr_t blk, ssize_t size);
void            check_fatal(const char *fmt, ...);
void            check_finalstats(void);
void            check_finish(int markclean);
void            check_getblk(struct bufarea *bp, ufs2_daddr_t blk, long size);
void            check_gjournal(const char *filesys);
void            check_inocleanup(void);
void            check_inodirty(void);
void            check_stats(char *what);
void            check_warn(const char *fmt, ...);

char            check_clean;		/* only do work if not cleanly unmounted */
char            check_nflag;		/* assume a no response */
char            check_preen;		/* just fix normal inconsistencies */
char            check_rerun;		/* rerun fsck. Only used in non-preen mode */
char            check_resolved;		/* cleared if unresolved changes => not clean */
char            check_skipclean;	/* skip clean file systems if preening */
char            check_usedsoftdep;	/* just fix soft dependency inconsistencies */
char            check_yflag;		/* assume a yes response */
const char      *check_filename;        /* name of device being checked */
ufs_ino_t       check_maxino;		/* number of inodes in file system */
ufs_ino_t       check_n_files;		/* number of files in use */
int             check_Eflag;		/* delete empty data blocks */
int             check_Zflag;		/* zero empty data blocks */
int             check_bflag;		/* location of alternate super block */
int             check_cvtlevel;		/* convert to newer file system format */
int             check_debug;            /* output debugging info */
int             check_fsmodified;	/* 1 => write done to file system */
int             check_fsreadfd;		/* file descriptor for reading file system */
int             check_fswritefd;	/* file descriptor for writing file system */
int             check_inoopt;		/* trim out unused inodes */
int             check_lfmode;		/* lost & found directory creation mode */
int             check_returntosingle;	/* 1 => return to single user mode on exit */
int             check_surrender;	/* Give up if reads fail */
long            check_secsize;		/* actual disk sector size */
struct	ufs1_dinode ufs1_zino;
struct	ufs2_dinode ufs2_zino;
struct bufarea  check_sblk;             /* file system superblock */
struct dups     *check_duplist;         /* head of dup list */
struct dups     *check_muldup;          /* end of unique duplicate dup block numbers */
struct inostatlist *check_inostathead;
u_int           check_real_dev_bsize;   /* actual disk sector size, not overriden */
ufs2_daddr_t    check_maxfsblock;	/* number of blocks in the file system */
ufs2_daddr_t    check_n_blks;		/* number of blocks in use */

/*
 * Wrapper for malloc() that flushes the cylinder group cache to try
 * to get space.
 */
static inline void*
Malloc(int size)
{
	void *retval;

	while ((retval = malloc(size)) == NULL)
		if (check_flushentry() == 0)
			break;
	return (retval);
}

/*
 * Wrapper for calloc() that flushes the cylinder group cache to try
 * to get space.
 */
static inline void*
Calloc(int cnt, int size)
{
	void *retval;

	while ((retval = calloc(cnt, size)) == NULL)
		if (check_flushentry() == 0)
			break;
	return (retval);
}
