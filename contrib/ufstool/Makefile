# $FreeBSD$

PROGS           = ufstool

OBJS_UFSTOOL    = ufstool.o manifest.o mount.o
OBJS_LIBUFS     = block.o cgroup.o disk.o inode.o sblock.o \
                  bitmap.o mkfs.o check.o check_suj.o

CC              = cc
CFLAGS          = -O1 -g -Wall -Werror
LDFLAGS         = -g
LIBS            = libufs.a

# Fuse
MOUNT_CFLAGS    = `pkg-config --cflags fuse`
LIBS            += `pkg-config --libs fuse`

all:            $(PROGS)

ufstool:        libufs.a $(OBJS_UFSTOOL)
		$(CC) $(LDFLAGS) $(OBJS_UFSTOOL) $(LIBS) -o $@

libufs.a:       $(OBJS_LIBUFS)
		@rm -f $@
		$(AR) cr $@ $(OBJS_LIBUFS)

mount.o:        mount.c libufs.h fs.h
		$(CC) $(CFLAGS) $(MOUNT_CFLAGS) -c -o $@ $<

clean:
		rm -f *.o *.a $(PROGS)
###
bitmap.o: bitmap.c fs.h dinode.h
block.o: block.c libufs.h fs.h dinode.h internal.h
cgroup.o: cgroup.c libufs.h fs.h dinode.h internal.h
check.o: check.c dir.h fs.h dinode.h libufs.h internal.h
check_suj.o: check_suj.c dir.h libufs.h fs.h dinode.h internal.h
disk.o: disk.c libufs.h fs.h dinode.h internal.h
inode.o: inode.c libufs.h fs.h dinode.h dir.h internal.h
manifest.o: manifest.c libufs.h fs.h dinode.h manifest.h
mkfs.o: mkfs.c dir.h libufs.h fs.h dinode.h internal.h
mount.o: mount.c libufs.h fs.h dinode.h dir.h
sblock.o: sblock.c libufs.h fs.h dinode.h internal.h
ufstool.o: ufstool.c libufs.h fs.h dinode.h manifest.h
