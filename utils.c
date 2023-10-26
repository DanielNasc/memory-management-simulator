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


#include <termios.h> /* UNIX terminal mode attributes */
#include <unistd.h> /* to buffers ids */
struct termios _default_info; /* terminal attributes information */


/* Read: <https://stackoverflow.com/a/18806671> */
void disable_canonical()
{
    struct termios info = _default_info;
    info.c_lflag &= ~ICANON; /* disable canonical mode */
    info.c_lflag &= ~ECHO;   /* disable echo */
    info.c_cc[VMIN] = 1;     /* wait until at least one keystroke available */
    info.c_cc[VTIME] = 0;    /* no timeout */
    tcsetattr(STDIN_FILENO, TCSANOW, &info); /* set immediately */
}


void enable_canonical()
{
    tcsetattr(0, TCSANOW, &_default_info); // Resets to default attributes
}


/* Setups this processing unity */
void utils_init()
{
    tcgetattr(STDIN_FILENO, &_default_info); /* get current terminal attributes; 0 is the file descriptor for stdin */
}
