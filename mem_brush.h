#pragma once
#include "globals.h"
#include <stddef.h>


/* Setups a process emulation execution. */
void setup_proc(struct command *commands, size_t size);

void none(void *vargp); /* Do nothing */

struct enter_scope_args {
    size_t pointer;
    char *scope_name;
};
/* Emulate entering a scope of a (virtual) function */
void enter_scope(void *vargp);

struct recursive_args {
    size_t pointer;
    int *arg_value;
    char *scope_name;
    char *header; // point to the process header string
    const char *header_fmt; // must be a string with a single %d format specifier
};
/* Emulate entering a scope of a (virtual) recursive function with one integer arg. */
void recursive_call(void *vargp);

/* Increments Program Counter */
void inc_pc(void *vargp);

/* Increments Stack Pointer */
void inc_sp(void *vargp);

struct inc_var_args {
    int *pointer;
};
/* Increments a variable at pointer location */
void inc_var(void *vargp);

struct set_var_args {
    size_t *pointer;
    int *data_stream;
};
/* Set a variable on stack memory */
void set_var(void *vargp);

/* Runs next step of execution. Returns 1 if running or 0 otherwise */
int step_proc();
