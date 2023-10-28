// #include "globals.h"
// #include "utils.h"
// 
// int *_mem;
// size_t _mem_size = DEF_MEM_SIZE;
// const float _mem_ratio = .7;
// size_t _raw_end;
// 
// struct _registers_table _registers;
// 
// bool waiting;
// 
// // Wait in a separated thread (ms)
// void *wait(void *vargp) 
// {
//     waiting = true;
//     msleep((*(size_t *)vargp));
//     waiting = false;
//     return NULL;
// }
