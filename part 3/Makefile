obj-m += project-4-3.o

all:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc test_user.c -o test_user.o

clean:
	sudo make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -rf test_user.o