#include "types.h"
#include "pic.h"

IDT_entry IDT[IDT_COUNT];

void _load_idt(void)
{
    LONG idt_ptr[2];
    LONG idt_address = (LONG)IDT;

    idt_ptr[0] = (sizeof (IDT_entry) * IDT_COUNT) + ((idt_address & 0xffff) << 16);
    idt_ptr[1] = idt_address >> 16;

    asm volatile(" mov %0,%%edx\n"
                 " lidt (%%edx)\n"
                 " sti\n"
                 : 
                 : "r"(idt_ptr)
                 : "%edx"
    );
}