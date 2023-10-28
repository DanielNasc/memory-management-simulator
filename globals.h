#pragma once
#include "utils.h"
#include <stddef.h>
#include <inttypes.h>
/* Include this file only in main.h */

#define LINE_WIDTH (_1KB / 8)
#define DEF_MEM_SIZE LINE_WIDTH // default memory size

/* Command Line Interface Styles */
#define CLIS_RESET "\33[0;49;49m"
#define CLIS_FOCUS "\033[1;32m"
#define CLIS_CHICKO "\33[32;43m"
#define CLIS_CK_BOLD(S) "\33[1m" S "\33[0;32;43m"
#define CLIS_CK_UNDER(S) "\33[4m" S "\33[0;32;43m"
#define CLIS_CK_STRIKE(S) "\33[9m" S "\33[0;32;43m"
#define CLIS_CK_ITALICS(S) "\33[3m" S "\33[0;32;43m"
#define CLIS_CK_EMPHASIS(S) "\33[41;97m" S "\33[32;43m"

/* Skip mode */
#define SKIP_ALL 1
#define SKIP_REFRESH 2 // TODO -> apply it
#define SKIP_TO_EVENT 4
extern int skip; /* skip simulation animations */
extern int skip_to_event; /* skip simulation until event id */

extern int *_mem; // memory starting pointer
extern size_t _mem_size; // memory size (bytes)
extern const float _mem_ratio; // the ratio of memory allocated against RAW data

extern size_t _raw_end; // End of RAW data pool

extern struct _registers_table {
    size_t pc; // Program Counter
    size_t stack_tail; // Stack Pointer
    size_t heap_end; // Heap Limit

    // for the purposes of this emulation, to ease visualization
    struct {
        size_t start;
        size_t end;
    } last_mod; // last modified data space
    char *scope_name;
} _registers;
struct command {
    char line[LINE_WIDTH];
    void (* call)(void *);
    void *args;
};

extern struct _emulation_register {
    size_t step; /* Step of execution */
    size_t size; /* #of steps */
    struct command *commands;
} _emu_reg;       /* Registers of emulation */

extern bool waiting;
void *wait(void *vargp);
