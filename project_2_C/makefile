all: bootloader
	
bootloader:
	nasm -f bin -o boot.bin boot.asm
	nasm -f bin -o loader.bin loader.asm
	dd if=boot.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=loader.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc

clear:
	rm -f boot/boot.img

run:
	qemu-system-x86_64 -L "C:/Program Files/qemu" -drive format=raw,file=os.img