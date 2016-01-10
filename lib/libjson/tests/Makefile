#	@(#)Makefile	8.1 (Berkeley) 5/31/93

CFLAGS+=-Os -Wall -Werror -I..
LDADD+= ../libjson.a -lc -lgcc
NOMAN=

TESTS   = test1 test2 test4 testReplaceExisting \
          test_cast test_charcase test_locale test_null \
          test_parse test_parse_int64 test_printbuf test_set_serializer

all:    $(TESTS)

clean:
	rm -f *.o $(TESTS)

.c:
	$(CC) $(CFLAGS) -c $(.IMPSRC) -o $(.TARGET).o
	$(CC) $(LDFLAGS) $(.TARGET).o $(LDADD) -o $(.TARGET)

.include <bsd.prog.mk>
