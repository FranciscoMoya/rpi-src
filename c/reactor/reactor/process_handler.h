#ifndef PROCESS_HANDLER_H
#define PROCESS_HANDLER_H

/* Especialización de event_handler para generación de procesos.  El
   constructor devuelve un event_handler diferente para cada proceso.
   Se comunican bidireccionalmente mediante dos pipes. El destructor
   del padre se ocupa de la destrucción del proceso hijo. */

#include <reactor/reactor.h>

typedef struct process_handler_ process_handler;
struct process_handler_ {
    event_handler parent;
    int pid;
    int out;
    event_handler_function destroy_parent_members;
};

process_handler* process_handler_new (event_handler_function parent_handle,
                                  event_handler_function child_handle);
void process_handler_init (process_handler* ev,
			 event_handler_function parent_handle,
			 event_handler_function child_handle);
void process_handler_destroy(process_handler* ev);
int process_handler_is_child (process_handler* ev);
void process_handler_stay_forever_on_child (process_handler* ev);

#endif
