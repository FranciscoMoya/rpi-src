#include "event_handler.h"
#include <assert.h>
#include <stdlib.h>

event_handler* event_handler_new (int fd, event_handler_function handler)
{
    event_handler* ev = malloc(sizeof(event_handler));
    event_handler_init (ev, fd, handler);
    ev->destroy = (event_handler_function) free;
    return ev;
}

static void do_nothing (event_handler* ev) { }

void event_handler_init (event_handler* ev, int fd, event_handler_function handle)
{
    assert(handle != NULL);
    ev->fd = fd; ev->handle_events = handle; ev->destroy = do_nothing;
}

void event_handler_handle_events (event_handler* ev)
{
    ev->handle_events(ev);
}

void event_handler_destroy (event_handler* ev)
{
    ev->destroy(ev);
}
