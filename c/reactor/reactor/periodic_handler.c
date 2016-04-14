#include <reactor/periodic_handler.h>
#include <reactor/exception.h>
#include <reactor/thread_handler_private.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void* periodic_handler_thread(thread_handler* h);
static void periodic_handler_handler(event_handler* ev);
static void periodic_handler_init_members(periodic_handler* h,
					 int period, event_handler_function handler);

periodic_handler* periodic_handler_new(int period, event_handler_function handler)
{
    periodic_handler* h = malloc(sizeof(periodic_handler));
    periodic_handler_init_members(h, period, handler);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, periodic_handler_thread);
    return h;
}

void periodic_handler_init(periodic_handler* h,
			  int period, event_handler_function handler)
{
    periodic_handler_init_members(h, period, handler);
    thread_handler_start (&h->parent, periodic_handler_thread);
}


void periodic_handler_destroy(periodic_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}


static void periodic_handler_init_members(periodic_handler* h,
					 int period, event_handler_function handler)
{
    thread_handler_init_members(&h->parent, periodic_handler_handler);
    h->period = period;
    h->handler = handler;
}


static void* periodic_handler_thread(thread_handler* h)
{
    periodic_handler* dh = (periodic_handler*) h;
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


static void periodic_handler_handler(event_handler* ev)
{
    int v;
    if (0 > read(ev->fd, &v, sizeof(v)))
	Throw Exception(errno, "No pudo leer");

    periodic_handler* h = (periodic_handler*) ev;
    h->handler(ev);
}
