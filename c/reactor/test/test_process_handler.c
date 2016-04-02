#include <reactor/reactor.h>
#include <reactor/process_handler.h>
#include <reactor/delayed_handler.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void parent (event_handler* ev)
{
    process_handler* h = (process_handler*) ev;

    char buf[128];
    int n = read(ev->fd, buf, sizeof(buf));
    if (n > 0)
        printf("Parent got: %s\n", buf);

    static int calls = 3;
    if (--calls < 0) return;

    write(h->out, "parent", 7);
}

void child (event_handler* ev)
{
    process_handler* h = (process_handler*) ev;

    char buf[128];
    int n = read(ev->fd, buf, sizeof(buf));
    if (n > 0)
        printf("Child got: %s\n", buf);

    write(h->out, "child", 6);
}


static void quit(event_handler* ev) { reactor_quit(ev->r); }

int main()
{
    reactor* r  = reactor_new();
    process_handler* h = process_handler_new(parent, child);


    process_handler_stay_forever_on_child(h);
    reactor_add(r, &h->parent);
    reactor_add(r, (event_handler*)delayed_handler_new(1000, quit));
    write(h->out, "main", 5);

    reactor_run(r);
    reactor_destroy(r);
}
