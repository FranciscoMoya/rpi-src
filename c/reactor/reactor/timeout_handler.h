#ifndef TIMEOUT_HANDLER_H
#define TIMEOUT_HANDLER_H

/* Clase que modela eventos periódicos */

#include <reactor/thread_handler.h>

typedef struct timeout_handler_ timeout_handler;
struct timeout_handler_ {
    thread_handler parent;
    int period;
    event_handler_function handler;
};

timeout_handler* timeout_handler_new(int period, event_handler_function handler);
void timeout_handler_init(timeout_handler* h,
			  int period, event_handler_function handler);
void timeout_handler_destroy(timeout_handler* h);

#endif
