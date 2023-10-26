#pragma once

/* Print Error */
#define print_err(S, ARGS...) \
        fprintf(stderr, "\033[31m""Error: " S "\033[m\n" __VA_OPT__(,) ARGS)
/* Print Warning */
#define print_war(S, ARGS...) \
        fprintf(stderr, "\033[31m""Warning: " S "\033[m\n" __VA_OPT__(,) ARGS)

#define bool uint8_t /* boolean */
#define true 1
#define false 0

#define _1KB 1024 /* 1KB in B */
#define SEC 1000 /* 3sec in msec */

/* ultob: converts the unsigned long n into a base b character representation in
 * the string s, with minimum size of min (filled with zeroes). Assumes b >= 2. */
void ultob(unsigned long ul, char s[], unsigned char b, unsigned long min);
/* Converts a unsigned long into a hex string */
#define ultohex(UL, S, M) ultob(UL, S, 16, M)
#define ultobyte(UL, S) ultohex(UL, S, 2)

void disable_canonical(void); /* Setup terminal for direct input. Without echoing. */
void enable_canonical(void); /* Setup terminal for default input mode. */

void clear_stdin(void);

/* ATTENTION! Must be called on program startup,
 * otherwise this lib won't work properly! */
void utils_init(void);
