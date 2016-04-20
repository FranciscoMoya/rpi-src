#include <reactor/reactor.h>
#include <reactor/socket_handler.h>
#include <stdio.h>

void handler(event_handler* ev)
{
    endpoint* ep = (endpoint*)ev;
    char buf[128];
    int n = endpoint_recv(ep, buf, sizeof(buf));
    buf[n] = '\0';
    printf("Got [%s]\n", buf);
}

void reinstall_server(reactor* r)
{
    reactor_add(r, (event_handler*)udp_endpoint_new("8888", handler));
}

void exception_handler(reactor* r, exception e)
{
    print_exception(e);
    reinstall_server(r);
}

int main()
{
    reactor* r = reactor_new();
    reactor_set_exception(r, exception_handler);
    reinstall_server(r);
    reactor_run(r);
    reactor_destroy(r);
    return 0;
}
