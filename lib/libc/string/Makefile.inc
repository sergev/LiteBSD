#	@(#)Makefile.inc	8.1 (Berkeley) 6/4/93

.PATH: ${.CURDIR}/${MACHINE}/string ${.CURDIR}/string

# machine-independent string sources
SRCS+=	memccpy.c strcasecmp.c strcoll.c strdup.c strerror.c \
	strftime.c strmode.c strtok.c strxfrm.c swab.c \
	strlcat.c strlcpy.c strnlen.c

# machine-dependent string sources
.include "${.CURDIR}/${MACHINE}/string/Makefile.inc"

# If no machine specific bzero(3), build one out of memset(3).
.if empty(SRCS:Mbzero.s)
OBJS+=	bzero.o
bzero.o: memset.c
	${MIPS_GCC_PREFIX}${CC} -DBZERO ${CCROSS} ${CFLAGS} -c ${.ALLSRC} -o ${.TARGET}

bzero.po: memset.c
	${MIPS_GCC_PREFIX}${CC} -DBZERO ${CCROSS} ${CFLAGS} -c -p ${.ALLSRC} -o ${.TARGET}
.endif

# If no machine specific memmove(3), build one out of bcopy(3).
.if empty(SRCS:Mmemmove.s)
OBJS+=	memmove.o
memmove.o: bcopy.c
	${MIPS_GCC_PREFIX}${CC} -DMEMMOVE ${CCROSS} ${CFLAGS} -c ${.ALLSRC} -o ${.TARGET}

memmove.po: bcopy.c
	${MIPS_GCC_PREFIX}${CC} -DMEMMOVE ${CCROSS} ${CFLAGS} -c -p ${.ALLSRC} -o ${.TARGET}
.endif

# If no machine specific memcpy(3), build one out of bcopy(3).
.if empty(SRCS:Mmemcpy.s)
OBJS+=	memcpy.o
memcpy.o: bcopy.c
	${MIPS_GCC_PREFIX}${CC} -DMEMCOPY ${CCROSS} ${CFLAGS} -c ${.ALLSRC} -o ${.TARGET}

memcpy.po: bcopy.c
	${MIPS_GCC_PREFIX}${CC} -DMEMCOPY ${CCROSS} ${CFLAGS} -c -p ${.ALLSRC} -o ${.TARGET}
.endif

# If no machine specific strchr(3), build one out of index(3).
.if empty(SRCS:Mstrchr.s)
OBJS+=	strchr.o
strchr.o: index.c
	${MIPS_GCC_PREFIX}${CC} -DSTRCHR ${CCROSS} ${CFLAGS} -c ${.ALLSRC} -o ${.TARGET}

strchr.po: index.c
	${MIPS_GCC_PREFIX}${CC} -DSTRCHR ${CCROSS} ${CFLAGS} -c -p ${.ALLSRC} -o ${.TARGET}
.endif

# If no machine specific strrchr(3), build one out of rindex(3).
.if empty(SRCS:Mstrrchr.s)
OBJS+=	strrchr.o
strrchr.o: rindex.c
	${MIPS_GCC_PREFIX}${CC} -DSTRRCHR ${CCROSS} ${CFLAGS} -c ${.ALLSRC} -o ${.TARGET}

strrchr.po: rindex.c
	${MIPS_GCC_PREFIX}${CC} -DSTRRCHR ${CCROSS} ${CFLAGS} -c -p ${.ALLSRC} -o ${.TARGET}
.endif

MAN3+=	bcmp.0 bcopy.0 bstring.0 bzero.0 ffs.0 index.0 memccpy.0 memchr.0 \
	memcmp.0 memcpy.0 memmove.0 memset.0 rindex.0 strcasecmp.0 strcat.0 \
	strchr.0 strcmp.0 strcoll.0 strcpy.0 strcspn.0 strftime.0 string.0 \
	strlen.0 strmode.0 strdup.0 strerror.0 strpbrk.0 strrchr.0 strsep.0 \
	strspn.0 strstr.0 strtok.0 strxfrm.0 swab.0

MLINKS+=strcasecmp.3 strncasecmp.3
MLINKS+=strcat.3 strncat.3
MLINKS+=strcmp.3 strncmp.3
MLINKS+=strcpy.3 strncpy.3
MLINKS+=strerror.3 perror.3
