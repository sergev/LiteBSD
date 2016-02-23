CC      = cc
PROG    = hello skeleton rain q8

all:    $(PROG)

hello:	hello.c
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $?

skeleton: skeleton.c
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $?

rain:   rain.c
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $?

q8:     q8.c
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $?

clean:
	rm -f *.o *~ $(PROG)
