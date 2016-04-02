#include <reactor/reactor.h>
#include <reactor/exception.h>
#include <reactor/thread_handler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
    
static void* producer(thread_handler* h)
{
    struct timespec t = { .tv_sec = 0, .tv_nsec = 500000000 };
    for(int i = 1; !h->cancel; ++i) {
	pipe_handler_write_ne(&h->parent, &i, sizeof(i));
	nanosleep(&t, NULL);
    }
    return NULL;
}

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
    thread_handler* p = thread_handler_new(reader, producer);
    reactor_add(r, (event_handler*)p);
    reactor_run(r);
    reactor_destroy(r);
}
