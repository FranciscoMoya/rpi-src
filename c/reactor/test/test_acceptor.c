#include <reactor/reactor.h>
#include <reactor/socket_handler.h>
#include <stdio.h>

void handler(event_handler* ev)
{
    char buf[128];
    int n = event_handler_recv(ev, buf, sizeof(buf));
    buf[n] = '\0';
    printf("Got [%s]\n", buf);
    event_handler_send(ev, buf, n);
}

int main()
{
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*)acceptor_new("8888", handler));
    reactor_run(r);
    reactor_destroy(r);
}
