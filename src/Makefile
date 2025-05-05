obj-m += dm_proxy.o
dm_proxy-objs = dmp.o dmp_stat.o

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean: 
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
