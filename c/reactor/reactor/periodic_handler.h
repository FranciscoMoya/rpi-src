#ifndef PERIODIC_HANDLER_H
#define PERIODIC_HANDLER_H

/* Clase que modela eventos periódicos */

#include <reactor/thread_handler.h>

typedef struct periodic_handler_ periodic_handler;
struct periodic_handler_ {
    thread_handler parent;
    int period;
    event_handler_function handler;
};

periodic_handler* periodic_handler_new(int period, event_handler_function handler);
void periodic_handler_init(periodic_handler* h,
			  int period, event_handler_function handler);
void periodic_handler_destroy(periodic_handler* h);

#endif
