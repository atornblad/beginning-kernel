#include "screen.h"
#include "hardware.h"

static int cur_x = 0;
static int cur_y = 0;
static unsigned char cur_attr = COLOR(7,0);

static void raw_clear_screen(int fromx, int fromy, int tox, int toy)
{
    int from_index = (fromy * VGAWIDTH + fromx);
    int to_index = (toy * VGAWIDTH + tox);
    BYTE *ptr = &VGAMEM[from_index * 2];
    int len = to_index - from_index;
    register unsigned char attr = cur_attr;

    while (len >= 0) {
        *(ptr++) = ' ';
        *(ptr++) = attr;
        --len;
    }
}

unsigned char raw_get_cur_attr(void)
{
    return cur_attr;
}

void raw_set_cur_attr(unsigned char attr)
{
    cur_attr = attr;
}

void raw_clrscr()
{
    raw_clear_screen(0, 0, VGAWIDTH - 1, VGAHEIGHT - 1);
}

void raw_clrscr_from_cursor()
{
    raw_clear_screen(cur_x, cur_y, VGAWIDTH - 1, VGAHEIGHT - 1);
}

void raw_clrscr_to_cursor()
{
    raw_clear_screen(0, 0, cur_x, cur_y);
}

void raw_print(const char *message)
{
    register char c;
    
    while ((c = *(message++)) != '\0') {
        raw_print_char(c);
    }
    
    raw_update_cursor_position();
}

void raw_print_char(char one)
{
    BYTE *ptr = &VGAMEM[cur_y * VGAWIDTH * 2 + cur_x * 2];
    *(ptr++) = (unsigned char)one;
    *(ptr++) = cur_attr;
    
    ++cur_x;
    if (cur_x >= VGAWIDTH) {
        raw_print_newline();
    }
}

void raw_print_move_left(void)
{
    --cur_x;
    if (cur_x < 0) {
        cur_x = VGAWIDTH - 1;
        raw_print_move_up();
    }
}

void raw_print_move_right(void)
{
    ++cur_x;
    if (cur_x >= VGAWIDTH) {
        cur_x = 0;
        raw_print_move_down();
    }
}

void raw_print_move_up(void)
{
    --cur_y;
    if (cur_y < 0) {
        /* TODO: SCROLL entire screen DOWN, using buffer */
        cur_y = 0;
    }
}

void raw_print_move_down(void)
{
    ++cur_y;
    while (cur_y >= VGAHEIGHT) {
        raw_scroll_up();
    }
}

void raw_print_backspace(char replacement)
{
    raw_print_move_left();
    raw_print_char(replacement);
    raw_print_move_left();
}

void raw_print_uint(unsigned int value)
{
    unsigned int power_of_ten = 1000000000;
    unsigned char must_print = 0;

    while (power_of_ten > 0) {
        if ((value >= power_of_ten) || must_print || power_of_ten == 1) {
            char display = '0';
            while (value >= power_of_ten) {
                display++;
                value -= power_of_ten;
            }
            raw_print_char(display);
            must_print = 1;
        }
        power_of_ten /= 10;
    }
}

void raw_print_newline()
{
    cur_x = 0;
    cur_y++;
    
    while (cur_y >= VGAHEIGHT) {
        raw_scroll_up();
    }

    raw_update_cursor_position();
}

void raw_scroll_up() {
    int i;
    BYTE *ptr_target = VGAMEM;
    BYTE *ptr_source = &(ptr_target[VGAWIDTH * 2]);

    for (i = 0; i < (VGAHEIGHT - 1) * VGAWIDTH * 2; ++i) {
        *(ptr_target++) = *(ptr_source++);
    }

    for (i = 0; i < 80; ++i) {
        *(ptr_target++) = ' ';
        *(ptr_target++) = cur_attr;
    }

    --cur_y;
}

void raw_update_cursor_position()
{
    register unsigned short position = (cur_y * VGAWIDTH) + cur_x;
    
    /* cursor LOW */
    outb(0x3d4, 0x0f);
    outb(0x3d5, (unsigned char)(position & 0xff));
    
    /* cursor HIGH */
    outb(0x3d4, 0x0e);
    outb(0x3d5, (unsigned char)(position >> 8));
}

void raw_gotoxy(int x, int y)
{
    cur_x = x;
    cur_y = y;
}

void raw_get_cursor_position(int *x, int *y)
{
    *x = cur_x;
    *y = cur_y;
}
