ifneq ($(KERNELRELEASE),)
    obj-m := finder.o
    finder-objs := tracer.o hook.o
else
    KERNELDIR ?= ~/dev/src/linux-perf
    PWD  := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

endif

clean:
	make -C ${KERNELDIR} M=${PWD} clean

