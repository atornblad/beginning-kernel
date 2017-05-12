kernel : kernel_asm.o kernel.o memory.o keyboard.o keyboard_asm.o keyboard_sv.o screen.o text.o pic.o linker.ld
	ld -m elf_i386 -T linker.ld -o kernel kernel_asm.o kernel.o memory.o keyboard.o keyboard_asm.o keyboard_sv.o screen.o text.o pic.o

kernel.o : kernel.c hardware.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -c kernel.c -o kernel.o

kernel_asm.o : kernel.asm
	nasm -g -f elf32 kernel.asm -o kernel_asm.o

memory.o : memory.c memory.h pic.h hardware.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -fno-stack-protector -DKEYB_DEBUG -c memory.c -o memory.o

keyboard.o : keyboard.c keyboard.h pic.h hardware.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -fno-stack-protector -DKEYB_DEBUG -c keyboard.c -o keyboard.o

keyboard_sv.o : keyb_layout_sv.c
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -c keyb_layout_sv.c -o keyboard_sv.o

screen.o : screen.c screen.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -c screen.c -o screen.o

text.o : text.c text.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -fno-stack-protector -c text.c -o text.o

keyboard_asm.o : keyboard.asm
	nasm -g -f elf32 keyboard.asm -o keyboard_asm.o

pic.o : pic.h pic.h hardware.h
	gcc -ggdb -m32 -ffreestanding -std=gnu99 -Wall -Werror -fno-stack-protector -c pic.c -o pic.o

emu : kernel
	qemu-system-i386 -kernel kernel

source : kernel.c hardware.h
	gcc -m32 -O0 -S kernel.c

clean : 
	rm *.o kernel

debug : kernel
	qemu-system-i386 -kernel kernel -gdb tcp::1234 -S -D ./qemulog -no-reboot -no-shutdown &
	gdb --command=gdbinit.script kernel
