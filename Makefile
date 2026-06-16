.PHONY: clean all

all:
	$(MAKE) -C user/ all VERBOSE=$(VERBOSE)
	$(MAKE) -C kernel/ kernel.bin VERBOSE=$(VERBOSE)

qemu: all
	qemu-system-i386 -machine q35 -m 256 -kernel kernel/kernel.bin

qemu-gdb: all
	qemu-system-i386 -machine q35 -m 256 -kernel kernel/kernel.bin -s -S

bochs: 
	$(MAKE) -C kernel/ bochs

bochs-debug: 
	$(MAKE) -C kernel/ bochs-debug

clean:
	$(MAKE) clean -C kernel/
	$(MAKE) clean -C user/

