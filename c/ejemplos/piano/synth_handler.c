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
#define SCSYNTH_UGENS_PATH "/opt/sonic-pi/app/server/native/raspberry/extra-ugens:/usr/lib/SuperCollider/plugins"


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
    this->pending_done = this->notify = 0;
    this->handler = handler;
    process_handler_init(&this->scsynth, do_nothing, do_nothing);
    if (process_handler_is_child(&this->scsynth)) {
	scsynth_start(this);
    }
    wait_seconds(5);
    udp_connector_init(&this->parent,
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
    size_t size = osc_encode_message(buf, sizeof(buf), cmd, &ap);
    va_end(ap);
    if (0 == strcmp(buf, "/notify")) {
	this->notify = buf[15];
    }
    endpoint_send(&this->parent, buf, size);
    if ((this->notify && osc_is_notified(cmd)) || osc_has_reply(cmd)) {
	printf(" ++pending_done [%s]\n", cmd);
	++this->pending_done;
    }
}


void synth_handler_wait_done(synth_handler* this)
{
    event_handler* ev = (event_handler*) this;
    ev->r->running = 1;
    while(this->pending_done && ev->r->running) {
        reactor_demultiplex_events(ev->r);
    }
}


static void scsynth_start (synth_handler* this)
{
#ifdef NDEBUG
    dup2(open("/dev/null", O_WRONLY), 1);
    dup2(1, 2);
#endif
    dup2(open("/dev/null", O_RDONLY), 0);
    execlp("/usr/bin/scsynth", "scsynth",
	   "-u", SCSYNTH_PORT,
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


static void synth_osc_handler(synth_handler* this)
{
    char in[256], out[256];
    size_t size = endpoint_recv(&this->parent, in, sizeof(in));
    size = osc_decode_message(in, size, out, sizeof(out));
    if (this->pending_done && osc_is_reply(out)) {
	printf(" --pending_done [%s]\n", out);
	--this->pending_done;
    }
    this->handler(this, out, size);
}


static void wait_seconds(int sec)
{
    struct timespec t = { sec, 0 };
    nanosleep(&t, NULL);
}
