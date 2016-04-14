#include <reactor/reactor.h>
#include <reactor/exception.h>
#include <reactor/socket_handler.h>
#include <reactor/periodic_handler.h>
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

    /* c puede ser liberado en cualquier momento. El periodic tiene que
       usar una variable que no pueda cambiar */
    connector c_aux = *c;
    
    void producer(event_handler* ev)
    {
	static int i;
	char buf[128];
	snprintf(buf, 128, "Prueba %d", i++);
	connector_send(&c_aux, buf, strlen(buf));
    }

    reactor_add(r, (event_handler*)periodic_handler_new(1000, producer));
    reactor_run(r);
    return 0;
}
