#ifndef HARDWARE_H
#define HARDWARE_H

static inline void outb(unsigned short port, unsigned char value)
{
    asm volatile (" outb %0,%1" : : "a" (value), "dN" (port));
}

static inline unsigned char inb(unsigned short port)
{
    unsigned char value;
    asm volatile (" inb %1,%0" : "=a" (value) : "dN" (port));
    return value;
}

static inline unsigned char is_protected()
{
    unsigned char value;
    asm volatile(" mov %%cr0,%%eax\n"
                 " and $0x01,%%eax\n"
                 " movb %%al,%0\n"

                 : "=r"(value)
                 :
                 : "%eax"
    );
    return value;
}

#endif
