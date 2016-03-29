#include "reactor.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define DEFAULT_TIMEOUT 1000

struct exception_context the_exception_context[1];

exception Exception(int ec, const char* what)
{
    exception e = { ec, what };
    return e;
}


static int max(int a, int b) { return a>b? a: b; }

static void reactor_destroy_members (reactor* r);
static void reactor_destroy_and_free (reactor* r);

reactor* reactor_new ()
{
    reactor* r = malloc(sizeof(reactor));
    reactor_init(r);
    r->destroy = reactor_destroy_and_free;
    return r;
}

void reactor_init (reactor* r)
{
    r->running = r->paused = 0;
    FD_ZERO(&r->fds);
    r->max_fd = -1;
    r->num_handlers = 0;
    r->timeout_handler = NULL;
    r->destroy = reactor_destroy_members;
    reactor_set_default_timeout(r);
}

void reactor_destroy (reactor* r)
{
    r->destroy(r);
}

static void reactor_dispatch_event (reactor* r, fd_set* fds);
static void reactor_dispatch_timeout (reactor* r);
static void reactor_demultiplex_events (reactor* r);

void reactor_run (reactor* r)
{
    for(r->running = 1; r->running; )
        reactor_demultiplex_events(r);
}

void reactor_add (reactor* r, event_handler* h)
{
    r->handlers[r->num_handlers++] = h;
    r->max_fd = max(r->max_fd, h->fd);
    reactor_enable(r, h->fd);
}

static void reactor_erase_handler_for_fd (reactor* r, int fd);

void reactor_remove (reactor* r, int fd)
{
    reactor_disable(r, fd);
    reactor_erase_handler_for_fd (r, fd);
}

void reactor_enable (reactor* r, int fd)
{
    FD_SET(fd, &r->fds);
}

void reactor_disable (reactor* r, int fd)
{
    FD_CLR(fd, &r->fds);
}

void reactor_set_timeout (reactor* r, int msec, event_handler* h)
{
    if (r->timeout_handler)
	event_handler_destroy(r->timeout_handler);
    r->timeout_handler = h;
    r->timeout.tv_sec = msec / 1000;
    r->timeout.tv_usec = (msec % 1000) * 1000;
    r->tv = r->timeout;
}

static void do_nothing (event_handler* ev) { }

void reactor_set_default_timeout(reactor* r)
{
    reactor_set_timeout(r, DEFAULT_TIMEOUT,
			event_handler_new(-1, do_nothing));
}

void reactor_pause(reactor* r, int do_pause)
{
    r->paused = do_pause;
}

void reactor_quit(reactor* r)
{
    r->running = 0;
}


static void reactor_destroy_members (reactor* r)
{
    for (int i=0; i<r->num_handlers; ++i)
        event_handler_destroy(r->handlers[i]);
    event_handler_destroy(r->timeout_handler);
}

static void reactor_destroy_and_free (reactor* r)
{
    reactor_destroy_members(r);
    free(r);
}

static void reactor_try_run_handler(reactor* r, int fd);

static void reactor_dispatch_event (reactor* r, fd_set* fds)
{
    for (int fd = 0; fd <= r->max_fd; ++fd)
        if (FD_ISSET (fd, fds))
            reactor_try_run_handler(r, fd);
}

static void reactor_dispatch_timeout (reactor* r)
{
    if (!r->paused) {
	exception e __attribute__((unused));
	Try {
	    event_handler_handle_events(r->timeout_handler);
	}
        Catch (e) {
	    reactor_set_default_timeout(r);
	}
    }
    r->tv = r->timeout;
}

static void reactor_demultiplex_events (reactor* r)
{
    fd_set fds_copy = r->fds;
    int ret = select(r->max_fd+1, &fds_copy, 0, 0, &r->tv);
    assert(ret >= 0);

    if (ret > 0) {
        reactor_dispatch_event(r, &fds_copy);
        return;
    }

    reactor_dispatch_timeout(r);
}

static void reactor_run_handler_or_remove(reactor* r, int i);

static void reactor_try_run_handler(reactor* r, int fd)
{
    for (int i=0; i<r->num_handlers; ++i)
        if (fd == r->handlers[i]->fd) {
            reactor_run_handler_or_remove(r, i);
            return;
        }
}

static void erase_handler (event_handler* ev[], int i, int n);

static void reactor_run_handler_or_remove(reactor* r, int i)
{
    event_handler* ev = r->handlers[i];
    exception e __attribute__((unused));
    Try {
        event_handler_handle_events(ev);
    }
    Catch (e) {
	erase_handler(r->handlers, i, r->num_handlers);
    }
}

static void reactor_erase_handler_for_fd (reactor* r, int fd)
{
    for (int i=0; i<r->num_handlers; ++i)
        if (fd == r->handlers[i]->fd)
            erase_handler(r->handlers, i, r->num_handlers);
}

static void erase_handler (event_handler* ev[], int i, int n)
{
    event_handler_destroy(ev[i]);
    memmove(ev + i, ev + i + 1, (n - i - 1)*sizeof(ev[0]));
    ev[n-1] = NULL;
}
