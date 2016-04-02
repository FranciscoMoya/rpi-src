#include <reactor/timeout_handler.h>
#include <reactor/exception.h>
#include <reactor/thread_handler_private.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void* timeout_handler_thread(thread_handler* h);
static void timeout_handler_handler(event_handler* ev);
static void timeout_handler_init_members(timeout_handler* h,
					 int period, event_handler_function handler);

timeout_handler* timeout_handler_new(int period, event_handler_function handler)
{
    timeout_handler* h = malloc(sizeof(timeout_handler));
    timeout_handler_init_members(h, period, handler);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, timeout_handler_thread);
    return h;
}

void timeout_handler_init(timeout_handler* h,
			  int period, event_handler_function handler)
{
    timeout_handler_init_members(h, period, handler);
    thread_handler_start (&h->parent, timeout_handler_thread);
}


void timeout_handler_destroy(timeout_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}


static void timeout_handler_init_members(timeout_handler* h,
					 int period, event_handler_function handler)
{
    thread_handler_init_members(&h->parent, timeout_handler_handler);
    h->period = period;
    h->handler = handler;
}


static void* timeout_handler_thread(thread_handler* h)
{
    timeout_handler* dh = (timeout_handler*) h;
    pipe_handler*  ph = (pipe_handler*) h;
    int period = dh->period;
    struct timespec t = { .tv_sec = period/1000,
			  .tv_nsec = 1000000 * (period % 1000) };
    int v = 1;
    while (!h->cancel) {
	nanosleep(&t, NULL);
	if (0 > pipe_handler_write_ne(ph, &v, sizeof(v)))
	    break;
    }
    return NULL;
}


static void timeout_handler_handler(event_handler* ev)
{
    int v;
    if (0 > read(ev->fd, &v, sizeof(v)))
	Throw Exception(errno, "No pudo leer");

    timeout_handler* h = (timeout_handler*) ev;
    h->handler(ev);
}
