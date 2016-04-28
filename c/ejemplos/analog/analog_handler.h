#ifndef ANALOG_HANDLER_H
#define ANALOG_HANDLER_H

#include <reactor/thread_handler.h>

typedef struct analog_handler_ analog_handler;
typedef void (*analog_handler_function)(analog_handler* this, int v);

struct analog_handler_ {
    thread_handler parent;
    event_handler_function destroy_parent_members;
    int pin, low, high, current, spi;
    analog_handler_function low_handler, high_handler;
    void* buf;
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
