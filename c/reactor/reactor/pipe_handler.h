#ifndef PIPE_HANDLER_H
#define PIPE_HANDLER_H

/* Clase para traducir eventos a eventos en un descriptor de una pipe. 

   Invariantes de clase:

   ...
*/

typedef struct pipe_handler_ pipe_handler;
typedef void (*pipe_handler_function)(event_handler* ev);

struct pipe_handler_ {
    event_handler parent;
    int pipe[2];
    event_handler_function destroy_parent;
};

pipe_handler* pipe_handler_new (event_handler_function handler);
void pipe_handler_init (pipe_handler* h, event_handler_function handler);
void pipe_handler_destroy (pipe_handler* ev);
void pipe_handler_write (pipe_handler* h, const void* buf, size_t size);
void pipe_handler_read (pipe_handler* h, void* buf, size_t max_size);

#endif
