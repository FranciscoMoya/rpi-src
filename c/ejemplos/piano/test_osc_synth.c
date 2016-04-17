#include <reactor/reactor.h>
#include <reactor/exception.h>
#include <reactor/socket_handler.h>
#include <reactor/periodic_handler.h>
#include "osc.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>

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

void handler(event_handler* ev)
{
    char buf[128];
    int n = event_handler_recv(ev, buf, sizeof(buf));
    buf[n] = '\0';
    printf("Got %d bytes\n", n);
    dump(buf, n);
}

size_t test(char* buf, size_t size, const char* cmd, ...)
{
    va_list ap;
    va_start(ap, cmd);
    size_t len = osc_encode_message(buf, size, cmd, ap);
    va_end(ap);
    return len;
}

int main()
{
    reactor* r = reactor_new();
    connector* c = connector_new("localhost", "9999", handler);
    reactor_add(r, (event_handler*)c);

    /* c puede ser liberado en cualquier momento. El periodic tiene que
       usar una variable que no pueda cambiar */
    connector c_aux = *c;
    
    void producer(event_handler* ev)
    {
	char buf[128];
	size_t len = test(buf, sizeof(buf),
			  "/d_load",
			  "/opt/sonic-pi/etc/synthdefs/compiled/sonic-pi-piano.scsyndef");
	connector_send(&c_aux, buf, len);
    }

    reactor_add(r, (event_handler*)periodic_handler_new(1000, producer));
    reactor_run(r);
    return 0;
}
