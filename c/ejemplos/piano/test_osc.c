#include "osc.h"
#include <ctype.h>
#include <stdio.h>

void dump_char(char c);
void dump(const char* buf, size_t len);
size_t encode(char* buf, size_t size, const char* cmd, va_list ap);
void decode(const char* buf, size_t size);

void test(const char* cmd, ...)
{
    va_list ap;
    va_start(ap, cmd);

    char buf[128];
    size_t len = encode(buf, sizeof(buf), cmd, ap);
    va_end(ap);
    decode(buf, len);
}

int main()
{
    test("/d_load", "/home/pi/supercolliderStandaloneRPI2/share/user/synthdefs/sine.scsyndef");
    test("/s_new", "", "sine", 1000, 1, 1);
    test("/s_new", "i", "sine", 1001, 1, 1, "freq", 900);
    test("/n_free", 1000);
    test("/n_free", 1001);
    return 0;
}

void dump_char(char c)
{
    if (isprint(c))
	putchar(c);
    else
	printf("\\x%02x", c);
}

void dump(const char* buf, size_t len)
{
    for (size_t i = 0; i<len; ++i)
	dump_char(buf[i]);
    putchar('\n');
}

size_t encode(char* buf, size_t size, const char* cmd, va_list ap)
{
    size_t len = osc_encode_message(buf, size, cmd, &ap);
    dump(buf, len);
    return len;
}

void decode(const char* buf, size_t size)
{
    char out[128];
    size_t len = osc_decode_message(buf, size, out, sizeof(out));
    dump(out, len);
}
