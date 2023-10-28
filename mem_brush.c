#include "mem_brush.h"
#include "utils.h"
#include "globals.h"
#include <stdio.h>


void setup_proc(struct command *commands, size_t size)
{
    if (_emu_reg.commands != commands) {
        _emu_reg.commands = commands;
        _emu_reg.size = size;
        _emu_reg.step = 0;
    }
}


void enter_scope(void *vargp)
{
    struct enter_scope_args args = *(struct enter_scope_args *)vargp;

    _registers.scope_name = args.scope_name;
    _registers.pc = args.pointer;
}


// FIXME
void recursive_call(void *vargp)
{
    struct recursive_args *args = (struct recursive_args *)vargp;

    _registers.scope_name = args->scope_name;
    _registers.pc = args->pointer;

    if (_registers.stack_tail < _registers.heap_end) // block if erroring
        _mem[_registers.stack_tail++] = *(args->arg_value);

    sprintf(args->header, args->header_fmt, *(args->arg_value));
}


void none(void *vargp) {}


void inc_pc(void *vargp)
{
    _registers.pc++;
}


void inc_sp(void *vargp)
{
    _registers.stack_tail++;
}


void inc_var(void *vargp)
{
    struct inc_var_args args = *(struct inc_var_args *)vargp;
    (*(args.pointer))++;
}


void set_var(void *vargp)
{
    struct set_var_args args = *(struct set_var_args *)vargp;
    while (*(args.data_stream) != EOF)
        _mem[*args.pointer] = (*(args.data_stream++));
}


int step_proc()
{
    if (_emu_reg.step < _emu_reg.size) {
        struct command *com = _emu_reg.commands + _emu_reg.step;
        if (com->call)
            com->call(com->args);
        _emu_reg.step++;

        return true;
    }

    return false;
}
