#ifndef PIPE_HANDLER_H
#define PIPE_HANDLER_H

/* Clase para traducir eventos externos a eventos en un descriptor de
   una pipe.  El método write escribe en la pipe y por tanto dispara
   eventos en el reactor. El método read se proporciona por
   conveniencia.

   Invariantes de clase:

   pipe es una pipe inicializada con flag O_DIRECT. Lo escrito en
   pipe[1] se lee de forma atómica en pipe[0] hasta un máximo de
   PIPE_BUF bytes.
*/

#include <reactor/event_handler.h>
#include <sys/types.h>

typedef struct pipe_handler_ pipe_handler;
struct pipe_handler_ {
    event_handler parent;
    int pipe[2];
    event_handler_function destroy_parent_members;
};

pipe_handler* pipe_handler_new (event_handler_function handler);
void pipe_handler_init (pipe_handler* h, event_handler_function handler);
void pipe_handler_destroy (pipe_handler* ev);
void pipe_handler_write (pipe_handler* h, const void* buf, size_t size);
int  pipe_handler_read (pipe_handler* h, void* buf, size_t max_size);

#endif
