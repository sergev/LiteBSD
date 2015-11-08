/*
 * Utility for dealing with 2.xBSD filesystem images.
 *
 * Copyright (C) 2006-2011 Serge Vakulenko, <serge@vak.ru>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <getopt.h>
#ifdef __APPLE__
#   include <sys/disk.h>
#endif
#ifdef linux
#   include <linux/fs.h>
#endif

#include "libufs.h"
#include "manifest.h"

int verbose;
static int extract;
static int add;
static int newfs;
static int check;
static int fix;
static int mount;
static int scan;
static int repartition;
static unsigned kbytes;
static unsigned pindex;
static ufs_t disk;

static const char *program_version =
    "BSD 4.4 file system tool, version 0.1\n"
    "Copyright (C) 2014 Serge Vakulenko";

static const char *program_bug_address = "<serge@vak.ru>";

static struct option program_options[] = {
    { "help",        no_argument,       0,  'h' },
    { "version",     no_argument,       0,  'V' },
    { "verbose",     no_argument,       0,  'v' },
    { "add",         no_argument,       0,  'a' },
    { "extract",     no_argument,       0,  'x' },
    { "check",       no_argument,       0,  'c' },
    { "fix",         no_argument,       0,  'f' },
    { "mount",       no_argument,       0,  'm' },
    { "scan",        no_argument,       0,  'S' },
    { "new",         no_argument,       0,  'n' },
    { "manifest",    required_argument, 0,  'M' },
    { "partition",   required_argument, 0,  'p' },
    { "repartition", required_argument, 0,  'r' },
    { "size",        required_argument, 0,  's' },
    { 0 }
};

static void print_help (char *progname)
{
    char *p = strrchr (progname, '/');
    if (p)
        progname = p+1;

    printf ("%s\n", program_version);
    printf ("This program is free software; it comes with ABSOLUTELY NO WARRANTY;\n"
            "see the BSD 3-Clause License for more details.\n");
    printf ("\n");
    printf ("Usage:\n");
    printf ("  %s [--verbose] [--partition=n] disk.img\n", progname);
    printf ("  %s --check [--fix] [--partition=n] disk.img\n", progname);
    printf ("  %s --new [--size=kbytes | --partition=n] [--manifest=file] disk.img [dir]\n", progname);
    printf ("  %s --mount [--partition=n] disk.img dir\n", progname);
    printf ("  %s --add [--partition=n] disk.img files...\n", progname);
    printf ("  %s --extract [--partition=n] disk.img\n", progname);
    printf ("  %s --repartition=format disk.img\n", progname);
    printf ("  %s --scan dir > file\n", progname);
    printf ("\n");
    printf ("Options:\n");
    printf ("  -c, --check         Check filesystem, use -c -f to fix.\n");
    printf ("  -f, --fix           Fix bugs in filesystem.\n");
    printf ("  -n, --new           Create new filesystem; add files from\n");
    printf ("                      specified directory and manifest (optional)\n");
    printf ("  -s NUM, --size=NUM  Size of filesystem in kbytes.\n");
    printf ("  -M file, --manifest=file\n");
    printf ("                      List of files and attributes to create.\n");
    printf ("  -m, --mount         Mount the filesystem.\n");
    printf ("  -a, --add           Add files to filesystem.\n");
    printf ("  -x, --extract       Extract all files.\n");
    printf ("  -r format, --repartition=format\n");
    printf ("                      Install new partition table.\n");
    printf ("  -p NUM, --partition=NUM\n");
    printf ("                      Select a partition.\n");
    printf ("  -S, --scan          Create a manifest from directory contents.\n");
    printf ("  -v, --verbose       Be verbose.\n");
    printf ("  -V, --version       Print version information and then exit.\n");
    printf ("  -h, --help          Print this message.\n");
    printf ("\n");
    printf ("Report bugs to \"%s\".\n", program_bug_address);
}

static void
scan_print (ufs_inode_t *dir, ufs_inode_t *inode,
    const char *dirname, const char *filename, void *arg)
{
    FILE *out = arg;
    char *path;

    ufs_inode_print_path (inode, dirname, filename, out);

    if (verbose > 1) {
        /* Print a list of blocks. */
        ufs_inode_print_blocks (inode, out);
        if (verbose > 2) {
            ufs_inode_print (inode, out);
            printf ("--------\n");
        }
    }
    if ((inode->mode & IFMT) == IFDIR && inode->number != ROOTINO) {
        /* Scan subdirectory. */
        path = alloca (strlen (dirname) + strlen (filename) + 2);
        strcpy (path, dirname);
        strcat (path, "/");
        strcat (path, filename);
        ufs_directory_scan (inode, path, scan_print, arg);
    }
}

