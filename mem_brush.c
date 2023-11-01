#include "mem_brush.h"
#include "utils.h"
#include "globals.h"
#include <limits.h>
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
    int *q, *new_tail;
    int was_swapping = args->emu->last_mod.swapping;

    if (was_swapping) // start access
        args->emu->last_mod.swap.start = q;
    else {
        q = *(args->stack.p);
        args->emu->last_mod.stack.start = q;

        while (p != args->data_stream.end) {
            if (q >= args->stack.lim && args->stack.inc) {
                if (args->swap.tail == NULL)
                    _flags |= STACK_OVERFLOW; // error
                else {
                    args->emu->last_mod.swapping = true; // start swapping
                    *(args->swap.tail) = args->emu->swap.head; // swap on head (assumes partition empty)
                    new_tail = q - 1;
                    
                }
                break;
            }
            else {
                *q = *p; // stream data
                p++, q++;

                if (args->stack.inc)
                    new_tail = q; // Update stack tail
            }
        }
        *(args->stack.p) = new_tail;
        args->emu->last_mod.stack.end = new_tail; // end access
    }

    if (args->emu->last_mod.swapping && args->swap.tail != NULL) {
        q = *(args->swap.tail); // start swap access
        args->emu->last_mod.swap.start = q; // start access (swap)

        while (*p != *(args->data_stream.end)) {
            if (q >= args->swap.lim) {
                _flags |= STACK_OVERFLOW;
                break;
            }
            else {
                *q = *p; // stream data to swap
                p++, q++;

                if (args->stack.inc)
                    new_tail = q;
            }
        }
        *(args->swap.tail) = new_tail; // Update swap tail
        args->emu->last_mod.swap.end = new_tail; // end access (swap)
    }
}


