#include "memory.h"
#include "hardware.h"
#include "types.h"

/*
 * count_memory
 * probes memory above 1MB
 * fetched from http://wiki.osdev.org/Detecting_Memory_(x86)
 * last changed: 21 Feb 1999
 */

ULONG mem_end = 0;
ULONG bse_end = 0;
ULONG upper_mem_size = 0;

void count_memory(void)
{
    register ULONG *mem;

    ULONG mem_count, a;
    USHORT memkb;
    BYTE irq1, irq2;
    ULONG cr0;

    /* save IRQs */
    irq1 = inb(0x21);
    irq2 = inb(0xa1);

    /* kill all IRQs */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);

    mem_count = 0;
    memkb = 0;

    /* store a copy of CR0 */
    asm ( " movl %%cr0, %0\n" : "=a"(cr0) : : );

    /* Write-back and invalidate the cache */
    asm ( " wbinvd\n" );

    /* Cache disable (486+), no-writeback(486+), 32bit mode (386+) */
    asm ( " movl %0, %%cr0\n" : : "a"(cr0 | 0x60000001) :  );

    do {
        memkb++;
        mem_count += 1048576;
        mem = (ULONG*)mem_count;

        a = *mem;
        *mem = 0x55aa55aa;

        /* Empty asm call to tell gcc not to trust its registers */
        asm ( "" : : : "memory" );

        if (*mem != 0x55aa55aa) {
            mem_count = 0;
        } else {
            *mem = 0xaa55aa55;
            asm ( "" : : : "memory" );
            if (*mem != 0xaa55aa55) {
                mem_count = 0;
            }
        }

        asm ( "" : : : "memory" );
        *mem = a;
    } while (memkb < 4096 && mem_count != 0);

    asm ( " movl %0, %%cr0\n" : : "a" (cr0) : );

    mem_end = memkb << 20;
    mem = (ULONG*) 0x413;
    bse_end = (*mem & 0xffff) << 6;
    upper_mem_size = (mem_end - 0x00100000);

    outb(0x21, irq1);
    outb(0xa1, irq2);
}

