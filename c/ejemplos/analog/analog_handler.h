#ifndef ANALOG_HANDLER_H
#define ANALOG_HANDLER_H

#include <reactor/thread_handler.h>

typedef struct {
    analog_handler_function low_handler, high_handler;
    unsigned low, high;
    int pin;
    unsigned buf_size, clk_freq;
} analog_config;


typedef struct analog_handler_ analog_handler;
typedef void (*analog_handler_function)(analog_handler* this, unsigned v);

struct analog_handler_ {
    thread_handler parent;
    event_handler_function destroy_parent_members;
    analog_config cfg;
    int spi;
    unsigned current;
    void* buf;
};

extern analog_config DEFAULT_ANALOG_CONFIG;

analog_handler* analog_handler_new (analog_config* cfg);
void analog_handler_init (analog_handler* this, analog_config* cfg);
void analog_handler_destroy (analog_handler* ev);

#endif
