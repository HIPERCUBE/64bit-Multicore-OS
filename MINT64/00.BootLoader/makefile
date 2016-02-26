all: BootLoader.bin

BootLoader.bin: BootLoader.asm
	nasm -o BootLoader.bin BootLoader.asm

clean:
	rm -f BootLoader.bin