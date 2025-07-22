#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>
#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    terminal_buffer = VGA_MEMORY;
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = vga_entry(' ', terminal_color);
        }
    }
}

void terminal_setcolor(uint8_t color) {
    terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
    if (x >= VGA_WIDTH || y >= VGA_HEIGHT) return;
    const size_t index = y * VGA_WIDTH + x;
    terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
        return;
    }
    unsigned char uc = c;
    terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) terminal_row = 0;
    }
}

void terminal_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
        terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
    terminal_write(data, strlen(data));
}

// Prosty sleep - pętle
void sleep(int seconds) {
    for (int s = 0; s < seconds; s++) {
        for (volatile unsigned long i = 0; i < 10000000; i++) {
        }
    }
}

// Klawiatura

key_state_t key_state = {0};

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void keyboard_update() {
    uint8_t status;
    __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"(0x64));

    while (status & 1) {
        uint8_t scancode = inb(0x60);
        bool pressed = !(scancode & 0x80);
        uint8_t code = scancode & 0x7F;

        switch (code) {
            case 0x11: key_state.w = pressed; break;     // W
            case 0x1E: key_state.a = pressed; break;     // A
            case 0x1F: key_state.s = pressed; break;     // S
            case 0x20: key_state.d = pressed; break;     // D
            case 0x39: key_state.space = pressed; break; // Space
            case 0x19: key_state.p = pressed; break;     // P
            default: break;
        }

        __asm__ volatile ("inb %1, %0" : "=a"(status) : "Nd"(0x64));
    }
}


//Rand

static uint32_t rand_state = 123456789; // dowolna ziarna
uint32_t rand(void) {
    rand_state = rand_state * 1103515245 + 12345;
    return (rand_state / 65536) % 32768;
}