static void
extract_inode (ufs_inode_t *inode, char *path)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    int fd, n, mode;
    unsigned long offset;
    unsigned char data [MAXBSIZE];

    /* Allow read/write for user. */
    mode = (inode->mode & 0777) | 0600;
    fd = open (path, O_CREAT | O_RDWR, mode);
    if (fd < 0) {
        perror (path);
        return;
    }
    for (offset = 0; offset < inode->size; offset += bsize) {
        n = inode->size - offset;
        if (n > bsize)
            n = bsize;
        if (ufs_inode_read (inode, offset, data, n) < 0) {
            fprintf (stderr, "%s: read error at offset %ld\n",
                path, offset);
            break;
        }
        if (write (fd, data, n) != n) {
            fprintf (stderr, "%s: write error\n", path);
            break;
        }
    }
    close (fd);
}

static void
extract_symlink (ufs_inode_t *inode, char *path)
{
    unsigned bsize = inode->disk->d_fs.fs_bsize;
    char data [MAXBSIZE];

    if (inode->size >= bsize) {
        fprintf (stderr, "%s: too long symlink\n", path);
        return;
    }
    if (inode->size < inode->disk->d_fs.fs_maxsymlinklen) {
        /* Short symlink is stored in inode. */
        strcpy (data, (char*)inode->daddr);

    } else if (ufs_inode_read (inode, 0, (unsigned char*)data, inode->size) < 0) {
        fprintf (stderr, "%s: error reading symlink\n", path);
        return;
    }
    data[inode->size] = 0;
    if (symlink(data, path) < 0) {
        perror (path);
        return;
    }
}

static void
scan_extract (ufs_inode_t *dir, ufs_inode_t *inode,
    const char *dirname, const char *filename, void *arg)
{
    FILE *out = arg;
    char *path, *relpath;

    if (verbose)
        ufs_inode_print_path (inode, dirname, filename, out);

    if ((inode->mode & IFMT) != IFDIR &&
        (inode->mode & IFMT) != IFREG &&
        (inode->mode & IFMT) != IFLNK)
        return;

    path = alloca (strlen (dirname) + strlen (filename) + 2);
    strcpy (path, dirname);
    strcat (path, "/");
    strcat (path, filename);
    for (relpath=path; *relpath == '/'; relpath++)
        continue;

    switch (inode->mode & IFMT) {
    case IFDIR:
        if (mkdir (relpath, 0775) < 0 && errno != EEXIST)
            perror (relpath);
        /* Scan subdirectory. */
        ufs_directory_scan (inode, path, scan_extract, arg);
        break;
    case IFREG:
        extract_inode (inode, relpath);
        break;
    case IFLNK:
        extract_symlink (inode, relpath);
        break;
    }
}

/*
 * Create a directory.
 */
static void
add_directory (ufs_t *disk, char *name, int mode, int owner, int group)
{
    ufs_inode_t dir, parent;
    char buf [MAXBSIZE], *p;

    /* Open parent directory. */
    strcpy (buf, name);
    p = strrchr (buf, '/');
    if (p)
        *p = 0;
    else
        *buf = 0;
    if (ufs_inode_lookup (disk, &parent, buf) < 0) {
        fprintf (stderr, "%s: cannot open directory\n", buf);
        return;
    }

    /* Create directory. */
    mode &= 07777;
    mode |= IFDIR;
    int done = ufs_inode_create (disk, &dir, name, mode);
    if (done < 0) {
        fprintf (stderr, "%s: directory inode create failed\n", name);
        return;
    }
    if (done == 0) {
        /* The directory already existed. */
        return;
    }
    dir.uid = owner;
    dir.gid = group;
    ufs_inode_save (&dir, 1);

    /* Make parent link '..' */
    strcpy (buf, name);
    strcat (buf, "/..");
    if (ufs_inode_link (disk, &dir, buf, parent.number, IFDIR) < 0) {
        fprintf (stderr, "%s: dotdot link failed\n", name);
        return;
    }
    if (ufs_inode_get (disk, &parent, parent.number) < 0) {
        fprintf (stderr, "inode %d: cannot open parent\n", parent.number);
        return;
    }
    ++parent.nlink;
    ufs_inode_save (&parent, 1);
/*printf ("*** inode %d: increment link counter to %d\n", parent.number, parent.nlink);*/
}

