#include <reactor/reactor.h>
#include <reactor/console.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void keyboard(event_handler* ev)
{
    char buf[2] = " ";
    read(ev->fd, buf, 1);

    if (buf[0] == 'q')
        reactor_quit(ev->r);

    printf("Got %c\n", buf[0]);
}

int main()
{
    void* state = console_set_raw_mode(0);
    reactor* r = reactor_new();
    reactor_add(r, event_handler_new(0, keyboard));
    reactor_run(r);
    reactor_destroy(r);
    console_restore(0, state);
}
