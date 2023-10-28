#include "globals.h"
#include "sleep.h"
#include <stdio.h>


void *dots(void *_vargp)
{
    const long SPEED = 100; // ms
    printf("\033[33m""Starting mini (virtual) Operational System emulation...");
    fflush(stdout);

    while (waiting) {
        msleep(SPEED);
        printf("\b\b\b   \b\b\b"); // clear dots
        fflush(stdout);

        for (int i = 3; i > 0; --i) {
            msleep(SPEED);
            putchar('.');
            fflush(stdout);
        }
    }
    printf("\033[m");
}


void print_memory(int *mem)
{
    const long SPEED = 5;
    putchar('[');
    printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if (i == _raw_end)
            printf("\033[m");

        // draw last modified space as green
        if (_registers.last_mod.start == i)
            printf("\033[32m");
        if (_registers.last_mod.end == i)
            printf("\033[m");

        // print mem
        if (mem[i] == EOF)
            printf("-");
        else
            printf("*");

        if ((i + 1) % LINE_WIDTH == 0)
            printf("\n ");
    }

    if (_mem_size % LINE_WIDTH == 0)
        putchar('\b');
    printf("]\n");
}


void print_mem_hex()
{
    const long SPEED = 3;
    const size_t WIDTH = LINE_WIDTH / 4;
    putchar('[');
    printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if (i == _raw_end)
            printf("\033[m");

        // draw last modified space as green
        if (_registers.last_mod.start == i)
            printf("\033[32m");
        if (_registers.last_mod.end == i)
            printf("\033[m");

        // print mem
        if (_mem[i] == EOF)
            printf(" -- ");
        else {
            char hex[3];
            ultobyte(_mem[i], hex);
            printf(" %s ", hex);
        }

        if ((i + 1) % WIDTH == 0)
            printf("\n ");
    }

    if (_mem_size % WIDTH == 0)
        putchar('\b');
    printf("]\n");
}


void print_code()
{
    printf("PC: %lu, Stack Tail: %lu\n", _registers.pc, _registers.stack_tail);
    putchar('\n');

    for (size_t i = 0; i < _emu_reg.size; i++) {
        if (!skip)
            msleep(100);
        printf("%s%lu: %s\n" CLIS_RESET, (_emu_reg.step == (i + 1) ? CLIS_FOCUS ">" : " "),
               (i + 1), _emu_reg.commands[i].line);
    }
}
