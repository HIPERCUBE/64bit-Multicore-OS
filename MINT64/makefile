all: BootLoader Kernel32 Disk.img

BootLoader:
	@echo
	@echo =============== Build Boot Loader ===============
	@echo

	make -C 00.BootLoader

	@echo
	@echo =============== Build Complete ===============
	@echo

Kernel32:
	@echo
	@echo =============== Build 32bit Kernel ===============
	@echo

	make -C 01.Kernel32

	@echo
	@echo =============== Build Complete ===============
	@echo

Disk.img: 00.BootLoader/BootLoader.bin 01.Kernel32/Kernel32.bin
	@echo
	@echo =============== Disk Image Build Start ===============
	@echo

	cat $^ > Disk.img

	@echo
	@echo =============== All Build Complete ===============
	@echo

clean:
	make -C 00.BootLoader clean
	make -C 01.Kernel32 clean
	rm -f Disk.img