#include "synth_handler.h"
#include <reactor/reactor.h>
#include <reactor/console.h>
#include <unistd.h>
#include <stdio.h>

#define SCSYNDEF "/opt/sonic-pi/etc/synthdefs/compiled/%s.scsyndef"

static const char* syndef(const char* name);
static int read_key(int fd);
static void print_osc(synth_handler* ev, const char* cmd, size_t size);


int main() {
    void* state = console_set_raw_mode(0);
    reactor* r = reactor_new();
    synth_handler* synth = synth_handler_new(print_osc);

    void keyboard(event_handler* ev) {
        int key = read_key(ev->fd);
        if ('q' == key)
            reactor_quit(r);
        else if ('1' == key)
            synth_handler_send(synth, "/s_new", "sonic-pi-piano", 1000, 1, 0, "freq", 800, NULL);
        else if ('0' == key)
            synth_handler_send(synth, "/n_free", 1000);
    }

    reactor_add(r, (event_handler*)synth);
    synth_handler_send(synth, "/d_load", syndef("sonic-pi-piano"));
    synth_handler_wait_done(synth);
    reactor_add(r, event_handler_new(0, keyboard));
    
    reactor_run(r);
    reactor_destroy(r);
    console_restore(0, state);
    return 0;
}


static void print_osc(synth_handler* ev, const char* cmd, size_t size)
{
    printf("received: %s\n", cmd);
}


static const char* syndef(const char* name)
{
    static char path[64];
    sprintf(path, SCSYNDEF, name);
    return path;
}


static int read_key(int fd)
{
    char buf[2] = " ";
    if (0 < read(fd, buf, 1))
        return buf[0];
    return -1;
}
