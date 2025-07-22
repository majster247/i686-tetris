#ifndef _KERNEL_TTY_H
#define _KERNEL_TTY_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool w, a, s, d, space, p;
} key_state_t;

extern key_state_t key_state;

void terminal_initialize(void);
void terminal_setcolor(uint8_t color);
void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y);
void terminal_putchar(char c);
void terminal_write(const char* data, size_t size);
void terminal_writestring(const char* data);

//Added functions
void keyboard_update(void);
void sleep(int seconds);

uint32_t rand(void);

#endif
