#include "mem_brush.h"
#include "utils.h"
#include "globals.h"
#include <stdio.h>


void setup_proc(struct _emulation_register *const emu, struct command * const commands,
                size_t size, int *head, int *lim)
{
    if (emu->commands != commands) {
        emu->commands = commands;
        emu->size = size;
        emu->pc = 0;
        emu->stack.head = emu->stack.tail = head;
        emu->stack.lim = lim;
        emu->last_mod.start = emu->last_mod.end = NULL;
    }
}

// TODO -> update all references
void goto_scope(void *vargp)
{
    struct enter_scope_args *args = (struct enter_scope_args *)vargp;

    args->emu->scope_name = args->scope_name;
    args->emu->pc = args->pc;
}


// FIXME
void recursive_call(void *vargp)
{
//     struct recursive_args *args = (struct recursive_args *)vargp;
// 
//     _registers.scope_name = args->scope_name;
//     _registers.pc = args->pointer;
// 
//     if (_registers.stack_tail < _registers.heap_end) // block if erroring
//         _mem[_registers.stack_tail++] = *(args->arg_value);
// 
//     sprintf(args->header, args->header_fmt, *(args->arg_value));
}


void none(void *vargp) {}


void inc_stack(void *vargp)
{
    struct _emulation_register *emu = (struct _emulation_register *)vargp;
    (emu->stack.tail)++;
}


void inc_var(void *vargp)
{
    struct inc_var_args *args = (struct inc_var_args *)vargp;
    (*args->mem)++;
    args->emu->last_mod.start = args->mem;
    args->emu->last_mod.end = args->mem + 1;
}


void set_var(void *vargp)
{
    struct set_var_args *args = (struct set_var_args *)vargp;

    // fix: Check access safety
    *(args->mem) = args->value;
    args->emu->last_mod.start = args->mem;
    args->emu->last_mod.end = args->mem + 1;
}


void stream_data(void *vargp)
{
    struct stream_data_args *args = (struct stream_data_args *)vargp;

    // fix: check access safety before using it
    int *q = args->mem;
    args->emu->last_mod.start = args->mem;

    for (int *p = args->data_stream; *p != EOF; p++, q++)
        *q = *p;

    args->emu->last_mod.end = q;
}


int step_proc(struct _emulation_register * const emu)
{
    if (emu->pc < emu->size) {
        struct command *com = emu->commands + emu->pc;

        if (com->call) // if not NULL
            com->call(com->args); // call it
        (emu->pc)++;

        return true;
    }
    else if (emu->pc == emu->size)
        (emu->pc)++; // No line will be highlighted

    return false;
}
