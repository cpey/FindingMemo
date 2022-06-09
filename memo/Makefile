CC = gcc
CFLAGS = -g
LDFLAGS = -nostdlib -nostartfiles -static
GLIBCDIR = ~/repos/glibc/install_64/lib
STARTFILES = $(GLIBCDIR)/crt1.o $(GLIBCDIR)/crti.o
ENDFILES = $(GLIBCDIR)/crtn.o
LIBGROUP = -Wl,--start-group $(GLIBCDIR)/libc.a -lgcc -lgcc_eh -Wl,--end-group

odir = build
odir32 = build32
src = memo.c libfm.c
obj = $(patsubst %.c,$(odir)/%.o,$(src))
src_lib = libfm.c
obj_lib = $(patsubst %.c,$(odir)/%.o,$(src_lib))
obj_lib32 = $(patsubst %.c,$(odir32)/%.o,$(src_lib))

.PHONY: clean

all: memo lib lib32
lib: libfm.a
lib32: libfm32.a

memo: $(src)
	@-mkdir $(odir)
	$(CC) $(LDFLAGS) -o $(odir)/$@ $(STARTFILES) $^ $(LIBGROUP) $(ENDFILES)

$(obj_lib): $(src_lib)
	@-mkdir $(odir)
	$(CC) -shared $(CFLAGS) -c -o $@ $<

libfm.a: $(obj_lib)
	ar rcs $(odir)/$@ $<

$(obj_lib32): $(src_lib)
	@-mkdir $(odir32)
	$(CC) -shared $(CFLAGS) -m32 -c -o $@ $<

libfm32.a: $(obj_lib32)
	ar rcs $(odir32)/libfm.a $<

clean:
	rm -rf build
	rm -rf build32
