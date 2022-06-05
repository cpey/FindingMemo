CC = gcc
CFLAGS = -g
LDFLAGS = -nostdlib -nostartfiles -static
GLIBCDIR = ~/repos/glibc/install_64/lib
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o
ENDFILES = $(GLIBCDIR)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -Wl,--end-group

src = $(wildcard *.c)
obj = $(src:.c=.o)
src_lib = libfm.c
obj_lib = $(src_lib:.c=.o)

.PHONY: clean

all: memo libfm.a

$(obj): $(src)
	$(CC) $(CFLAGS) -c $^

memo: $(obj)
	$(CC) $(LDFLAGS) -o $@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES)

$(obj_lib): $(src_lib)
	$(CC) -shared $(CFLAGS) $@ -c $^

libfm.a: libfm.o
	ar rcs $@ $<

clean:
	rm -f memo $(obj)
	rm -f libfm.a $(obj_lib)
