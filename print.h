#pragma once

/* Boot loading message */
void *dots(void *_vargp);

/* print memory usage simulation */
void print_memory(int *const mem, struct _emulation_register *const s, const unsigned n);

/* print memory simulation's data (hex codes) */
void print_mem_hex(int * const mem, struct _emulation_register *const s, const unsigned n);

/* Emulates a process step. */
void print_code(struct _emulation_register *const);
