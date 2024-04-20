
all:  bootloader kernel userland image

bootloader:
	cd Bootloader; make all

kernel:
	$(MAKE) -C Kernel -f Makefile.notroot all

userland:
	$(MAKE) -C Userland -f Makefile.notroot all

image: kernel bootloader userland
	cd Image; make all

clean:
	cd Toolchain; make clean
	cd Bootloader; make clean
	cd Image; make clean
	$(MAKE) -C Kernel -f Makefile.notroot clean
	$(MAKE) -C Userland -f Makefile.notroot clean

.PHONY: bootloader image collections kernel userland all clean