/*
 * Copy regular file to filesystem.
 */
static void
add_file (ufs_t *disk, const char *path, const char *dirname,
    int mode, int owner, int group)
{
    ufs_inode_t inode;
    FILE *fd;
    char accpath [MAXBSIZE];
    unsigned char data [MAXBSIZE];
    unsigned long offset;
    struct stat st;
    int len;

    if (dirname && *dirname) {
        /* Concatenate directory name and file name. */
        strcpy (accpath, dirname);
        len = strlen (accpath);
        if (accpath[len-1] != '/' && path[0] != '/')
            strcat (accpath, "/");
        strcat (accpath, path);
    } else {
        /* Use filename relative to current directory. */
        strcpy (accpath, path);
    }
    fd = fopen (accpath, "r");
    if (! fd) {
        perror (accpath);
        return;
    }
    fstat (fileno(fd), &st);
    if (mode == -1)
        mode = st.st_mode;
    mode &= 07777;
    mode |= IFREG;
    if (ufs_inode_create (disk, &inode, path, mode) < 0) {
        fprintf (stderr, "%s: cannot create\n", path);
        return;
    }
    ufs_inode_truncate (&inode, 0);
    ufs_inode_save (&inode, 0);
    offset = 0;
    for (;;) {
        len = fread (data, 1, disk->d_fs.fs_bsize, fd);
/*      printf ("read %d bytes from %s\n", len, accpath);*/
        if (len < 0)
            perror (accpath);
        if (len <= 0)
            break;
        if (ufs_inode_write (&inode, offset, data, len) < 0) {
            fprintf (stderr, "inode %d: file write failed, %u bytes, offset %lu\n",
                inode.number, len, offset);
            break;
        }
        offset += len;
    }
    inode.uid = owner;
    inode.gid = group;
    inode.mtime = st.st_mtime;
    inode.dirty = 1;
    if (ufs_inode_save (&inode, 0) < 0) {
        fprintf (stderr, "inode %d: file close failed\n", inode.number);
        return;
    }
    fclose (fd);
}

/*
 * Create a device node.
 */
static void
add_device (ufs_t *disk, char *name, int mode, int owner, int group,
    int type, int majr, int minr)
{
    ufs_inode_t dev;

    mode &= 07777;
    mode |= (type == 'b') ? IFBLK : IFCHR;
    if (ufs_inode_create (disk, &dev, name, mode) < 0) {
        fprintf (stderr, "%s: device inode create failed\n", name);
        return;
    }
    dev.daddr[0] = majr << 8 | minr;
    dev.uid = owner;
    dev.gid = group;
    dev.mtime = time(NULL);
    ufs_inode_save (&dev, 1);
}

/*
 * Create a symlink.
 */
static void
add_symlink (ufs_t *disk, const char *path, const char *link,
    int mode, int owner, int group)
{
    ufs_inode_t inode;
    struct fs *fs = &disk->d_fs;
    int len;

    mode &= 07777;
    mode |= IFLNK;
    if (ufs_inode_create (disk, &inode, path, mode) < 0) {
        fprintf (stderr, "%s: cannot create\n", path);
        return;
    }
    ufs_inode_truncate (&inode, 0);
    ufs_inode_save (&inode, 0);

    len = strlen (link);
    if (len < fs->fs_maxsymlinklen) {
        /* Short symlink is stored in inode. */
        strcpy ((char*)inode.daddr, link);
        inode.size = len;

    } else if (ufs_inode_write (&inode, 0, (unsigned char*) link, len) < 0) {
        fprintf (stderr, "inode %d: symlink write failed, %u bytes\n",
            inode.number, len);
        return;
    }
    inode.uid = owner;
    inode.gid = group;
    inode.mtime = time(NULL);
    inode.dirty = 1;
    if (ufs_inode_save (&inode, 0) < 0) {
        fprintf (stderr, "inode %d: symlink close failed\n", inode.number);
        return;
    }
}

/*
 * Create a hard link.
 */
