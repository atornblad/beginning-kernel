#include "types.h"
#include "hardware.h"
#include "pic.h"
#include "screen.h"

extern void _keyboard_irq_handler_raw(void);

extern char keyb_sv[];
static char *current_keyb_map = keyb_sv;

void _init_keyboard_idt(void)
{
    LONG irq_handler_address = (LONG)_keyboard_irq_handler_raw;

    IDT[0x21].offset_lowerbits = (irq_handler_address & 0x0000ffff);
    IDT[0x21].offset_higherbits = (irq_handler_address >> 16);
    IDT[0x21].selector = 0x08;      /* KERNEL_CODE_SEGMENT_OFFSET */
    IDT[0x21].zero = 0;
    IDT[0x21].type_attr = 0x8e;     /* INTERRUPT_GATE */

    /* Ports
     *           PIC1    PIC2
     * Command   0x20    0xa0
     * Data      0x21    0xa1
     */
    
    /* ICW1 - Begin initialization */
    outb(0x20, 0x11);
    outb(0xa0, 0x11);

    /* ICW2 - remap offset address of IDT
     * In x86 protected mode we have to remap the PICs beyond 0x20
     * because Intel has designated the first 32 interrupts as
     * "reserved" for cpu exceptions */
    outb(0x21, 0x20);
    outb(0xa1, 0x28);

    /* ICW3 - setup cascading */
    outb(0x21, 0x00);
    outb(0xa1, 0x00);

    /* ICW4 - environment info */
    outb(0x21, 0x01);
    outb(0xa1, 0x01);

    /* mask interrupts */
    outb(0x21, 0xff);
    outb(0xa1, 0xff);
}

#define KPB_SIZE 256
static unsigned short keypress_buffer[KPB_SIZE];
static int keypress_buffer_index = 0;
static int keypress_buffer_length = 0;

void restart_keyboard(void)
{
    /* 0xfd : ~(1 << 1) - enables only IRQ1 (keyboard) */
    outb(0x21, 0xfd);

    BYTE data = inb(0x61);
    outb(data | (1 << 7), 0x61);    /* Disable keyboard */
    outb(data & ~(1 << 7), 0x61);   /* Enable keyboard */
}

unsigned char raw_keyb_nowait(void) {
    unsigned short result = 0;

    asm (" cli\n");

    if (keypress_buffer_length > 0) {
        result = keypress_buffer[keypress_buffer_index];
        keypress_buffer_length--;
        keypress_buffer_index = (keypress_buffer_index + 1) % KPB_SIZE;
    }

    asm (" sti\n");

    return current_keyb_map[result];
}

unsigned char raw_keyb(void) {
    unsigned char output = raw_keyb_nowait();
    while (output == 0) {
        asm volatile (" hlt\n");
        output = raw_keyb_nowait();
    }
    return output;
}

void _keyboard_irq_handler(void)
{
    unsigned char status;
    unsigned short keycode;

    static int left_shift = 0;
    static int right_shift = 0;
    static int ctrl = 0;
    static int alt = 0;
    static int alt_gr = 0;
    static int flagged = 0;

    #ifdef KEYB_DEBUG
    static unsigned char debug_keycode_history[25] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    int debug_keep_curs_x;
    int debug_keep_curs_y;
    #endif

    status = inb(0x64);
    if (status & (1 << 0))
    {
        keycode = inb(0x60);

        switch (keycode) {
            case 224:
                flagged = 1;
                break;
            case 29:
                ctrl = 1;
                break;
            case (29 | 0x80):
                ctrl = 0;
                break;
            case 42:
                left_shift = 1;
                break;
            case (42 | 0x80):
                left_shift = 0;
                break;
            case 54:
                right_shift = 1;
                break;
            case (54 | 0x80):
                right_shift = 0;
                break;
            case 56:
                if (flagged) alt_gr = 1; else alt = 1;
                break;
            case (56 | 0x80):
                if (flagged) alt_gr = 0; else alt = 0;
                break;
            default:
                #ifdef KEYB_DEBUG
                {
                    int y;
                    raw_get_cursor_position(&debug_keep_curs_x, &debug_keep_curs_y);
                    for (y = 0; y <= 23; ++y) {
                        debug_keycode_history[y] = debug_keycode_history[y + 1];
                    }
                    debug_keycode_history[24] = keycode;
                    for (y = 0; y <= 24; ++y) {
                        raw_gotoxy(75, y);
                        raw_print_uint(debug_keycode_history[y]);
                        raw_print_char(' ');
                    }
                    raw_gotoxy(debug_keep_curs_x, debug_keep_curs_y);
                }
                #endif

                /* BIT 7 : key released */
                if ((keycode & (1 << 7)) == 0) {
                    if (keypress_buffer_length < KPB_SIZE) {
                        keycode = keycode | (left_shift << 7) | (right_shift << 7)
                                          | (ctrl << 8) | (alt << 9)
                                          | (alt_gr << 9);
                        keypress_buffer[(keypress_buffer_index + keypress_buffer_length) % KPB_SIZE] = keycode;
                        ++keypress_buffer_length;
                    }
                }
                break;
        }
    }

    /* Acknowledge interrupt */
    outb(0x20, 0x20);
}

int raw_gets(char *buffer, int buffer_size)
{
    int pos = 0;
    while (1) {
        buffer[pos] = '\0';

        char input = raw_keyb();

        switch (input) {
            case '\n':
                raw_print_newline();
                raw_update_cursor_position();
                return pos;
                break;
            case '\b':
                if (pos >= 1) {
                    raw_print_backspace(' ');
                    raw_update_cursor_position();
                    pos--;
                }
                break;
            default:
                if (pos < (buffer_size - 1)) {
                    raw_print_char(input);
                    raw_update_cursor_position();
                    buffer[pos++] = input;
                }
                break;
        }
    }
}
