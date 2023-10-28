#pragma once

/* Boot loading message */
void *dots(void *_vargp);

/* print memory usage simulation */
void print_memory(int *mem);

/* print memory simulation's data (hex codes) */
void print_mem_hex(void);

/* Emulates a process step. */
void print_code(void);