static void
add_hardlink (ufs_t *disk, const char *path, const char *link)
{
    ufs_inode_t source, target;

    /* Find source. */
    if (ufs_inode_lookup (disk, &source, link) < 0) {
        fprintf (stderr, "%s: link source not found\n", link);
        return;
    }
    if ((source.mode & IFMT) == IFDIR) {
        fprintf (stderr, "%s: cannot link directories\n", link);
        return;
    }

    /* Check the target does not exist. */
    if (ufs_inode_lookup (disk, &target, path) >= 0) {
        fprintf (stderr, "%s: link target already exists\n", path);
        return;
    }

    /* Create target link. */
    if (ufs_inode_link (disk, &target, path, source.number, source.mode) < 0) {
        fprintf (stderr, "%s: link failed\n", path);
        return;
    }
    source.nlink++;
    ufs_inode_save (&source, 1);
}

/*
 * Create a file/device/directory in the filesystem.
 * When name is ended by slash as "name/", directory is created.
 */
static void
add_object (ufs_t *disk, char *name)
{
    char *p;

    if (verbose) {
        printf ("%s\n", name);
    }
    p = strrchr (name, '/');
    if (p && p[1] == 0) {
        *p = 0;
        add_directory (disk, name, 0777, 0, 0);
        return;
    }
    add_file (disk, name, 0, -1, 0, 0);
}

/*
 * Add the contents from the specified directory.
 * Use the optional manifest file.
 */
static void
add_contents (ufs_t *disk, const char *dirname, const char *manifest)
{
    manifest_t m;
    void *cursor;
    char *path, *link;
    int filetype, mode, owner, group, majr, minr;
    int ndirs = 0, nfiles = 0, nlinks = 0, nsymlinks = 0, ndevs = 0;

    if (manifest) {
        /* Load manifest from file. */
        if (! manifest_load (&m, manifest)) {
            fprintf (stderr, "%s: cannot read\n", manifest);
            return;
        }
    } else {
        /* Create manifest from directory contents. */
        if (! manifest_scan (&m, dirname)) {
            fprintf (stderr, "%s: cannot read\n", dirname);
            return;
        }
    }

    /* For every file in the manifest,
     * add it to the target filesystem. */
    cursor = 0;
    while ((filetype = manifest_iterate (&m, &cursor, &path, &link, &mode,
        &owner, &group, &majr, &minr)) != 0)
    {
        switch (filetype) {
        case 'd':
            add_directory (disk, path, mode, owner, group);
            ndirs++;
            break;
        case 'f':
            add_file (disk, path, dirname, mode, owner, group);
            nfiles++;
            break;
        case 'l':
            add_hardlink (disk, path, link);
            nlinks++;
            break;
        case 's':
            add_symlink (disk, path, link, mode, owner, group);
            nsymlinks++;
            break;
        case 'b':
            add_device (disk, path, mode, owner, group, 'b', majr, minr);
            ndevs++;
            break;
        case 'c':
            add_device (disk, path, mode, owner, group, 'c', majr, minr);
            ndevs++;
            break;
        }
    }
    if (disk->d_fs.fs_fmod)
        ufs_superblock_write(disk, 0);
    ufs_disk_close (disk);
    printf ("Installed %u directories, %u files, %u devices, %u links, %u symlinks\n",
        ndirs, nfiles, ndevs, nlinks, nsymlinks);
}

void create_partition_table (const char *filename, char *format)
{
    unsigned char buf [512], *entry;
    unsigned pindex, offset;
    int fd, activated = 0;

    /* Initialize an empty partition table. */
    memset (buf, 0, sizeof(buf));
    buf[510] = 0x55;
    buf[511] = 0xaa;

    /* Parse format string and fill partition entries. */
    offset = 2;
    char *p = strtok (format, ":");
    for (pindex=1; p && pindex<=4; pindex++) {
        char *q, *endptr;
        unsigned type, len;

        /* Split into type and length. */
        q = strchr (p, '=');
        if (! q) {
            fprintf (stderr, "%s: wrong format '%s' for partition %u\n",
                filename, p, pindex);
            exit(-1);
        }
        *q++ = 0;

        /* Get length in sectors. */
        len = strtoul (q, &endptr, 0);
        if (*endptr == 'k')
            len *= 2;
        else if (*endptr == 'm' || *endptr == 'M')
            len *= 2*1024;
        else if (*endptr != '\0') {
            fprintf (stderr, "%s: wrong length '%s' for partition %u\n",
                filename, q, pindex);
            exit(-1);
        }

        /* Get type of partition. */
        if (strcmp (p, "fs") == 0)
            type = 0xb7;
        else if (strcmp (p, "swap") == 0)
            type = 0xb8;
        else {
            type = strtoul (p, &endptr, 16);
            if (*endptr != '\0') {
                fprintf (stderr, "%s: wrong type '%s' for partition %u\n",
                    filename, p, pindex);
                exit(-1);
            }
        }
        printf ("Allocated partition %u, type %02x, start %u, size %u sectors\n",
            pindex, type, offset, len);

        /* Fill partition entry. */
        entry = &buf [446 + (pindex-1)*16];
        entry [4] = type;
        *(unsigned*) &entry [8] = offset;
        *(unsigned*) &entry [12] = len;

        /* Make first FS active. */
        if (type == 0xb7 && ! activated) {
            entry [0] = 0x80;
            activated = 1;
        }

        p = strtok (NULL, ":");
        offset += len;
    }

    /* Open or create the file. */
    fd = open (filename, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        fprintf (stderr, "%s: cannot create file\n", filename);
        exit(-1);
    }

    /* Write the partition table to file. */
    if (write (fd, buf, 512) != 512) {
        fprintf (stderr, "%s: error writing partition table\n", filename);
        exit(-1);
    }
}

