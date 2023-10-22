#pragma once

/* Print Error */
#define perr(S) fprintf(stderr, "\033[31m""Error: " S "\033[m\n")
#define pwarr(S) fprintf(stderr, "\033[33m""Warning: " S "\033[m\n")

#define bool uint8_t /* boolean */
#define true 1
#define false 0

#define _1KB 1024 /* 1KB in B */

/* ultob: converts the unsigned long n into a base b character representation in
 * the string s, with minimum size of min (filled with zeroes). Assumes b >= 2. */
void ultob(unsigned long ul, char s[], unsigned char b, unsigned long min);
/* Converts a unsigned long into a hex string */
#define ultohex(UL, S, M) ultob(UL, S, 16, M)
#define ultobyte(UL, S) ultohex(UL, S, 2)
