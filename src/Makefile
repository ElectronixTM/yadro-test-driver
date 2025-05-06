obj-m += dmp.o
dmp-objs = dmp_main.o dmp_stat.o

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
