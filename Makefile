
all:  bootloader kernel userland image

bootloader:
	$(MAKE) -C Bootloader -f Makefile.notroot all

kernel:
	$(MAKE) -C Kernel -f Makefile.notroot all

userland:
	$(MAKE) -C Userland -f Makefile.notroot all

image: kernel bootloader userland
	$(MAKE) -C Image -f Makefile.notroot all

clean:
	$(MAKE) -C Toolchain -f Makefile.notroot clean
	$(MAKE) -C Bootloader -f Makefile.notroot clean
	$(MAKE) -C Image -f Makefile.notroot clean
	$(MAKE) -C Kernel -f Makefile.notroot clean
	$(MAKE) -C Userland -f Makefile.notroot clean

.PHONY: bootloader image collections kernel userland all clean
