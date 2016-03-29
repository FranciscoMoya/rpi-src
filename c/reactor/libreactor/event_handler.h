#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

/* Clase base para todos los manejadores de eventos. Incluye un
   descriptor de archivo porque esta implementación de reactor está
   basada en select/poll/epoll para el dispatcher.

   Invariantes de clase:

   destroy != NULL
   handle_events != NULL
*/

typedef struct event_handler_ event_handler;
typedef void (*event_handler_function)(event_handler* ev);

struct event_handler_ {
    int fd;
    event_handler_function handle_events;
    event_handler_function destroy;
};

event_handler* event_handler_new (int fd, event_handler_function handler);
void event_handler_init (event_handler* ev, int fd, event_handler_function handler);
void event_handler_handle_events (event_handler* ev);
void event_handler_destroy (event_handler* ev);

#endif
