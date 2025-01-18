obj-m += vtfs.o
vtfs-objs:= source/vtfs.o source/fs.o source/inode.o source/ops.o source/impl.o

PWD := $(CURDIR) 
KDIR = /lib/modules/$(shell uname -r)/build
EXTRA_CFLAGS = -Wall -g

all:
	make -C $(KDIR) M=$(PWD) modules 

clean:
	if [ -e compile_commands.json ]; then \
		mkdir tmp; \
		cp compile_commands.json tmp/compile_commands.json; \
	fi; 
	make -C $(KDIR) M=$(PWD) clean 
	rm -rf .cache 
	if [ -e tmp/compile_commands.json ]; then \
		cp  tmp/compile_commands.json compile_commands.json; \
		rm -rf tmp; \
	fi;