int main (int argc, char **argv)
{
    int i, key;
    manifest_t m;
    const char *manifest = 0;
    char *partition_format = 0;

    for (;;) {
        key = getopt_long (argc, argv, "vaxmSncfM:s:p:r:",
            program_options, 0);
        if (key == -1)
            break;
        switch (key) {
        case 'v':
            ++verbose;
            break;
        case 'a':
            ++add;
            break;
        case 'x':
            ++extract;
            break;
        case 'n':
            ++newfs;
            break;
        case 'c':
            ++check;
            break;
        case 'f':
            ++fix;
            break;
        case 'm':
            ++mount;
            break;
        case 'S':
            ++scan;
            break;
        case 'M':
            manifest = optarg;
            break;
        case 's':
             kbytes = strtol (optarg, 0, 0);
             break;
        case 'p':
            pindex = strtol (optarg, 0, 0);
            if (pindex < 1 || pindex > 4) {
                fprintf (stderr, "Incorrect partition index %u\n", pindex);
                return -1;
            }
            break;
        case 'r':
            ++repartition;
            partition_format = optarg;
            break;
        case 'V':
            printf ("%s\n", program_version);
            return 0;
        case 'h':
            print_help (argv[0]);
            return 0;
        default:
            print_help (argv[0]);
            return -1;
        }
    }
    i = optind;
    if (extract + newfs + check + add + mount + scan + repartition > 1) {
        print_help (argv[0]);
        return -1;
    }

    if (newfs) {
        /* Create new filesystem. */
        if (i != argc-1 && i != argc-2) {
            print_help (argv[0]);
            return -1;
        }

        if (pindex == 0) {
            /* Create the file. */
            close(open(argv[i], O_RDONLY | O_CREAT | O_TRUNC, 0664));
        }
        if (ufs_disk_open_blank(&disk, argv[i]) < 0 ||
            ufs_disk_reopen_writable(&disk) < 0) {
            fprintf(stderr, "%s: cannot open disk image\n", argv[i]);
            return -1;
        }

        /* Get size and offset from partition table. */
        if (pindex > 0) {
            if (ufs_disk_set_partition (&disk, pindex) < 0)
                return -1;
            kbytes = disk.d_part_nsectors / 2;
        } else if (kbytes == 0) {
            /* Get size of existing file or device. */
            unsigned long long nbytes = 0;

#ifdef __APPLE__
            if (ioctl(disk.d_fd, DKIOCGETBLOCKCOUNT, &nbytes) >= 0) {
                /* For Apple Darwin */
                nbytes *= 512;
            } else
#endif
#ifdef linux
            if (ioctl(disk.d_fd, BLKGETSIZE64, &nbytes) >= 0) {
                /* For Linux */
            } else
#endif
#ifdef DIOCGMEDIASIZE
            if (ioctl(disk.d_fd, DIOCGMEDIASIZE, &nbytes) >= 0) {
                /* For FreeBSD */
            } else
#endif
            {
                /* Get size of existing regular file. */
                struct stat st;

                fstat(disk.d_fd, &st);
                if ((st.st_mode & S_IFMT) != S_IFREG) {
                    fprintf (stderr, "%s: cannot get size of special fileh\n", argv[i]);
                    return -1;
                }
                nbytes = st.st_size;
            }
            kbytes = nbytes / 1024;
        }
        if (kbytes < 64) {
            /* Need at least 16 blocks. */
            fprintf (stderr, "%s: %u kbytes is not enough\n", argv[i], kbytes);
            return -1;
        }

        /* Set size of filesystem. */
        /* TODO: set fssize and part_ofs from the partition */
        mkfs_mediasize = kbytes * 1024;
        mkfs_fssize = mkfs_mediasize / 512;

        /* Set 4k block size, no fragments. */
        mkfs_bsize = 4096;
        mkfs_sectorsize = 512;
        mkfs_realsectorsize = mkfs_bsize;
        mkfs_fsize = mkfs_bsize;
        mkfs_nflag = 1; /* no .snap directory */

        mkfs(&disk, argv[i]);
        if (ufs_superblock_read(&disk) < 0) {
            fprintf(stderr, "%s: cannot read superblock\n", argv[i]);
            return (-1);
        }
        if (i == argc-2) {
            /* Add the contents from the specified directory.
             * Use the optional manifest file. */
            add_contents (&disk, argv[i+1], manifest);
        }
        ufs_disk_close (&disk);
        if (pindex)
            printf ("Created filesystem at partition %u of %s - %u kbytes\n",
                pindex, argv[i], kbytes);
        else
            printf ("Created filesystem %s - %u kbytes\n", argv[i], kbytes);
        return 0;
    }

    if (check) {
        /* Check filesystem for errors, and optionally fix them. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
        if (ufs_disk_open_blank(&disk, argv[i]) == -1 ||
            (fix && ufs_disk_reopen_writable(&disk) == -1)) {
            fprintf(stderr, "%s: cannot open disk image\n", argv[i]);
            return -1;
        }
        /* Get size and offset from partition table. */
        if (pindex > 0 && ufs_disk_set_partition (&disk, pindex) < 0) {
                return -1;
        }
        check_debug = verbose;
        ufs_check (&disk, argv[i], verbose, fix);
        ufs_disk_close (&disk);
        return 0;
    }

    if (scan) {
        /* Create a manifest from directory contents. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
        if (! manifest_scan (&m, argv[i])) {
            fprintf (stderr, "%s: cannot read\n", argv[i]);
            return -1;
        }
        manifest_print (&m);
        return 0;
    }

    if (repartition) {
        /* Install a new partition table. */
        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
        create_partition_table (argv[i], partition_format);
        return 0;
    }

    /* Add or extract or info. */
    if (i >= argc) {
        print_help (argv[0]);
        return -1;
    }
    if (ufs_disk_open (&disk, argv[i], pindex) < 0) {
        fprintf (stderr, "%s: cannot open\n", argv[i]);
        return -1;
    }

    if (extract) {
        /* Extract all files to current directory. */
        ufs_inode_t inode;

        if (i != argc-1) {
            print_help (argv[0]);
            return -1;
        }
        if (ufs_inode_get (&disk, &inode, ROOTINO) < 0) {
            fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
            return -1;
        }
        ufs_directory_scan (&inode, "", scan_extract, (void*) stdout);
        ufs_disk_close (&disk);
        return 0;
    }

    if (add) {
        /* Add files i+1..argc-1 to filesystem. */
        if (i >= argc) {
            print_help (argv[0]);
            return -1;
        }
        while (++i < argc)
            add_object (&disk, argv[i]);

        if (disk.d_fs.fs_fmod)
            ufs_superblock_write(&disk, 0);
        ufs_disk_close (&disk);
        return 0;
    }

    if (mount) {
        /* Mount the filesystem. */
        if (i != argc-2) {
            print_help (argv[0]);
            return -1;
        }
        return ufs_mount (&disk, argv[i+1]);
    }

    /* Print the structure of flesystem. */
    if (i != argc-1) {
        print_help (argv[0]);
        return -1;
    }
    ufs_print (&disk, stdout);
    if (verbose) {
        ufs_inode_t inode;

        if (ufs_inode_get (&disk, &inode, ROOTINO) < 0) {
            fprintf (stderr, "%s: cannot get inode 1\n", argv[i]);
            return -1;
        }
        printf ("--------\n");
        printf ("/\n");
        if (verbose > 1) {
            /* Print a list of blocks. */
            ufs_inode_print_blocks (&inode, stdout);
            if (verbose > 2) {
                ufs_inode_print (&inode, stdout);
                printf ("--------\n");
            }
        }
        ufs_directory_scan (&inode, "", scan_print, (void*) stdout);
    }
    ufs_disk_close (&disk);
    return 0;
}
