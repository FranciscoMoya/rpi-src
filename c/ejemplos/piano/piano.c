#include <reactor/reactor.h>
#include "matrix_handler.h"
#include "synth_handler.h"
#include <wiringPi.h>
#include <stdio.h>

static synth_handler* synth = NULL;

static void do_nothing(synth_handler* ev, const char* cmd, lo_message m) {}
static void release(matrix_handler* ev, int key) {}
static void press(matrix_handler* ev, int key);
static void init_synth(synth_handler* synth);

#define NELEMS(x) (sizeof(x)/sizeof(x[0]))

int main()
{
    int rows[] = { 4, 17, 27, 22 };
    int cols[] = { 18, 23, 24, 25 };

    wiringPiSetupGpio();
    synth = synth_handler_new(do_nothing);
    reactor* r = reactor_new();
    reactor_add(r, (event_handler*) matrix_handler_new(rows, NELEMS(rows),
						       cols, NELEMS(cols),
						       press, release));
    reactor_add(r, (event_handler*) synth);
    init_synth(synth);
    printf("Ready!\n");
    reactor_run(r);
    reactor_destroy(r);
    return 0;
}


static void press(matrix_handler* ev, int key)
{
    static int n = 100;
    synth_handler_send(synth,
		       "/s_new", "siiisfsi",
		       "sonic-pi-piano", n++, 0, 7,
		       "note", 48.0 + key,
		       "out_bus", 12,
		       LO_ARGS_END);
}


/*
  Secuencia de inicialización copiada de sonic-pi con Wireshark
  al ejecutar el programa siguiente:

  	use_synth :piano
  	play :e1
 */
void init_synth(synth_handler* synth)
{
    synth_handler_connect(synth);
    synth_handler_send(synth, "/clearSched", NULL, LO_ARGS_END);
    synth_handler_send(synth, "/g_freeAll", "i", 0, LO_ARGS_END);
    synth_handler_send(synth, "/d_loadDir", "s",
		       "/opt/sonic-pi/etc/synthdefs/compiled",
		       LO_ARGS_END);
    synth_handler_send(synth, "/b_allocRead", "isii", 0,
		       "/opt/sonic-pi/etc/buffers/rand-stream.wav", 0, 0,
		       LO_ARGS_END);
    synth_handler_sync(synth);
    synth_handler_send(synth, "/g_new", "iii", 2, 0, 0, LO_ARGS_END);
    synth_handler_send(synth, "/g_new", "iii", 3, 2, 2, LO_ARGS_END);
    synth_handler_send(synth, "/g_new", "iii", 4, 2, 3, LO_ARGS_END);
    synth_handler_send(synth, "/g_new", "iii", 5, 3, 2, LO_ARGS_END);
    synth_handler_send(synth, "/s_new", "siiisi",
		       "sonic-pi-mixer", 6, 0, 2,
		       "in_bus", 10,
		       LO_ARGS_END);
    synth_handler_send(synth, "/g_new", "iii", 7, 1, 4, LO_ARGS_END);
    synth_handler_send(synth, "/s_new", "siiisisfsisisisfsi",
		       "sonic-pi-basic_mixer", 8, 0, 2,
		       "amp", 1,
		       "amp_slide", 0.1, // 1.0
		       "amp_slide_shape", 1,
		       "amp_slide_curve", 0,
		       "in_bus", 12,
		       "amp", 0.3, // 0
		       "out_bus", 10,
		       LO_ARGS_END);
}
