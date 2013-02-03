#obj-m += mydev2.o
#obj-m += procfs.o
#obj-m += nl_kern.o
#obj-m += gnKernel.o
obj-m += mydev4.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
