#ifndef REACTOR_H
#define REACTOR_H

#include "event_handler.h"
#include "cexcept.h"
#include <sys/select.h>
#include <time.h>

define_exception_type(const char*);
extern struct exception_context the_exception_context[1];

#define REACTOR_MAX_HANDLERS 64

typedef struct reactor_ reactor;
typedef void (*reactor_function)(reactor* ev);

struct reactor_ {
    int running;
    int paused;
    int max_fd;
    fd_set fds;
    int num_handlers;
    event_handler* timeout_handler;
    event_handler* handlers[REACTOR_MAX_HANDLERS];
    struct timeval tv, timeout;
    reactor_function destroy;
};

reactor* reactor_new ();
void reactor_init (reactor* r);
void reactor_destroy (reactor* r);
void reactor_run (reactor* r);
void reactor_add (reactor* r, event_handler* h);
void reactor_remove (reactor* r, int fd);
void reactor_enable (reactor* r, int fd);
void reactor_disable (reactor* r, int fd);
void reactor_set_timeout(reactor* r, int msecs, event_handler* h);
void reactor_pause(reactor* r, int do_pause);
void reactor_quit(reactor* r);

#endif
