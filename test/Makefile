CC = gcc

CFLAGS = -g -I../memo
CFLAGS32 = -g -m32 -I../memo
LDFLAGS = -nostdlib -nostartfiles -static -L../memo/build
LDFLAGS32 = -nostdlib -nostartfiles -static -L../memo/build32
GLIBCDIR = ~/repos/glibc/install_64/lib
GLIBCDIR32 = ~/repos/glibc/install/lib
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o
STARTFILES32 = $(GLIBCDIR32)/crt1.o $(GLIBCDIR32)/crti.o
ENDFILES = $(GLIBCDIR)/crtn.o
ENDFILES32 = $(GLIBCDIR32)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -lfm -Wl,--end-group
LIBGROUP32 = -Wl,--start-group $(GLIBCDIR32)/libc.a -lgcc -lgcc_eh -lfm -Wl,--end-group

.PHONY: clean

msg-queue-test: msg-queue-test.c
	$(CC) -o $@ $^
	scp $@ test:

libfm-test.o: libfm-test.c
	$(CC) $(CFLAGS) -c $^

libfm-test: libfm-test.o
	$(CC) $(LDFLAGS) -o $@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES)

libfm-test-32.o: libfm-test.c
	$(CC) $(CFLAGS32) -c -o $@ $^

libfm-test-32: libfm-test-32.o
	$(CC) $(LDFLAGS32) -m32 -o $@ $(STARTFILES32) $^ $(LIBGROUP32) $(ENDFILES32)

clean:
	rm -rf *.o
	rm -rf msg-queue-test
	rm -rf libfm-test
	rm -rf libfm-test-32
