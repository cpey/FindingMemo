CC = gcc
CFLAGS = -g -I../memo
LDFLAGS = -nostdlib -nostartfiles -static -L../memo
GLIBCDIR = ~/repos/glibc/install_64/lib
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o
ENDFILES = $(GLIBCDIR)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -lfm -Wl,--end-group

.PHONY: clean

msg-queue-test: msg-queue-test.c
	$(CC) -o $@ $^
	scp $@ test:

libfm-test.o: libfm-test.c
	$(CC) $(CFLAGS) -c $^

libfm-test: libfm-test.o
	$(CC) $(LDFLAGS) -o $@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES)

clean:
	rm -rf *.o
	rm -rf msg-queue-test
	rm -rf libfm-test
