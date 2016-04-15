#include "synth_handler.h"
#include <unistd.h>
#include <time.h>
#include <stdarg.h>

#define SCSYNTH_PORT "10101"

static void do_nothing(event_handler*) {}
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
}

void synth_handler_init(synth_handler* this,
			synth_handler_function handler)
{

    event_handler* ev = (event_handler*) this;
    process_handler_init(&this->scsynth, do_nothing, do_nothing);
    if (process_handler_is_child(&this->scsynth)) {
	scsynth_start(this);
    }
    wait_seconds(3);
    connector_init(&this->parent,
		   "localhost", SCSYNTH_PORT,
		   synth_osc_handler);
    this->destroy_parent_members = ev->destroy_members;
    ev->destroy_members = (event_handler_function) synth_handler_free_members;
}

void synth_handler_destroy(synth_handler* this)
{
    event_handler_destroy((event_handler*)this);
}

void synth_handler_send(synth_handler* this, void* msg, int size)
{
    char buf[128];
    size_t size = osc_encode_message(buf, sizeof(buf), msg);
    connector_send(&this->parent, buf, size);
}

static void scsynth_start (synth_handler* this)
{
    dup2(open("/dev/null", O_WRONLY), 1);
    dup2(1, 2);
    dup2(open("/dev/null", O_RDONLY), 0);
    execlp("/usr/bin/scsynth", "scsynth",
	   "-t", SCSYNTH_PORT,
	   "-R", "0", 0);
}

static void synth_handler_free_members(synth_handler* this)
{
    synth_handler_send(this, "/quit");
    wait_seconds(3);
    process_handler_destroy(&this->scsynth);
    this->destroy_parent_members((event_handler*)this);
}


static int synth_recv(synth_handler* this, char* buf, size_t size)
{
    if (4 > connector_recv(&this->parent, &msg_size, sizeof(msg_size)))
	Throw Exception(0, "Incomplete OSC message");

    msg_size = ntohl(msg_size);
    if (msg_size > size)
	Throw Exception(0, "Buffer too small to hold OSC message");
    
    char* p = buf;
    for(;;) {
	int n = connector_recv(&this->parent, p, size);
	p += n;
	size -= n;
	if (p - buf >= msg_size)
	    break;
    }
    return msg_size;
}


static void synth_osc_handler(synth_handler* this)
{
    char buf[128];
    int size = synth_recv(&this->parent, buf, sizeof(buf));
    osc_decode_message(buf, size, buf);
    if (0 == strcmp(buf, "/done") && !this->ready) {
	this->ready = 1;
	return;
    }
    this->handle_osc(this, buf);
}

static void wait_seconds(int sec)
{
    struct timespec t = { sec, 0 };
    nanosleep(&t, NULL);
}
