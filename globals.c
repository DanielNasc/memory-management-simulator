#include "globals.h"
#include "utils.h"
#include "sleep.h"

int skip = false;
int skip_to_event = 0;

int *_mem;
int *_swap;
size_t _mem_size = DEF_MEM_SIZE;
const float _mem_ratio = .7;
size_t _raw_end;

struct _registers_table _registers;
struct _emulation_register _emu_reg;

bool waiting;

// Wait in a separated thread (ms)
void *wait(void *vargp) 
{
    waiting = true;
    msleep((*(size_t *)vargp));
    waiting = false;
    return NULL;
}
