#include "synth_handler.h"
#include <reactor/reactor.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

#define SCSYNTH_PORT "9999"
#define SCSYNTH_UGENS_PATH "/opt/sonic-pi/app/server/native/raspberry/extra-ugens:/usr/lib/SuperCollider/plugins"


static void scsynth_start(synth_handler*);
static void synth_handler_free_members(synth_handler*);
static void synth_osc_handler(synth_handler*);
static void wait_seconds(int sec);
static void do_nothing(event_handler* ev) {}

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
    udp_connector_init(&this->parent,
		       "localhost", SCSYNTH_PORT,
		       (event_handler_function)synth_osc_handler);
    this->pending_done = 0;
    this->handler = handler;
    process_handler_init(&this->scsynth, do_nothing, do_nothing);
    if (process_handler_is_child(&this->scsynth))
	scsynth_start(this);
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) synth_handler_free_members;
}


void synth_handler_destroy(synth_handler* this)
{
    event_handler_destroy((event_handler*)this);
}


void synth_handler_send(synth_handler* this,
			const char* cmd,
			const char* types,
			...)
{
    size_t size;
    char buf[256];

    va_list ap;
    va_start(ap, types);
    lo_message m = lo_message_new();
    lo_message_add_varargs(m, types, ap);
    lo_message_serialise(m, cmd, buf, &size);
    lo_message_free(m);
    va_end(ap);
    
    endpoint_send(&this->parent, buf, size);
}


void synth_handler_sync(synth_handler* this)
{
    static int seq = 1000;
    event_handler* ev = (event_handler*) this;
    Assert (ev->r != NULL);
    this->pending_done = seq;
    synth_handler_send(this, "/sync", "i", seq++, LO_ARGS_END);

    ev->r->running = 1;
    while(this->pending_done && ev->r->running)
        reactor_demultiplex_events(ev->r);
}


void synth_handler_connect(synth_handler* this)
{
    event_handler* ev = (event_handler*) this;
    Assert (ev->r != NULL);
    for(;;) {
	wait_seconds(1);
	exception e __attribute__((unused));
	Try {
	    char buf[256];
	    synth_handler_send(this, "/status", NULL, LO_ARGS_END);
	    endpoint_recv(&this->parent, buf, sizeof(buf));
	    return;
	}
	Catch(e) {
	}
    }
}


static void scsynth_start (synth_handler* this)
{
#ifdef NDEBUG
    dup2(open("/dev/null", O_WRONLY), 1);
    dup2(1, 2);
#endif
    dup2(open("/dev/null", O_RDONLY), 0);
    setenv("SC_JACK_DEFAULT_OUTPUTS",
	   "system:playback_1,system:playback_2", 0);
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
    exit(0);
}


static void synth_handler_free_members(synth_handler* this)
{
    synth_handler_send(this, "/quit", NULL, LO_ARGS_END);
    wait_seconds(1);
    process_handler_destroy(&this->scsynth);
    this->destroy_parent_members((event_handler*)this);
}


static void synth_osc_handler(synth_handler* this)
{
    char buf[256];
    size_t size = endpoint_recv(&this->parent, buf, sizeof(buf));

    lo_message m = lo_message_deserialise (buf, size, NULL);
    Assert(m != NULL);
    this->handler(this, buf, m);

    if (lo_message_get_argv(m)[0]->i == this->pending_done
	&& 0 == strcmp("/synced", buf))
	this->pending_done = 0;

    lo_message_free(m);
}


static void wait_seconds(int sec)
{
    struct timespec t = { sec, 0 };
    nanosleep(&t, NULL);
}
