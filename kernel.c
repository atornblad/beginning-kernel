/*
 * kernel.c - version 2017-03-14
 * author: Anders Tornblad
 */

#include "hardware.h"
#include "pic.h"
#include "keyboard.h"
#include "screen.h"
#include "text.h"
#include "memory.h"

void k_setup()
{
    /* Initialize everything without being disturbed by interrupts */
    _init_keyboard_idt();
    _load_idt();
}

void k_main()
{
    char input_buffer[256];

    unsigned short total;
    unsigned char lowmem, highmem;

    outb(0x70, 0x30);
    lowmem = inb(0x71);
    outb(0x70, 0x31);
    highmem = inb(0x71);
    total = lowmem | (highmem << 8);

    count_memory();

    restart_keyboard();
    raw_clrscr();
    printf("\x9b""1mTornblad's \x9b""32mProtected Mode\x9bm Kernel (C) 2017\n"
           "CMOS Memory size: %d kB\n"
           "Probed Memory size: %d MB\n",
           (int)total, upper_mem_size / 1048576);

    while (1) {
        print("\x9b""m> ");
        raw_gets(input_buffer, 256);
        print("\x9b""34;1mUnknown command: \x9b""m");
        print(input_buffer);
        raw_print_newline();
    }
}