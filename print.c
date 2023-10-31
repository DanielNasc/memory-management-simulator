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


void print_memory(int *const mem, ProcessRegister *const emus, const unsigned emus_n)
{
    const long SPEED = 5;
    putchar('[');

    if (_mem == mem)        // if stack, print RAW in purple
        printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if ((mem + i) == _raw_end && _mem == mem)
            printf("\033[m");

        // draw last modified space as green
        for (unsigned j = emus_n; j != 0; j--) {
            if (emus[j].last_mod.stack.start == (mem + i))
                printf("\033[32m");
            if (emus[j].last_mod.stack.end == (mem + i))
                printf("\033[m");
            if (emus[j].last_mod.swap.start == (mem + i))
                printf("\033[32m");
            if (emus[j].last_mod.swap.end == (mem + i))
                printf("\033[m");
        }

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


void print_mem_hex(int *const mem, ProcessRegister *const emus, const unsigned emus_n)
{
    const long SPEED = 3;
    const size_t WIDTH = LINE_WIDTH / 4;
    putchar('[');

    if (mem == _mem) // if stack, print RAW in purple
        printf("\033[35m"); // RAW data section color

    for (size_t i = 0; i != _mem_size; ++i) {
        if (!skip) {
            fflush(stdout);
            msleep(SPEED);
        }

        // Place the mem ratio separation
        if ((mem + i) == _raw_end && mem == _mem)
            printf("\033[m");

        // draw last modified space as green
        // TODO -> Verificar por quê não está colorindo o espaço recém modificado
        for (unsigned j = emus_n; j != 0; j--) {
            if (emus[j].last_mod.stack.start == (mem + i))
                printf("\033[32m");
            if (emus[j].last_mod.stack.end == (mem + i))
                printf("\033[m");
            if (emus[j].last_mod.swap.start == (mem + i))
                printf("\033[32m");
            if (emus[j].last_mod.swap.end == (mem + i))
                printf("\033[m");
        }

        // print mem
        if (mem[i] == EOF)
            printf(" -- ");
        else {
            char hex[3];
            ultobyte(mem[i], hex);
            printf(" %s ", hex);
        }

        if ((i + 1) % WIDTH == 0)
            printf("\n ");
    }

    if (_mem_size % WIDTH == 0)
        putchar('\b');
    printf("]\n");
}


void print_code(ProcessRegister *const emu)
{
    printf("\nPC: %lu, Stack Tail: %ld\n", emu->pc, emu->stack.tail - _mem);
    putchar('\n');

    for (size_t i = 0; i < emu->size; i++) {
        if (!skip)
            msleep(100);
        printf("%s%lu: %s\n" CLIS_RESET, (emu->pc == (i + 1) ? CLIS_FOCUS ">" : " "),
               (i + 1), emu->commands[i].line);
    }
}
