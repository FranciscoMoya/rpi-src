#ifndef SPAWN_HANDLER_H
#define SPAWN_HANDLER_H

/* Especialización de event_handler para generación de procesos.  El
   constructor devuelve un event_handler diferente para cada proceso.
   Se comunican bidireccionalmente mediante dos pipes. El destructor
   del padre se ocupa de la destrucción del proceso hijo. */

#include <reactor/reactor.h>

typedef struct spawn_handler_ spawn_handler;
struct spawn_handler_ {
    event_handler parent;
    int pid;
    int out;
    event_handler_function destroy_parent_members;
};

spawn_handler* spawn_handler_new (event_handler_function parent_handle,
                                  event_handler_function child_handle);
void spawn_handler_init (spawn_handler* ev,
			 event_handler_function parent_handle,
			 event_handler_function child_handle);
void spawn_handler_destroy(spawn_handler* ev);
int spawn_handler_is_child (spawn_handler* ev);
void spawn_handler_stay_forever_on_child (spawn_handler* ev);

#endif
