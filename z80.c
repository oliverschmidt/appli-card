/*

MIT License

Copyright (c) 2024 Oliver Schmidt (https://a2retro.de/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <stdio.h>
#include <pico/stdlib.h>

static uint16_t dmaAddr;

static uint8_t _RamRead(uint32_t addr);
static void _RamWrite(uint32_t addr, uint32_t value);

static uint32_t _HardwareIn(uint32_t port);
static void _HardwareOut(uint32_t port, uint32_t value);

static void _Bios(void);
static void _Bdos(void);

typedef   int8_t   int8;
typedef  int16_t  int16;
typedef  int32_t  int32;
typedef  uint8_t  uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

#define TRUE  true
#define FALSE false

#define LOW_DIGIT(x)     ((x) & 0xf)
#define HIGH_DIGIT(x)    (((x) >> 4) & 0xf)
#define LOW_REGISTER(x)  ((x) & 0xff)
#define HIGH_REGISTER(x) (((x) >> 8) & 0xff)

#define SET_LOW_REGISTER(x, v)  x = (((x) & 0xff00) | ((v) & 0xff))
#define SET_HIGH_REGISTER(x, v) x = (((x) & 0xff) | (((v) & 0xff) << 8))

#define _kbhit()       (getchar_timeout_us(0) != PICO_ERROR_TIMEOUT)
#define _getch         getchar
#define _putcon        putchar
#define _putch         putchar
#define _puts          printf
#define _puthex8(arg)  printf("%02x", (arg))
#define _puthex16(arg) printf("%04x", (arg))

#include "cpu.h"

#include "z80.h"

extern const __attribute__((aligned(4))) uint8_t firmware[];

volatile uint32_t z80_rd_value;
volatile bool     z80_rd_state;
volatile uint32_t z80_wr_value;
volatile bool     z80_wr_state;

static uint8_t ram[0x10000];
static bool shadow;

static uint8_t _RamRead(uint32_t addr) {
    if (shadow && addr < 0x8000) {
        return firmware[addr];
    } else {
        return ram[addr];
    } 
}

static void _RamWrite(uint32_t addr, uint32_t value) {
    if (shadow && addr < 0x8000) {
        return;
    }
    ram[addr] = value;
}

static uint32_t _HardwareIn(uint32_t port) {
    uint32_t value = 0x00;
    switch (port) {
        case 0x20:
            value = z80_rd_value;
            z80_rd_state = false;
#ifdef PICO_DEFAULT_LED_PIN
            gpio_put(PICO_DEFAULT_LED_PIN, true);
#endif
            break;
        case 0x040:
            value = z80_rd_state ? 0x80 : 0x00 |
                    z80_wr_state ? 0x01 : 0x00;
    }
    return value;
}

static void _HardwareOut(uint32_t port, uint32_t value) {
    switch (port) {
        case 0x00:
            z80_wr_value = value;
            z80_wr_state = true;
#ifdef PICO_DEFAULT_LED_PIN
            gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
            break;
        case 0x60:
            shadow = !!(value & 0x01);
            break;
    }
}

static void _Bios(void) {
    puts("BIOS");
}

static void _Bdos(void) {
    puts("BDOS");
}

void z80_run(void) {
#ifdef PICO_DEFAULT_LED_PIN
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#endif    

    while (true) {
        Z80reset();
        z80_rd_state = false;
        z80_wr_state = false;
        shadow = true;
        puts("Reset");
#ifdef PICO_DEFAULT_LED_PIN
        gpio_put(PICO_DEFAULT_LED_PIN, false);
#endif
        Z80run();
    }
}
