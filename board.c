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
                switch (addr & 0xF) {
                    case 0x0:
                        a2pico_putdata(pio0, z80_wr_value);
                        z80_wr_state = false;
                        break;
                    case 0x2:
                        a2pico_putdata(pio0, z80_rd_state ? 0x80 : 0x00);
                        break;
                    case 0x3:
                        a2pico_putdata(pio0, z80_wr_state ? 0x80 : 0x00);
                        break;
                    case 0x5:
                        z80_reset();
                        break;
                }
            }
        } else {
            uint32_t data = a2pico_getdata(pio0);
            if (!io) {  // DEVSEL
                switch (addr & 0xF) {
                    case 0x1:
                        z80_rd_value = data;
                        z80_rd_state = true;
                        break;
                    case 0x5:
                        z80_reset();
                        break;
                }
            }
        }
    }
}
