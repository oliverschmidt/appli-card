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

#include <a2pico.h>

#include "z80.h"

#include "board.h"

static void __time_critical_func(reset)(bool asserted) {
    if (asserted) {
        z80_reset();
    }
}

static void __time_critical_func(nop_get)(void) {
}

static void __time_critical_func(input_get)(void) {
    a2pico_putdata(pio0, z80_wr_value);
    z80_wr_state = false;
}

static void __time_critical_func(rd_state_get)(void) {
    a2pico_putdata(pio0, z80_rd_state ? 0x80 : 0x00);
}

static void __time_critical_func(wr_state_get)(void) {
    a2pico_putdata(pio0, z80_wr_state ? 0x80 : 0x00);
}

static void __time_critical_func(reset_get)(void) {
    z80_reset();
}

static void __time_critical_func(nmi_get)(void) {
    z80_nmi();
}

static const void __not_in_flash("devsel_get")(*devsel_get[])(void) = {
    input_get, nop_get,   rd_state_get, wr_state_get,
    nop_get,   reset_get, nop_get,      nmi_get,
    nop_get,   nop_get,   nop_get,      nop_get,
    nop_get,   nop_get,   nop_get,      nop_get
};

static void __time_critical_func(nop_put)(uint32_t data) {
}

static void __time_critical_func(output_put)(uint32_t data) {
    z80_rd_value = data;
    z80_rd_state = true;
}

static void __time_critical_func(reset_put)(uint32_t data) {
    z80_reset();
}

static void __time_critical_func(nmi_put)(uint32_t data) {
    z80_nmi();
}

static const void __not_in_flash("devsel_put")(*devsel_put[])(uint32_t) = {
    nop_put, output_put, nop_put, nop_put,
    nop_put, reset_put,  nop_put, nmi_put,
    nop_put, nop_put,    nop_put, nop_put,
    nop_put, nop_put,    nop_put, nop_put
};

void __time_critical_func(board)(void) {

    a2pico_init(pio0);

    a2pico_resethandler(&reset);

    while (true) {
        uint32_t pico = a2pico_getaddr(pio0);
        uint32_t addr = pico & 0x0FFF;
        uint32_t io   = pico & 0x0F00;  // IOSTRB or IOSEL
        uint32_t strb = pico & 0x0800;  // IOSTRB
        uint32_t read = pico & 0x1000;  // R/W

        if (read) {
            if (!io) {  // DEVSEL
                devsel_get[addr & 0xF]();
            }
        } else {
            uint32_t data = a2pico_getdata(pio0);
            if (!io) {  // DEVSEL
                devsel_put[addr & 0xF](data);
            }
        }
    }
}
