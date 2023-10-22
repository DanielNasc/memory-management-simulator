#pragma once

/* Print Error */
#define perr(S) fprintf(stderr, "\033[31m""Error: " S "\033[m\n")
#define pwarr(S) fprintf(stderr, "\033[33m""Warning: " S "\033[m\n")

#define bool uint8_t /* boolean */
#define true 1
#define false 0

#define _1KB 1024 /* 1KB in B */
