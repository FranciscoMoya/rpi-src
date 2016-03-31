#ifndef DELAYED_HANDLER_H
#define DELAYED_HANDLER_H

/* Ejecuta el manejador pasado cierto tiempo.  Al terminar eleva
   excepción, por lo que se desinstala de forma automática.
 */

#include <reactor/thread_handler.h>

typedef struct delayed_handler_ delayed_handler;
struct delayed_handler_ {
    thread_handler parent;
    int delay;
    event_handler_function handler;
};

delayed_handler* delayed_handler_new(int delay, event_handler_function handler);
void delayed_handler_init(delayed_handler* h,
			  int delay, event_handler_function handler);
void delayed_handler_destroy(delayed_handler* h);

#endif
