#pragma once
#include "globals.h"
#include <stddef.h>


/* Setups a process emulation execution. */
void setup_proc(ProcessRegister *const, Command *const,
                size_t size, int *head, int *lim);

struct goto_scope_args {
    ProcessRegister *emu;
    char *scope_name;
    size_t pc;
};
/* Emulate entering a scope (line number) in a (virtual) method. */
void goto_scope(void *vargp);

// TODO -> Update all references
/* Emulates a method call. Receives a goto_scope_args as parameter. */
void call_scope(void *vargp);

struct recursive_call_args
{
    ProcessRegister *emu;
    struct {
        const char *name; // base name
        char *p;          // edited name place to be saved
    } scope;
    struct {
        const char *fmt;
        char *p;
    } header;
    int rec_lvl; // current level of recursion
};
void recursive_call(void *vargp);

struct recursive_args {
    size_t pointer;
    int *arg_value;
    char *scope_name;
    char *header; // point to the process header string
    const char *header_fmt; // must be a string with a single %d format specifier
};

/* Increments Stack Pointer. Receive a process_register as argument */
void inc_stack(void *vargp);

/* Increments a variable at pointer location. Receives a int* as argument */
struct inc_var_args {
    ProcessRegister *emu;
    int **mem;
};
void inc_var(void *vargp);

struct set_var_args {
    ProcessRegister *emu;
    int *mem;
    int value;
};
// TODO -> Check access safety
/* Set a variable on stack memory. Assumes valid access. */
void set_var(void *vargp);

struct stream_data_args {
    ProcessRegister *emu;
    Segment data_stream; // data to be transferred
    struct {
        /* If inc is true, updates stack pointer, and use swapping
         * if false, doesn't check for limit */
        bool inc;
        // TODO -> update references
        int **p; // Stack pointer
        int *lim; // limit of stack
    } stack;
    struct {
        // TODO -> update references
        int **tail;
        int *lim;
    } swap; // Use only if stack is set. in case of SWAPPING set use swap as "stack.p"
};
/* Stream a sequence of data on stack memory. Assumes valid access. */
void stream_data(void *vargp);

/* Compare a with b using comp (<, >, =). Assigns the result to _flags (COMP) */
struct comp_var_args {
    ProcessRegister *emu;
    int jump;
    int **a, **b;
    char comp;
};
void comp_var(void *vargp);

/* Runs next step of execution. Returns 1 if running or 0 otherwise */
int step_proc(ProcessRegister * const);

/* Clean a partition "from" first pointer arg "to" second  */
void clear_partition(int *, int *);

/* Function to allocate space in StackPartition using First Fit */
bool allocateFirstFit(StackPartition *partition, size_t size, Segment *allocatedSegment);

/* Function to allocate space in StackPartition using Best Fit */
bool allocateBestFit(StackPartition *partition, size_t size, Segment *allocatedSegment);

/* Function to allocate space in StackPartition using Worst Fit */
bool allocateWorstFit(StackPartition *partition, size_t size, Segment *allocatedSegment);