#include <reactor/reactor.h>
#include <reactor/socket_handler.h>
#include <reactor/timeout_handler.h>
#include <stdio.h>
#include <string.h>

void handler(event_handler* ev)
{
    char buf[128];
    int n = event_handler_recv(ev, buf, sizeof(buf));
    buf[n] = '\0';
    printf("Got [%s]\n", buf);
}

int main()
{
    reactor* r = reactor_new();
    connector* c = connector_new("localhost", "8888", handler);
    reactor_add(r, (event_handler*)c);

    void producer(event_handler* ev)
    {
	static int i = 1;
	char buf[128];
	snprintf(buf, 128, "Prueba %d", i++);
	connector_send(c, buf, strlen(buf));
    }

    reactor_add(r, (event_handler*)timeout_handler_new(1000, producer));
    reactor_run(r);
    reactor_destroy(r);
}
