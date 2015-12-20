PROG=	ld

SRCS=	ld_arch.c		\
	ld_dynamic.c		\
	ld_ehframe.c		\
	ld_error.c 		\
	ld_exp.c		\
	ld_file.c		\
	ld_hash.c		\
	ld_input.c		\
	ld_layout.c		\
	ld_main.c 		\
	ld_options.c		\
	ld_output.c		\
	ld_path.c		\
	ld_reloc.c		\
	ld_script.c		\
	ld_script_lexer.c	\
	ld_script_parser.c	\
	ld_strtab.c		\
	ld_symbols.c		\
	ld_symver.c		\
	mips.c			\
	littlemips_script.c	\
	bigmips_script.c

.SUFFIXES:	.ld .c
.ld.c:
	awk -f ld_script.awk ${.ALLSRC} > ${.TARGET}

ld_script_lexer.l: ld_script_parser.c

ld_script_parser.c y.tab.h: ld_script_parser.y
	$(YACC) -d ld_script_parser.y
	mv y.tab.c ld_script_parser.c

GENSRCS=	littlemips_script.c bigmips_script.c

CLEANFILES+=	${GENSRCS} y.tab.h ld_script_parser.c ld_script_lexer.c

LDADD=	-lelftc -ldwarf -lelf

CFLAGS+= -DARCH_MIPS

NOMAN=

.include <bsd.prog.mk>
