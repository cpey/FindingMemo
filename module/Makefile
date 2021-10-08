ifneq ($(KERNELRELEASE),)
    obj-m := finder.o
    finder-objs := tracer.o hook.o impl.o
else
    KERNELDIR ?= ~/dev/src/linux
    PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	make -C ${KERNELDIR} M=${PWD} clean

