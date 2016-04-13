#include <reactor/exception.h>
#include <reactor/blink_handler.h>
#include <reactor/thread_handler_private.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

static void* blink_handler_thread(thread_handler* h);
static void blink_handler_handler(event_handler* ev);
static void blink_handler_init_members(blink_handler* h,
				       int output, int period, size_t nblinks);

blink_handler* blink_handler_new(int output, int period, size_t nblinks)
{
    blink_handler* h = malloc(sizeof(blink_handler));
    blink_handler_init_members(h, output, period, nblinks);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, blink_handler_thread);
    return h;
}

void blink_handler_init(blink_handler* h,
			int output, int period, size_t nblinks)
{
    blink_handler_init_members(h, output, period, nblinks);
    thread_handler_start (&h->parent, blink_handler_thread);
}


void blink_handler_destroy(blink_handler* h)
{
    event_handler* ev = (event_handler*)h;
    event_handler_destroy(ev);
}


static void blink_handler_init_members(blink_handler* h,
				       int output, int period, size_t nblinks)
{
    thread_handler_init_members(&h->parent, blink_handler_handler);
    h->output = output;
    h->period = period;
    h->nblinks = nblinks;
    pinMode(output, OUTPUT);
    pullUpDnControl(output, PUD_OFF);
}

static void* blink_handler_thread(thread_handler* h)
{
    blink_handler* bh = (blink_handler*) h;
    pipe_handler*  ph = (pipe_handler*) h;
    int period = bh->period;
    struct timespec t = { .tv_sec = period/1000,
			  .tv_nsec = 1000000 * (period % 1000) };

    int v = 1;
    int n = 2 * bh->nblinks;
    for (int i = 0; i < n; ++i) {
	if (h->cancel) break;
	if (0 > pipe_handler_write_ne(ph, &v, sizeof(v)))
	    break;
	nanosleep(&t, NULL);
	v = !v;
    }
    v = -1;
    pipe_handler_write_ne(ph, &v, sizeof(v));
    return NULL;
}


static void blink_handler_handler(event_handler* ev)
{
    int v;
    if (0 > read(ev->fd, &v, sizeof(v)))
	Throw Exception(errno, "No pudo leer");

    if (v<0)
	Throw Exception(0, "Fin parpadeo");

    blink_handler* h = (blink_handler*) ev;
    digitalWrite(h->output, v);
}
