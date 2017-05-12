#ifndef SCREEN_H
#define SCREEN_H

#include "types.h"

typedef enum {
    CC_BLACK,
    CC_RED,
    CC_GREEN,
    CC_YELLOW,
    CC_BLUE,
    CC_MAGENTA,
    CC_CYAN,
    CC_WHITE
} COLOR_CODE;

#define COLOR(fg,bg) ((bg << 4) | (fg))

#define VGAMEM ((PBYTE)0xb8000)
#define VGAWIDTH (80)
#define VGAHEIGHT (25)

void raw_print_char(char);
void raw_clrscr();
void raw_clrscr_from_cursor();
void raw_clrscr_to_cursor();
void raw_print(const char *);
void raw_print_char(char);
void raw_print_backspace(char);
void raw_print_newline();
void raw_print_move_up(void);
void raw_print_move_down(void);
void raw_print_move_left(void);
void raw_print_move_right(void);
void raw_scroll_up();
void raw_update_cursor_position();
void raw_print_uint(unsigned int value);
void raw_get_cursor_position(int *x, int *y);
void raw_gotoxy(int x, int y);
unsigned char raw_get_cur_attr(void);
void raw_set_cur_attr(unsigned char attr);

/* Change graphics mode: http://wiki.osdev.org/VGA_Hardware
                         http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c */

#endif
