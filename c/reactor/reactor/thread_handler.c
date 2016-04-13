#define _GNU_SOURCE
#include <reactor/exception.h>
#include <reactor/thread_handler.h>
#include <stdlib.h>

typedef void* (*thread_function)(void*);

void thread_handler_start (thread_handler* h, thread_handler_function thread);
void thread_handler_stop (thread_handler* h);
static void thread_handler_free_members (thread_handler* h);

void thread_handler_init_members (thread_handler* h,
				  event_handler_function handler)
{
    pipe_handler_init (&h->parent, handler);
    event_handler* ev = &h->parent.parent;
    h->destroy_parent_members = ev->destroy_members;
    h->cancel = 0;
    ev->destroy_members = (event_handler_function) thread_handler_free_members;
}

thread_handler* thread_handler_new (event_handler_function handler,
				    thread_handler_function thread)
{
    thread_handler* h = malloc(sizeof(thread_handler));
    thread_handler_init_members(h, handler);
    event_handler* ev = &h->parent.parent;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (h, thread);
    return h;
}

void thread_handler_init (thread_handler* h,
			  event_handler_function handler,
			  thread_handler_function thread)
{
    pipe_handler_init (&h->parent, handler);
    thread_handler_init_members(h, handler);
    thread_handler_start (h, thread);
}

void thread_handler_destroy (thread_handler* h)
{
    event_handler* ev = &h->parent.parent;
    event_handler_destroy(ev);
}

static void thread_handler_free_members (thread_handler* h)
{
    thread_handler_stop(h);
    h->destroy_parent_members(&h->parent.parent);
}

/* Puede elevar excepción y se llama en constructor.  Debe retrasarse
   hasta el momento en que puede destruirse sin problemas. */
void thread_handler_start (thread_handler* h, thread_handler_function thread)
{
    int error = pthread_create(&h->thread, NULL, (thread_function)thread, h);
    if (error != 0)
	Throw Exception(error, "Imposible crear hilo");
}

/* Llamado en destructor, no puede elevar excepciones */
void thread_handler_stop (thread_handler* h)
{
    h->cancel = 1;
    struct timespec t;
    clock_gettime(CLOCK_REALTIME, &t);
    t.tv_sec += 3;
    int error = pthread_timedjoin_np(h->thread, NULL, &t);
    if (error == 0)
	return;
    pthread_cancel(h->thread);
    pthread_join(h->thread, NULL);
}
