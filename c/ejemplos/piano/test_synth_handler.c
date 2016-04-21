#include "synth_handler.h"
#include <reactor/reactor.h>
#include <reactor/console.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static int read_key(int fd);
static void do_nothing(synth_handler* ev, const char* cmd, size_t size) {}
static void init_synth(synth_handler* synth);

int main() {
    void* state = console_set_raw_mode(0);
    reactor* r = reactor_new();
    synth_handler* synth = synth_handler_new(do_nothing);

    void keyboard(event_handler* ev) {
	static unsigned n = 10;
        int key = read_key(ev->fd);
        if ('q' == key)
            reactor_quit(r);
        else if ('1' == key)
	    synth_handler_send(synth,
			       "/s_new", "fi",
			       "sonic-pi-piano", n++, 0, 7,
			       "note", 28.0,
			       "out_bus", 12,
			       NULL);
    }

    reactor_add(r, (event_handler*)synth);
    reactor_add(r, event_handler_new(0, keyboard));
    init_synth(synth);
    printf("1 para tocar :e1, q para salir\n");
    reactor_run(r);
    reactor_destroy(r);
    console_restore(0, state);
    return 0;
}


/*
  Secuencia de inicialización copiada de sonic-pi con Wireshark
  al ejecutar el programa siguiente:

  	use_synth :piano
  	play :e1
 */
void init_synth(synth_handler* synth)
{
    synth_handler_send(synth, "/clearSched");
    synth_handler_send(synth, "/g_freeAll", 0);
    synth_handler_send(synth, "/d_loadDir",
		       "/opt/sonic-pi/etc/synthdefs/compiled");
    synth_handler_send(synth, "/b_allocRead", 0,
		       "/opt/sonic-pi/etc/buffers/rand-stream.wav", 0, 0);
    synth_handler_wait_done(synth);
    synth_handler_send(synth, "/g_new", 2, 0, 0);
    synth_handler_send(synth, "/g_new", 3, 2, 2);
    synth_handler_send(synth, "/g_new", 4, 2, 3);
    synth_handler_send(synth, "/g_new", 5, 3, 2);
    synth_handler_send(synth, "/s_new", "i",
		       "sonic-pi-mixer", 6, 0, 2,
		       "in_bus", 10);
    synth_handler_send(synth, "/g_new", 7, 1, 4);
    synth_handler_send(synth, "/s_new", "ifiiifi",
		       "sonic-pi-basic_mixer", 8, 0, 2,
		       "amp", 1,
		       "amp_slide", 0.1, // 1.0
		       "amp_slide_shape", 1,
		       "amp_slide_curve", 0,
		       "in_bus", 12,
		       "amp", 0.3, // 0
		       "out_bus", 10);
}


static int read_key(int fd)
{
    char buf[2] = " ";
    if (0 < read(fd, buf, 1))
        return buf[0];
    return -1;
}
