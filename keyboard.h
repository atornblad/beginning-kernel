#ifndef KEYBOARD_H
#define KEYBOARD_H

void _init_keyboard_idt(void);
void restart_keyboard(void);
unsigned char raw_keyb_nowait(void);
unsigned char raw_keyb(void);
int raw_gets(char *buffer, int buffer_size);

#endif
