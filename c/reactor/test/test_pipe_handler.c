#include <reactor/reactor.h>
#include <reactor/exception.h>
#include <reactor/pipe_handler.h>
#include <reactor/timeout_handler.h>
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


int main()
{
    reactor* r = reactor_new();
    pipe_handler* p = pipe_handler_new(reader);
    reactor_add(r, &p->parent);

    void writer(event_handler* ev) {
      static int n = 1;
      pipe_handler_write(p, &n, sizeof(n));
      ++n;
    }

    reactor_add(r, (event_handler*)timeout_handler_new(500, writer));
    reactor_run(r);
    reactor_destroy(r);
}
