all: image


image: bootloader kernel userland toolchain
	$(MAKE) -C Image -f Makefile.notroot all

bootloader:
	$(MAKE) -C Bootloader -f Makefile.notroot all

kernel:
	$(MAKE) -C Kernel -f Makefile.notroot all

userland:
	$(MAKE) -C Userland -f Makefile.notroot all

toolchain:
	$(MAKE) -C Toolchain -f Makefile.notroot all


clean:
	$(MAKE) -C Toolchain -f Makefile.notroot clean
	$(MAKE) -C Bootloader -f Makefile.notroot clean
	$(MAKE) -C Image -f Makefile.notroot clean
	$(MAKE) -C Kernel -f Makefile.notroot clean
	$(MAKE) -C Userland -f Makefile.notroot clean

.PHONY: all clean toolchain userland kernel bootloader
