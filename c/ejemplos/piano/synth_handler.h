#ifndef SYNTH_HANDLER_H
#define SYNTH_HANDLER_H

#include <reactor/socket_handler.h>
#include <reactor/process_handler.h>

typedef struct synth_handler_ synth_handler;
typedef void (*synth_handler_function)(synth_handler* ev, const char* cmd, size_t size);

struct synth_handler_ {
    connector parent;
    process_handler scsynth;
    synth_handler_function handler;
    event_handler_function destroy_parent_members;
    int done;
};


synth_handler* synth_handler_new(synth_handler_function handler);
void synth_handler_init(synth_handler* this,
			synth_handler_function handler);
void synth_handler_destroy(synth_handler* this);

void synth_handler_send(synth_handler* h, const char* cmd, ...);
void synth_handler_wait_done(synth_handler* this);

#endif
