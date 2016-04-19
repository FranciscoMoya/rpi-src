#include "synth_handler.h"
#include "osc.h"
#include <reactor/reactor.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

#include <stdio.h>

#define SCSYNTH_PORT "9999"
#define SCSYNTH_UGENS_PATH "/usr/lib/SuperCollider/plugins:/opt/sonic-pi/app/server/native/raspberry/extra-ugens"


static void do_nothing(event_handler* ev) {}
static void scsynth_start(synth_handler*);
static void synth_handler_free_members(synth_handler*);
static void synth_osc_handler(synth_handler*);
static void wait_seconds(int sec);


synth_handler* synth_handler_new(synth_handler_function handler)
{
    synth_handler* this = malloc(sizeof(synth_handler));
    event_handler* ev = (event_handler*) this;
    synth_handler_init(this, handler);
    ev->destroy_self = (event_handler_function) free;
    return this;
}


void synth_handler_init(synth_handler* this,
			synth_handler_function handler)
{

    event_handler* ev = (event_handler*) this;
    this->pending_done = 0;
    this->handler = handler;
    process_handler_init(&this->scsynth, do_nothing, do_nothing);
    if (process_handler_is_child(&this->scsynth)) {
	scsynth_start(this);
    }
    wait_seconds(5);
    connector_init(&this->parent,
		   "localhost", SCSYNTH_PORT,
		   (event_handler_function)synth_osc_handler);
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) synth_handler_free_members;
}


void synth_handler_destroy(synth_handler* this)
{
    event_handler_destroy((event_handler*)this);
}


void synth_handler_send(synth_handler* this, const char* cmd, ...)
{
    printf("Sending %s...\n", cmd);
    char buf[256];
    va_list ap;
    va_start(ap, cmd);
    size_t size = osc_encode_message(buf, sizeof(buf), cmd, ap);
    va_end(ap);
    connector_send(&this->parent, buf, size);
    if (osc_async(cmd))
	++this->pending_done;
}


void synth_handler_wait_done(synth_handler* this)
{
    event_handler* ev = (event_handler*) this;
    while(this->pending_done) {
        reactor_demultiplex_events(ev->r);
    }
}


static void scsynth_start (synth_handler* this)
{
    printf("starting scsynth...\n");
#ifdef NDEBUG
    dup2(open("/dev/null", O_WRONLY), 1);
    dup2(1, 2);
#endif
    dup2(open("/dev/null", O_RDONLY), 0);
    execlp("/usr/bin/scsynth", "scsynth",
	   "-t", SCSYNTH_PORT,
	   "-a", "64",
	   "-m", "131072",
	   "-D", "0", // no carga synthdefs
	   "-R", "0", // no se anuncia
	   "-l", "1", // solo uno conectado
	   "-z", "128",
	   "-c", "128",
	   "-U", SCSYNTH_UGENS_PATH,
	   "-i", "2",
	   "-o", "2",
	   NULL);
}


static void synth_handler_free_members(synth_handler* this)
{
    synth_handler_send(this, "/quit");
    wait_seconds(3);
    process_handler_destroy(&this->scsynth);
    this->destroy_parent_members((event_handler*)this);
}


static size_t synth_recv(synth_handler* this, char* buf, size_t size)
{
    size_t msg_size;
    if (4 > connector_recv(&this->parent, &msg_size, sizeof(msg_size)))
	Throw Exception(0, "Incomplete OSC message");

    msg_size = ntohl(msg_size);
    if (msg_size > size)
	Throw Exception(0, "Buffer too small to hold OSC message");
    
    char* p = buf;
    for(;;) {
	int n = connector_recv(&this->parent, p, size);
	p += n;	size -= n;
	if (p - buf >= msg_size)
	    break;
    }
    return msg_size;
}


static void synth_osc_handler(synth_handler* this)
{
    char in[256], out[256];
    size_t size = synth_recv(this, in, sizeof(in));
    size = osc_decode_message(in, size, out, sizeof(out));
    if (0 == strcmp(out, "/done"))
	--this->pending_done;
    this->handler(this, out, size);
}


static void wait_seconds(int sec)
{
    struct timespec t = { sec, 0 };
    nanosleep(&t, NULL);
}
