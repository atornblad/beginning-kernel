#include <stdarg.h>

#include "text.h"
#include "screen.h"

static void normal_print_char(char c);
static void esc_print_char(char c);
static void csi_print_char(char c);
static void (*print_char_func)(char) = normal_print_char;

static void normal_print_char(char c)
{
    switch (c) {
        case '\b':
            raw_print_move_left();
            break;
        case '\n':
            raw_print_newline();
            break;
        case '\x1b':
            print_char_func = esc_print_char;
            break;
        case '\x9b':
            print_char_func = csi_print_char;
            break;
        default:
            raw_print_char(c);
    }
}

static void esc_print_char(char c)
{
    switch (c) {
        case '[':
            print_char_func = csi_print_char;
            break;
        default:
            print_char_func = normal_print_char;
            break;
    }
}

static void csi_print_char(char c)
{
    static int values[16] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
    static int value_index = 0;
    int i, x, y, v;

    if (c >= '0' && c <= '9') {
        if (values[value_index] == -1) values[value_index] = 0;
        values[value_index] = values[value_index] * 10 + (c - '0');
    } else if (c == ';' && value_index <= 14) {
        ++value_index;
    } else {
        switch (c) {
            case 'A':
                v = values[0] == -1 ? 1 : values[0];
                for (i = 0; i < v; ++i) raw_print_move_up();
                break;
            case 'B':
                v = values[0] == -1 ? 1 : values[0];
                for (i = 0; i < v; ++i) raw_print_move_down();
                break;
            case 'C':
                v = values[0] == -1 ? 1 : values[0];
                for (i = 0; i < v; ++i) raw_print_move_right();
                break;
            case 'D':
                v = values[0] == -1 ? 1 : values[0];
                for (i = 0; i < v; ++i) raw_print_move_left();
                break;
            case 'H':
            case 'f':
                y = values[0] == -1 ? 1 : values[0];
                x = values[1] == -1 ? 1 : values[1];
                raw_gotoxy(x - 1, y - 1);
                break;
            case 'm':
                v = raw_get_cur_attr();
                for (i = 0; i <= value_index; ++i) {
                    switch (values[i]) {
                        case -1:
                        case 0:
                            v = 0x07;
                            break;
                        case 1:
                            v |= 0x08;
                            break;
                        case 7:
                            v = ((v & 0xf0) >> 4) | ((v & 0x0f) << 4);
                            break;
                        case 22:
                            v &= 0x77;
                            break;
                        case 39:
                            v = ((v & 0xf8) | 0x07);
                            break;
                        case 49:
                            v = (v & 0x0f);
                            break;
                        default:
                            if (values[i] >= 30 && values[i] <= 37) {
                                v = (v & 0xf8) | (values[i] - 30);
                            }
                            else if (values[i] >= 40 && values[i] <= 47) {
                                v = (v & 0x8f) | ((values[i] - 40) << 4);
                            }
                            else if (values[i] >= 90 && values[i] <= 97) {
                                v = (v & 0xf0) | 0x08 | (values[i] - 90);
                            }
                            else if (values[i] >= 100 && values[i] <= 107) {
                                v = (v & 0x0f) | 0x80 | ((values[i] - 100) << 4);
                            }
                    }
                }
                raw_set_cur_attr(v);
                break;
            case 'J':
                v = values[0] == -1 ? 0 : values[0];
                switch (v) {
                    case 1:
                        raw_clrscr_to_cursor();
                        break;
                    case 2:
                        raw_clrscr();
                        break;
                    case 0:
                    default:
                        raw_clrscr_from_cursor();
                        break;
                }
        }

        for (int i = 0; i < 16; ++i) values[i] = -1;
        value_index = 0;
        print_char_func = normal_print_char;
    }
}

void print(const char *message) {
    char c;

    while ((c = *message++) != '\0') {
        print_char_func(c);
    }

    raw_update_cursor_position();
}

static void print_int(int value) {
    int power_of_ten = 1000000000;
    int must_print = 0;

    if (value < 0) {
        print_char_func('-');
        value = -value;
    }

    while (power_of_ten > 0) {
        if (value >= power_of_ten || power_of_ten == 1 || must_print) {
            char output = '0';
            while (value >= power_of_ten) {
                ++output;
                value -= power_of_ten;
            }
            print_char_func(output);
            must_print = 1;
        }
        power_of_ten /= 10;
    }
}

static void print_hex(ULONG value) {
    int i;

    for (i = 0; i < 8; ++i) {
        int c = (value >> 28) & 0x0f;
        c += '0';
        if (c > '9') {
            c += ('a' - '9' - 1);
        }
        print_char_func(c);
        value = value << 4;
    }
}

void printf(const char *format, ...) {
    va_list args;
    char c;
    int percent = 0;
    char *string_arg;
    int int_arg;

    va_start(args, format);

    while ((c = *format++) != '\0') {
        if (percent) {
            switch (c) {
                case 's':
                    string_arg = va_arg(args, char *);
                    print(string_arg);
                    break;
                case 'd':
                    int_arg = va_arg(args, int);
                    print_int(int_arg);
                    break;
                case 'x':
                    int_arg = va_arg(args, int);
                    print_hex((ULONG)int_arg);
                    break;
                default:
                    print_char_func('%');
                    print_char_func(c);
                    break;
            }
            percent = 0;
        } else {
            switch (c) {
                case '%':
                    percent = 1;
                    break;
                default:
                    print_char_func(c);
                    break;
            }
        }
    }

    raw_update_cursor_position();
}
