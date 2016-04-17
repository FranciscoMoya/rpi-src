#include "osc.h"
#include <ctype.h>
#include <stdio.h>

void dump_char(char c)
{
    if (isalnum(c) || ispunct(c))
	putchar(c);
    else
	printf("\\%o", c);
}

void dump(const char* buf, size_t len)
{
    for (size_t i = 0; i<len; ++i)
	dump_char(buf[i]);
    putchar('\n');
}

void encode(const char* cmd, ...)
{
    va_list ap;
    char buf[128];
    size_t len;
    va_start(ap, cmd);
    len = osc_encode_message(buf, sizeof(buf), cmd, ap);
    va_end(ap);
    dump(buf, len);
}

int main()
{
    encode("/d_load", "/home/pi/supercolliderStandaloneRPI2/share/user/synthdefs/sine.scsyndef", NULL, 0);
    encode("/s_new", "sine", 1000, 1, 1, NULL);
    encode("/s_new", "sine", 1001, 1, 1, "freq", 900, NULL);
    encode("/n_free", 1000);
    encode("/n_free", 1001);
    return 0;
}
