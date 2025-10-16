#include "tnki.h"
#include <stdint.h>

/* incomplete string conversion functions */

/* courtesy of paddy stkovrflw */
int
fast_atoi(const char * str)
{
    int val = 0;
    while (*str)
        val = val * 10 + (*str++ - '0');
    return val;
}

/* courtesy of Terje Mathisen 1997/1998 */

/* 0 <= val <= 99999 */
static void
itoa_half(char * buf, uint32_t val)
{
    /* move all but the first digit to the right of the decimal point */
    float tmp = val / 10000.0;

    for (size_t i = 0; i < 5; i++)
    {
        /* extract the next digit */
        int digit = (int) tmp;
        /* convert to a character */
        buf[i] = '0' + (char) digit;
        /* remove the lead digit and shift left 1 decimal place */
        tmp = (tmp - digit) * 10.0;
    }
}

/* buffer must be large enough to hold a maximum valid 32 bit integer */
void
fast_itoa(char * buf, uint32_t val)
{
    uint32_t lo = val % 100000;
    uint32_t hi = val / 100000;
    itoa_half(&buf[0], hi);
    itoa_half(&buf[5], lo);
}
