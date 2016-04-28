#include "analog_handler.h"

// FIXME: declaraciones de funciones estáticas específicas

analog_handler* analog_handler_new (int pin, int low, int high,
				    analog_handler_function low_handler,
				    analog_handler_function high_handler)
{
    analog_handler* h = malloc(sizeof(analog_handler));
    analog_handler_init_members(h, pin, low, high, low_handler, high_handler);
    event_handler* ev = (event_handler*) h;
    ev->destroy_self = (event_handler_function) free;
    thread_handler_start (&h->parent, analog_handler_thread);
    return h;
}


void analog_handler_init (analog_handler* this,
			  int pin, int low, int high,
			  analog_handler_function low_handler,
			  analog_handler_function high_handler)
{
    analog_handler_init_members(this, pin, low, high, low_handler, high_handler);
    thread_handler_start (&this->parent, analog_handler_thread);
}


void analog_handler_destroy (analog_handler* this)
{
    event_handler_destroy((event_handler*)this);
}

// FIXME: Definiciones de funciones estáticas específicas
