all: VirtualOS.bin

VirtualOS.bin: VirtualOS.asm
	nasm -o VirtualOS.bin VirtualOS.asm

clean:
	rm -f VirtualOS.bin