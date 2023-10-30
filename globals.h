#pragma once
#include "utils.h"
#include <stddef.h>
#include <inttypes.h>

// TODO -> refactor rename to use the conventional _ (underscore)

#define LINE_WIDTH (_1KB / 8)
#define DEF_MEM_SIZE LINE_WIDTH // default memory size

/* Command Line Interface Styles */
#define CLIS_RESET "\33[0;49;49m"
#define CLIS_FOCUS "\33[1;32m"
#define CLIS_CHICKO "\33[32;43m"
#define CLIS_CK_BOLD(S) "\33[1m" S "\33[0;32;43m"
#define CLIS_CK_UNDER(S) "\33[4m" S "\33[0;32;43m"
#define CLIS_CK_STRIKE(S) "\33[9m" S "\33[0;32;43m"
#define CLIS_CK_ITALICS(S) "\33[3m" S "\33[0;32;43m"
#define CLIS_CK_EMPHASIS(S) "\33[41;97m" S "\33[0;32;43m"
#define CLIS_CK_PURPLE(S) "\33[35;43m" S "\33[0;32;43m"

/* Skip mode */
#define SKIP_ALL 1
#define SKIP_REFRESH 2 // TODO -> apply it
#define SKIP_TO_EVENT 4
extern int skip; /* skip simulation animations */
extern int skip_to_event; /* skip simulation until event id */

#define PARTITION_SIZE 10 /* Size of a process partition */

#define COMP 1 /* Comparison flag. Active if fail */
#define STACK_OVERFLOW 2 /* Stack space overflowed */
extern int _flags; // Register of some code checks

extern int *_mem; // memory starting pointer
// TODO -> Aplicar swap
extern int *_swap; // swap starting pointer
extern int *_raw_end; // End of RAW data pool
// TODO -> Aplicar heap
extern int *_heap_end; // End of Heap

extern size_t _mem_size; // memory size (bytes)
extern const float _mem_ratio; // the ratio of memory allocated against RAW data

typedef struct command {
    char line[LINE_WIDTH]; // Line of code
    void (* call)(void *); // Callback
    void *args;            // Callback args
} Command;
typedef struct {
    int *start;
    int *end;
} Segment;
typedef struct {
    int *head; /* where it starts */
    int *tail; /* where it ends */
    int *lim;  /* where it can grow to */
} StackPartition;
typedef struct process_register {
    size_t pc; // Program Counter: step of execution */
    size_t size; /* #of steps */
    Command *commands; /* Command line "program" */
    StackPartition stack; /* stack's partition register */
    StackPartition swap; /* swap's partition register */
    struct {
        Segment stack;
        Segment swap;
        bool swapping; /* Pointer moved to swap */
    } last_mod; // last modified data space
    char *scope_name;
    struct {
        Segment values;
        size_t n;
    } args; // Arguments passed as parameters of a this method
} ProcessRegister; /* Registers of emulation: for the purposes of this emulation, to ease visualization */

extern bool waiting;
void *wait(void *vargp);
