#include <reactor/delayed_handler.h>
#include <reactor/reactor.h>
#include <reactor/thread_handler_private.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

static void* delayed_handler_thread(thread_handler* h);
static void delayed_handler_handler(event_handler* ev);
static void delayed_handler_init_members(delayed_handler* h,
					 int delay, event_handler_function handler);

delayed_handler* delayed_handler_new(int delay, event_handler_function handler)
{
    delayed_handler* h = malloc(sizeof(delayed_handler));
    delayed_handler_init_members(h, delay, handler);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, delayed_handler_thread);
    return h;
}

void delayed_handler_init(delayed_handler* h,
			  int delay, event_handler_function handler)
{
    delayed_handler_init_members(h, delay, handler);
    thread_handler_start (&h->parent, delayed_handler_thread);
}


void delayed_handler_destroy(delayed_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}


static void delayed_handler_init_members(delayed_handler* h,
					 int delay, event_handler_function handler)
{
    thread_handler_init_members(&h->parent, delayed_handler_handler);
    h->delay = delay;
    h->handler = handler;
}


static void* delayed_handler_thread(thread_handler* h)
{
    delayed_handler* dh = (delayed_handler*) h;
    pipe_handler*  ph = (pipe_handler*) h;
    int delay = dh->delay;
    struct timespec t = { .tv_sec = delay/1000,
			  .tv_nsec = 1000000 * (delay % 1000) };

    int v = 1;
    nanosleep(&t, NULL);
    pipe_handler_write_ne(ph, &v, sizeof(v));
    return NULL;
}


static void delayed_handler_handler(event_handler* ev)
{
    int v;
    if (0 > read(ev->fd, &v, sizeof(v)))
	Throw Exception(errno, "No pudo leer");

    delayed_handler* h = (delayed_handler*) ev;
    h->handler(ev);
    
    Throw Exception(0, "Expiró el retardo");
}
