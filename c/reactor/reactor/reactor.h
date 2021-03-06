#ifndef REACTOR_H
#define REACTOR_H

#include <reactor/event_handler.h>
#include <reactor/exception.h>
#include <sys/select.h>
#include <time.h>

/* Clase que implementa el dispatcher del patrón reactor.  Utiliza la
   demultiplexión de eventos de Unix (select/poll/epoll) por lo que
   las fuentes de eventos están asociadas a un descriptor de archivo.

   Invariantes de clase:

   running, paused tienen valores 0/1, inicialmente 0
   max_fd = descriptor más alto registrado, inicialmente -1
   handler[i] != NULL para i < num_handlers
   destroy != NULL
   exception != NULL
 */

#define REACTOR_MAX_HANDLERS 64

typedef struct reactor_ reactor;
typedef void (*reactor_function)(reactor* r);
typedef void (*reactor_exception_function)(reactor* r, exception e);


struct reactor_ {
    int running;
    int paused;
    int max_fd;
    fd_set fds;
    int num_handlers;
    event_handler* handlers[REACTOR_MAX_HANDLERS];
    struct timeval tv, timeout;
    reactor_function destroy;
    reactor_exception_function exception;
};

reactor* reactor_new ();
void reactor_init (reactor* r);
void reactor_destroy (reactor* r);
void reactor_run (reactor* r);
void reactor_add (reactor* r, event_handler* h);
void reactor_remove (reactor* r, int fd);

void reactor_demultiplex_events (reactor* r);
void reactor_enable (reactor* r, int fd);
void reactor_disable (reactor* r, int fd);

void reactor_set_timeout(reactor* r, int msecs);
void reactor_set_default_timeout(reactor* r);
void reactor_pause(reactor* r, int do_pause);
void reactor_quit(reactor* r);

void reactor_set_exception(reactor* r, reactor_exception_function exc);

#endif
