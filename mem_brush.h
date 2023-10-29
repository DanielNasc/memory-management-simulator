#pragma once
#include "globals.h"
#include <stddef.h>


/* Setups a process emulation execution. */
void setup_proc(struct _emulation_register *const, struct command *const,
                size_t size, int *head, int *lim);

struct enter_scope_args {
    struct _emulation_register *emu;
    char *scope_name;
    size_t pc;
};
/* Emulate entering a scope (line number) in a (virtual) method. */
void goto_scope(void *vargp);

struct recursive_args {
    size_t pointer;
    int *arg_value;
    char *scope_name;
    char *header; // point to the process header string
    const char *header_fmt; // must be a string with a single %d format specifier
};
/* Emulate entering a scope of a (virtual) recursive method with one integer arg. */
void recursive_call(void *vargp);

/* Increments Stack Pointer. Receive a _emulation_register as argument */
// void inc_sp(void *vargp); // TODO -> update all references
void inc_stack(void *vargp);

/* Increments a variable at pointer location. Receives a int* as argument */
struct inc_var_args {
    struct _emulation_register *emu;
    int *mem;
};
void inc_var(void *vargp);

struct set_var_args {
    struct _emulation_register *emu;
    int *mem;
    int value;
};
// TODO -> Check access safety
/* Set a variable on stack memory. Assumes valid access. */
void set_var(void *vargp);

struct stream_data_args {
    struct _emulation_register *emu;
    int *mem;
    int *data_stream;
};
// TODO -> Check access safety
/* Stream a sequence of data on stack memory. Assumes valid access. */
void stream_data(void *vargp);

/* Runs next step of execution. Returns 1 if running or 0 otherwise */
int step_proc(struct _emulation_register * const);
