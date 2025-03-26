all: bootloader
	
bootloader:
	nasm -f elf64 -o start.o start.asm
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c 
	ld -nostdlib -T link.lds -o user start.o main.o lib.a
	objcopy -O binary user totalmem


clear:
	rm -f boot/boot.img

run:
	qemu-system-x86_64 -L "C:/Program Files/qemu" -drive format=raw,file=os.img