void recursive_call(void *vargp)
{
    struct recursive_call_args *args = (struct recursive_call_args *)vargp;

    sprintf(args->scope.p, "%s[%d]", args->scope.name, args->rec_lvl);
    (args->rec_lvl)++;

    Segment arg_list;
    struct stream_data_args stream_args = {
        .data_stream = args->emu->args.values,
        .emu = args->emu,
        .stack.inc = true,
    }; // next copy segment

    bool was_swapping = args->emu->last_mod.swapping;
    if (was_swapping) { // Stream to selected partition
        stream_args.swap.tail = &args->emu->swap.tail;
        stream_args.swap.lim = args->emu->swap.lim;
        stream_args.stack.lim = NULL; // don't swap the swap
        stream_args.stack.p = NULL;
        arg_list.start = args->emu->swap.tail;
        arg_list.end = arg_list.start + args->emu->args.n;
    }
    else {
        stream_args.stack.p = &args->emu->stack.tail;
        stream_args.stack.lim = args->emu->stack.lim;
        stream_args.swap.tail = &args->emu->swap.tail;
        stream_args.swap.lim = args->emu->swap.lim;
        arg_list.start = args->emu->stack.tail;
        arg_list.end = arg_list.start + args->emu->args.n;
    }

    stream_data(&stream_args); // Copy arguments to method call

    // fix: swap segmentation (IGNORE: currently only supports one argument)
    if (!was_swapping && args->emu->last_mod.swapping) // Just started swapping
        arg_list = (Segment){ .start=args->emu->swap.head, .end=args->emu->swap.tail };
    else
        args->emu->args.values = arg_list;

    // TODO -> Implement custom sprintf that receives a vector of int
    switch (args->emu->args.n) { // update header: only one arg was necessary until then
        case 1:
            sprintf(args->header.p, args->header.fmt, *(args->emu->args.values.start));
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
}


void call_scope(void *vargp)
{
    // if (_flags & COMP)
    //     return; // Don't call recursive

    struct goto_scope_args *args = (struct goto_scope_args *)vargp;

    goto_scope(args);
    _call(args->emu);

}


void comp_var(void *vargp)
{
    struct comp_var_args *args = (struct comp_var_args *)vargp;
    switch (args->comp) { // Only this given checks for simplicity
        case '<':
            if (**(args->a) >= **(args->b))
                args->emu->pc = args->jump;
        break;
        case '>':
            if (**(args->a) <= **(args->b))
                args->emu->pc = args->jump;
        break;
        case '=':
            if (**(args->a) != **(args->b))
                args->emu->pc = args->jump;
        break;
        default: break;
    }
}


int step_proc(ProcessRegister *const emu)
{
    if (emu->pc < emu->size) {
        _call(emu);
        (emu->pc)++;
        return true;
    }

    return false;
}

void clear_partition(int *from, int *to)
{
    while (from != to)
        *((from)++) = EOF;
}

/*
Inputs:
    emptySpaces: empty partition array
    size: required allocation size
    allocatedSegment : segment that will be allocated
*/
bool allocateFirstFit(Segment *emptySpaces, size_t size, Segment *allocatedSegment) {
    int *current = emptySpaces->start;
    int *end = emptySpaces->end;
    // Pointer to the start of the segment that will be allocated (or not)
    int *startOfSegment = NULL;
    
    while (current < end) {
        // Find a large enough empty space
        if (current + size <= end) {
            startOfSegment = current;
            break;
        }
        current++;
    }
    
    if (startOfSegment) {
        // Allocate the segment
        emptySpaces->start = startOfSegment + size;
        allocatedSegment->start = startOfSegment;
        allocatedSegment->end = startOfSegment + size;
        return true;
    } else {
        // No space available
        return false;
    }
}

/*
Inputs:
    emptySpaces: empty partition array
    size: required allocation size
    allocatedSegment : segment that will be allocated
*/
bool allocateBestFit(Segment *emptySpaces, size_t size, Segment *allocatedSegment) {
    int *current = emptySpaces->start;
    int *end = emptySpaces->end;
    int *startOfSegment = NULL;
    int minSize = INT_MAX; // Minimum size initially set as maximum
    
    while (current < end) {
        // Find a large enough empty space
        if (current + size <= end) {
            int currentSize = current + size - current;
            if (currentSize < minSize) {
                minSize = currentSize;
                startOfSegment = current;
            }
        }
        current++;
    }
    
    if (startOfSegment) {
        // Allocate the segment
        emptySpaces->start = startOfSegment + size;
        allocatedSegment->start = startOfSegment;
        allocatedSegment->end = startOfSegment + size;
        return true;
    } else {
        // No space available
        return false;
    }
}

/*
Inputs:
    emptySpaces: empty partition array
    size: required allocation size
    allocatedSegment : segment that will be allocated
*/
bool allocateWorstFit(Segment *emptySpaces, size_t size, Segment *allocatedSegment) {
    int *current = emptySpaces->start;
    int *end = emptySpaces->end;
    int *startOfSegment = NULL;
    int maxSize = 0; // Maximum size initially set to zero
    
    while (current < end) {
        // Find a large enough empty space
        if (current + size <= end) {
            int currentSize = current + size - current;
            if (currentSize > maxSize) {
                maxSize = currentSize;
                startOfSegment = current;
            }
        }
        current++;
    }
    
    if (startOfSegment) {
        // Allocate the segment
        emptySpaces->start = startOfSegment + size;
        allocatedSegment->start = startOfSegment;
        allocatedSegment->end = startOfSegment + size;
        return true;
    } else {
        // No space available
        return false;
    }
}

/*
Inputs:
    occupiedPartitions: occupied partition array
    numOccupied: number of occupied partitions
    numEmpty: pointer to the number of empty partitions
Output:
    emptySpaces: array that will store empty partitions
*/
Segment* calculateEmptySpaces(Segment *occupiedPartitions, size_t numOccupied, size_t *numEmpty) {
    // Initializes the number of empty partitions to 0
    *numEmpty = 0;
    // Pointer to the end of the last occupied partition
    int *lastEnd = NULL;

    Segment *emptySpaces = (Segment *)malloc(numOccupied * sizeof(Segment));

    if (emptySpaces == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < numOccupied; i++) {
        // First partition occupied
        if (lastEnd == NULL) {
            lastEnd = occupiedPartitions[i].end;
        }
        
        else {
            int *start = lastEnd;
            int *end = occupiedPartitions[i].start;
            
            // Empty space between partitions
            if (start < end) {
                emptySpaces[*numEmpty].head = start;
                emptySpaces[*numEmpty].tail = end;
                (*numEmpty)++;
            }

            lastEnd = occupiedPartitions[i].end;
        }
    }
    return emptySpaces;
}