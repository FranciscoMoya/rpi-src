#include <reactor/reactor.h>
#include <reactor/pipe_handler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void reader(event_handler* ev)
{
    int n;
    if (0 > read(ev->fd, &n, sizeof(n)))
	Throw Exception(errno, "No pudo leer");

    printf("Got %d\n", n);

    if (n > 9)
	reactor_quit(ev->r);
}

static void writer(event_handler* ev)
{
    static int n = 1;
    if (0 > write(ev->fd, &n, sizeof(n)))
	Throw Exception(errno, "No pudo escribir");
    ++n;
}

int main()
{
    reactor* r = reactor_new();
    pipe_handler* p = pipe_handler_new(reader);
    reactor_add(r, &p->parent);
    reactor_set_timeout(r, 500,
			event_handler_new(p->pipe[1], writer));
    reactor_run(r);
    reactor_destroy(r);
}
