#include "mem_brush.h"
#include "utils.h"
#include "globals.h"
#include <stdio.h>


void setup_proc(ProcessRegister *const emu, Command * const commands,
                size_t size, int *head, int *lim)
{
    if (emu->commands != commands) {
        emu->commands = commands;
        emu->size = size;
        emu->pc = 0;
        emu->stack.head = emu->stack.tail = head;
        emu->stack.lim = lim;
        emu->last_mod.stack.start = emu->last_mod.stack.end = NULL;
        emu->last_mod.swap.start = emu->last_mod.swap.end = NULL;
        emu->last_mod.swapping = false;
    }
}


void goto_scope(void *vargp)
{
    struct goto_scope_args *args = (struct goto_scope_args *)vargp;

    args->emu->scope_name = args->scope_name;
    args->emu->pc = args->pc;
}


void inc_stack(void *vargp)
{
    ProcessRegister *emu = (ProcessRegister *)vargp;
    (emu->stack.tail)++;
}


void inc_var(void *vargp)
{
    struct inc_var_args *args = (struct inc_var_args *)vargp;
    (**args->mem)++;
    args->emu->last_mod.stack.start = *args->mem;
    args->emu->last_mod.stack.end = *args->mem + 1;
}


void set_var(void *vargp)
{
    struct set_var_args *args = (struct set_var_args *)vargp;

    // fix: Check access safety
    *(args->mem) = args->value;
    args->emu->last_mod.stack.start = args->mem;
    args->emu->last_mod.stack.end = args->mem + 1;
}


void stream_data(void *vargp)
{
    struct stream_data_args *args = (struct stream_data_args *)vargp;

    int *p = args->data_stream.start;
    int *q = *(args->stack.p);
    args->emu->last_mod.stack.start = q; // start access

    while (p != args->data_stream.end) {
        if (q >= args->stack.lim && args->stack.inc) {
            if (args->stack.inc) {
                if (args->swap.tail == NULL) {
                    _flags |= STACK_OVERFLOW; // error
                    break;
                }
                else
                    args->emu->last_mod.swapping = true; // start swapping
            }
        }
        else
            *q = *p; // stream data
        p++, q++;
    }
    *(args->stack.p) = q; // Update stack tail
    args->emu->last_mod.stack.end = q; // end access (stack)

    if (args->emu->last_mod.swapping && args->swap.tail != NULL) {
        q = *(args->swap.tail); // start swap access
        args->emu->last_mod.stack.end = q; // start access (swap)

        while (*p != *(args->data_stream.end)) {
            if (q >= args->swap.lim) {
                _flags |= STACK_OVERFLOW;
                break;
            }
            else
                *q = *p; // stream data to swap
            p++, q++;
        }
        *(args->swap.tail) = q; // Update swap tail
        args->emu->last_mod.stack.end = q; // end access (swap)
    }
}


void enter_scope(void *vargp)
{
    struct enter_scope_args *args = (struct enter_scope_args *)vargp;

    sprintf(args->scope.p, "%s[%d]", args->scope.name, args->rec_lvl);
    (args->rec_lvl)++;

    Segment arg_list;
    struct stream_data_args stream_args = {
        .data_stream = *(args->args.from),
        .emu = args->emu,
        .stack.inc = true,
    }; // next copy segment

    if (args->emu->last_mod.swapping) { // Stream to selected partition
        stream_args.stack.p = &args->emu->swap.tail;
        stream_args.stack.lim = args->emu->swap.lim;
        stream_args.swap.lim = NULL; // don't swap the swap
        stream_args.swap.tail = NULL;
        arg_list.start = args->emu->swap.tail;
        arg_list.end = arg_list.start + args->args.n;
    }
    else {
        stream_args.stack.p = &args->emu->stack.tail;
        stream_args.stack.lim = args->emu->stack.lim;
        stream_args.swap.tail = &args->emu->swap.tail;
        stream_args.swap.lim = args->emu->swap.lim;
        arg_list.start = args->emu->stack.tail;
        arg_list.end = arg_list.start + args->args.n;
    }

    stream_data(&stream_args); // Copy arguments to method call
    *(args->args.from) = arg_list;
    // fix: swap segmentation

    // TODO -> Implement custom sprintf that receives a vector of int
    switch (args->args.n) { // update header: only one arg was necessary until then
        case 1:
            sprintf(args->header.p, args->header.fmt, *(args->args.from->start));
            break;
        default:
            break;
    };
}


/* Call the next PC line, if exists */
static void _call(ProcessRegister *const emu)
{
    Command *com = emu->commands + emu->pc;

    if (com->call)            // if not NULL
        com->call(com->args); // call it
    (emu->pc)++;
}


void call_scope(void *vargp)
{
    struct goto_scope_args *args = (struct goto_scope_args *)vargp;

    if (_flags & COMP) {
        step_proc(args->emu);
        return;
    }

    goto_scope(args);
    _call(args->emu);
}


void comp_var(void *vargp)
{
    struct comp_var_args *args = (struct comp_var_args *)vargp;
    switch (args->comp) { // Only this given checks for simplicity
        case '<':
            if (**(args->a) >= **(args->b))
                _flags |= COMP;
        break;
        case '>':
            if (**(args->a) <= **(args->b))
                _flags |= COMP;
        break;
        case '=':
            if (**(args->a) != **(args->b))
                _flags |= COMP;
        break;
        default: break;
    }
}


int step_proc(ProcessRegister *const emu)
{
    if (emu->pc < emu->size) {
        _call(emu);
        return true;
    }
    else if (emu->pc == emu->size)
        (emu->pc)++; // No line will be highlighted

    return false;
}


void clear_partition(int *from, int *to)
{
    while (from != to)
        *((from)++) = EOF;
}
