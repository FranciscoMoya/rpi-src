#ifndef ANALOG_HANDLER_H
#define ANALOG_HANDLER_H

#include <reactor/thread_handler.h>

typedef void (*analog_handler_function)(analog_handler* this);

typedef struct analog_handler_ analog_handler;
struct analog_handler_ {
    thread_handler parent;
    event_handler_function destroy_parent_members;
    // FIXME: Añade todos los atributos que necesites
};

analog_handler* analog_handler_new (int pin, int low, int high,
				    analog_handler_function low_handler,
				    analog_handler_function high_handler);
void analog_handler_init (analog_handler* this,
			  int pin, int low, int high,
			  analog_handler_function low_handler,
			  analog_handler_function high_handler);
void analog_handler_destroy (analog_handler* ev);

#endif
