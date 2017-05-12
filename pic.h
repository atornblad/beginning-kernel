#ifndef PIC_H
#define PIC_H

/* Programmable Interrupt Controller (PIC) */

#include "types.h"

typedef struct idt_entry {
    WORD offset_lowerbits;
    WORD selector;
    BYTE zero;
    BYTE type_attr;
    WORD offset_higherbits;
} IDT_entry, *PIDT_entry;

#define IDT_COUNT 256

extern IDT_entry IDT[IDT_COUNT];
void _load_idt(void);

#endif
