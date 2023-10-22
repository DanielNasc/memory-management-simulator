#include "utils.h"
#include <stddef.h>
#include <string.h>

#define TO_NBASE_C(FROM) FROM < 10 ? FROM + '0' : FROM - 10 + 'a'
void reverse(char s[]);


void ultob(size_t ul, char s[], unsigned char b, size_t min)
{
    int i = 0;

    /* reduces the value of n once */
    s[i++] = TO_NBASE_C(ul % b);
    ul /= b;

    while (ul > 0) {  /* generate digits in reverse order */
        s[i++] = TO_NBASE_C(ul % b); /* get next digit */
        ul /= b; /* delete it */
    }

    while (i < min)
        s[i++] = '0';

    s[i] = '\0';

    reverse(s);
}


/* reverse: reverse string s in place */
void reverse(char s[])
{
	char c;
	int i, j;

	for (i = 0, j = strlen(s) - 1; i < j; i++, j--)
		c = s[i], s[i] = s[j], s[j] = c; // reverse operation as a single expression.
}
