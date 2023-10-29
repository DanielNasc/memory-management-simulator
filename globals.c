#include "globals.h"
#include "utils.h"
#include "sleep.h"

int skip = false;
int skip_to_event = 0;

int *_mem;
int *_swap;
int *_raw_end;
int *_heap_end;
size_t _mem_size = DEF_MEM_SIZE;
const float _mem_ratio = .7;

bool waiting;

// Wait in a separated thread (ms)
void *wait(void *vargp) 
{
    waiting = true;
    msleep((*(size_t *)vargp));
    waiting = false;
    return NULL;
}
